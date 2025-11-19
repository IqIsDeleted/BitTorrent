#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <inttypes.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

#include "common/bitfield.h"
#include "common/client_list.h"
#include "common/epoll_utils.h"
#include "common/network_utils.h"
#include "common/path_utils.h"
#include "config/config.h"
#include "file/file_assembler.h"
#include "file/file_reader.h"
#include "file/torrent_parser.h"
#include "hash/hash.h"
#include "hash/table.h"
#include "leecher.h"
#include "network/tcp_client.h"
#include "network/tcp_server.h"
#include "network/udp_broadcast.h"
#include "network/udp_broadcast_receiver.h"
#include "seeder.h"
#include "signals/signals.h"
#include "ui/progress_bar.h"

#define NO_SUCH_FILE_MSG "%s: No such file or directory\n"

static int is_path_exist(const char* path) { return access(path, F_OK) == 0; }

static int run_application(const Config* cfg, int epoll_fd, int signal_fd) {
  print_client_config(cfg);

  if (cfg->mode == SEED) {
    run_seeder_mode(epoll_fd, signal_fd, cfg);
  } else {
    run_leecher_mode(epoll_fd, signal_fd, cfg);
  }

  return 0;
}

static void cleanup_resources(int epoll_fd, int signal_fd) {
  printf("Cleaning up resources...\n");

  if (epoll_fd >= 0) {
    close(epoll_fd);
  }

  if (signal_fd >= 0) {
    close(signal_fd);
  }
}

int main(int argc, char* argv[]) {
  int signal_fd = -1;
  int epoll_fd = -1;

  Config config = {0};
  int result = init_config(&config, argc, argv);
  if (result != 0) {
    return result;
  }

  if (!is_path_exist(config.torrent_path)) {
    printf(NO_SUCH_FILE_MSG, config.torrent_path);
    return -1;
  }

  if (!is_path_exist(config.data_path)) {
    printf(NO_SUCH_FILE_MSG, config.data_path);
    return -1;
  }

  signal_fd = setup_signal_handlers();
  if (signal_fd < 0) {
    fprintf(stderr, "Failed to setup signal handlers\n");
    return -1;
  }

  epoll_fd = epoll_create1(0);
  if (epoll_fd < 0) {
    perror("epoll_create1");
    cleanup_resources(epoll_fd, signal_fd);
    return -1;
  }

  if (add_to_epoll(epoll_fd, signal_fd) < 0) {
    perror("add_to_epoll");
    cleanup_resources(epoll_fd, signal_fd);
    return -1;
  }

  run_application(&config, epoll_fd, signal_fd);

  cleanup_resources(epoll_fd, signal_fd);

  return 0;
}

#ifndef SEEDER_H_
#define SEEDER_H_

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

#include "bit_torrent.h"
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
#include "signals/signals.h"
#include "ui/progress_bar.h"

void run_seeder_mode(int epoll_fd, int signal_fd, const Config* cfg);

#endif  // SEEDER_H_
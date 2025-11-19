#ifndef LEECHER_H_
#define LEECHER_H_

#include <arpa/inet.h>
#include <inttypes.h>
#include <limits.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
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
#include "file/torrent_parser.h"
#include "hash/hash.h"
#include "network/tcp_client.h"
#include "network/udp_broadcast.h"
#include "network/udp_broadcast_receiver.h"
#include "signals/signals.h"
#include "ui/progress_bar.h"

void run_leecher_mode(int epoll_fd, int signal_fd, const Config* cfg);

#endif  // LEECHER_H_
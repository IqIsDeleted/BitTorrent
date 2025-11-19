#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE
#include "leecher.h"

static int create_timerfd(long interval) {
  int tfd = timerfd_create(CLOCK_REALTIME, 0);
  if (tfd < 0) {
    return -1;
  }
  struct itimerspec spec = {
      .it_value.tv_sec = interval,
      .it_interval.tv_sec = interval,
  };
  timerfd_settime(tfd, 0, &spec, NULL);

  return tfd;
}

static uint64_t find_next_piece(const uint8_t* needed_pieces,
                                uint64_t pieces_count) {
  for (uint64_t i = 0; i < pieces_count; i++) {
    if (get_bit(needed_pieces, i)) {
      return i;
    }
  }
  return pieces_count;  // none found
}

static const char* format_piece_index(uint64_t piece_index) {
  static char piece_index_buf[PIECE_INDEX_BUF_SIZE];
  int len =
      snprintf(piece_index_buf, PIECE_INDEX_BUF_SIZE, "%" PRIu64, piece_index);
  if (len < 0 || (size_t)len >= PIECE_INDEX_BUF_SIZE) {
    return NULL;
  }
  return piece_index_buf;
}

static void init_leecher(eltextorrent_file_t* torrent, char* full_file_path,
                         const Config* cfg) {
  if (torrent_loader(torrent, cfg->torrent_path) < 0) {
    fprintf(stderr, "torrent_loader failed\n");
    exit(EXIT_FAILURE);
  }
  if (build_full_path(full_file_path, PATH_MAX, cfg->data_path,
                      torrent->name) != 0) {
    fprintf(stderr, "Failed to build full path\n");
    exit(EXIT_FAILURE);
  }
  printf("Loaded Torrent file with INFOHASH [%s]\n", torrent->infohash);
  printf("Piece count [%u]\n", torrent->pieces_count);
}

static ClientNode* event_client_connect(udp_broadcast_receiver_t* udprec,
                                        char* buffer,
                                        const uint8_t* needed_pieces,
                                        const eltextorrent_file_t* torrent,
                                        ClientNode** current_client,
                                        ClientNode* clients, int epoll_fd) {
  char sender_ip[INET_ADDRSTRLEN + 6];
  int received = udp_broadcast_receiver_receive(
      udprec, buffer, NETWORK_BUFFER_SIZE, sender_ip, sizeof(sender_ip));
  if (received > 0) {
    printf("received UDP broadcast: [%s] from [%s]\n", buffer, sender_ip);

    int port;
    if (split_ip_port(buffer, &port) == 0) {
      printf("IP: %s\nPort: %d\n", buffer, port);
      TCPClient_t* new_client = tcp_client_create();
      if (tcp_client_connect(new_client, buffer, port) == 0) {
        clients = client_list_add(clients, new_client);

        if (!*current_client) {
          *current_client = clients;  // Разыменовываем указатель
        }

        uint64_t next_piece =
            find_next_piece(needed_pieces, torrent->pieces_count);
        const char* piece_i_str = format_piece_index(next_piece);
        if (!piece_i_str) {
          fprintf(stderr, "Failed to format piece index\n");
          return clients;
        }

        tcp_client_send(new_client, piece_i_str, PIECE_INDEX_BUF_SIZE);
        add_to_epoll(epoll_fd, new_client->socket_fd);
      } else {
        tcp_client_destroy(new_client);
      }
    } else {
      fprintf(stderr, "Failed to split or max clients reached\n");
    }
  }
  return clients;
}

static void handle_tcp_client(ClientNode** clients, ClientNode** current_client,
                              eltextorrent_file_t* torrent,
                              uint8_t* needed_pieces, uint64_t* have_pieces,
                              int epoll_fd, char* piece_buffer, int client_fd) {
  TCPClient_t* client = client_list_find(*clients, client_fd);
  if (!client) {
    return;
  }

  uint64_t piece_index = 0;
  tcp_client_receive(client, (char*)&piece_index, sizeof(piece_index));
  uint32_t packet_size = 0;
  tcp_client_receive(client, (char*)&packet_size, sizeof(packet_size));

  int received = tcp_client_receive(client, piece_buffer, packet_size);
  if (received > 0) {
    if (get_bit(needed_pieces, piece_index)) {
      clear_bit(needed_pieces, piece_index);

      int hash_correct = verify_piece_hash(torrent, (uint8_t*)piece_buffer,
                                           received, piece_index);
      if (hash_correct == 0) {
        printf("ERROR: Hash verification failed for piece %lu\n", piece_index);
      }

      write_piece_to_file(piece_index, (uint8_t*)piece_buffer, received,
                          torrent->piece_size);
      (*have_pieces)++;
      update_progress_bar(torrent->file_size,
                          torrent->piece_size * (*have_pieces));

      if (*have_pieces < torrent->pieces_count) {
        // Cycle to next client
        if (*current_client) {
          *current_client = (*current_client)->next;
          if (!*current_client) {
            *current_client = *clients;
          }
        }

        if (*current_client) {
          TCPClient_t* next_client = (*current_client)->client;
          uint64_t next_piece =
              find_next_piece(needed_pieces, torrent->pieces_count);
          const char* piece_i_str = format_piece_index(next_piece);
          if (!piece_i_str) {
            fprintf(stderr, "Failed to format piece index\n");
            return;
          }
          tcp_client_send(next_client, piece_i_str, PIECE_INDEX_BUF_SIZE);
        }
      }
    }
  } else {
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client->socket_fd, NULL);
    tcp_client_destroy(client);

    // Remove from list
    *clients = client_list_remove(*clients, client);

    if (*current_client && (*current_client)->client == client) {
      *current_client = (*current_client)->next;
    }
  }
}

void run_leecher_mode(int epoll_fd, int signal_fd, const Config* cfg) {
  char buffer[NETWORK_BUFFER_SIZE];
  char full_file_path[PATH_MAX];
  int shutdown_requested = 0;
  struct epoll_event events[MAX_EPOLL_EVENTS];

  ClientNode* clients = client_list_create();
  ClientNode* current_client = NULL;  // Теперь это указатель

  udp_broadcast_t* udpbr = udp_broadcast_create(UDP_RECEIVE_PORT);
  udp_broadcast_receiver_t* udprec =
      udp_broadcast_receiver_create(UDP_BROADCAST_PORT);
  add_to_epoll(epoll_fd, udprec->socket_fd);

  int tfd = create_timerfd(TIMER_INTERVAL_SEC);
  add_to_epoll(epoll_fd, tfd);

  eltextorrent_file_t torrent = {0};
  init_leecher(&torrent, full_file_path, cfg);

  uint8_t* needed_pieces = calloc((torrent.pieces_count + 7) / 8, 1);
  if (!needed_pieces) {
    fprintf(stderr, "Failed to allocate needed_pieces\n");
    exit(EXIT_FAILURE);
  }
  for (uint64_t i = 0; i < torrent.pieces_count; i++) {
    set_bit(needed_pieces, i);
  }

  file_assembler_init(full_file_path, torrent.file_size);
  char* piece_buffer = malloc(torrent.piece_size);
  if (!piece_buffer) {
    fprintf(stderr, "Failed to allocate piece_buffer\n");
    exit(EXIT_FAILURE);
  }

  uint64_t have_pieces = 0;
  while (!shutdown_requested && have_pieces < torrent.pieces_count) {
    int nfds = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, EPOLL_TIMEOUT_MS);

    if (nfds == -1) {
      perror("epoll_wait");
      break;
    }

    for (int i = 0; i < nfds; i++) {
      if (events[i].data.fd == signal_fd) {
        if (handle_signalfd_event(signal_fd, &shutdown_requested) != 0) {
          shutdown_requested = 1;
          break;
        }
      } else if (events[i].data.fd == tfd) {
        uint64_t numExp;
        ssize_t rs = read(tfd, &numExp, sizeof(uint64_t));
        if (rs == -1) {
          fprintf(stderr, "Failed to read in numExp\n");
        }
        udp_broadcast_send(udpbr, (const char*)&torrent.infohash, HASH_SIZE);
      } else if (events[i].data.fd == udprec->socket_fd) {
        clients = event_client_connect(udprec, buffer, needed_pieces, &torrent,
                                       &current_client, clients, epoll_fd);
      } else {
        handle_tcp_client(&clients, &current_client, &torrent, needed_pieces,
                          &have_pieces, epoll_fd, piece_buffer,
                          events[i].data.fd);
      }
    }
  }

  free(piece_buffer);
  client_list_destroy(clients);
  free(needed_pieces);
  udp_broadcast_destroy(udpbr);
  udp_broadcast_receiver_destroy(udprec);
  torrent_free(&torrent);
}

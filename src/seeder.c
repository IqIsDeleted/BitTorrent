#include "seeder.h"

static int is_path_exist(const char* path) { return access(path, F_OK) == 0; }

static int init_torrent(eltextorrent_file_t* torrent, char* full_file_path,
                        size_t path_size, const Config* cfg) {
  if (torrent_loader(torrent, cfg->torrent_path) < 0) {
    fprintf(stderr, "Failed to load torrent: %s\n", cfg->torrent_path);
    return -1;
  }

  if (build_full_path(full_file_path, path_size, cfg->data_path,
                      torrent->name) != 0 ||
      !is_path_exist(full_file_path)) {
    fprintf(stderr, "File not found: %s/%s\n", cfg->data_path, torrent->name);
    return -1;
  }

  printf("Torrent loaded: [%s]\n", torrent->infohash);
  return 0;
}

static int init_network(TCPServer_t** tcp_srv, udp_broadcast_t** udp_bcast,
                        udp_broadcast_receiver_t** udp_recv, int epoll_fd) {
  *tcp_srv = tcp_server_create(SEEDER_TCP_PORT);
  if (!*tcp_srv || tcp_server_listen(*tcp_srv, 64) < 0 ||
      add_to_epoll(epoll_fd, (*tcp_srv)->socket_fd) < 0) {
    return -1;
  }

  *udp_bcast = udp_broadcast_create(UDP_BROADCAST_PORT);
  *udp_recv = udp_broadcast_receiver_create(UDP_RECEIVE_PORT);
  if (!*udp_bcast || !*udp_recv ||
      add_to_epoll(epoll_fd, (*udp_recv)->socket_fd) < 0) {
    return -1;
  }

  return 0;
}

static void handle_udp_broadcast(udp_broadcast_receiver_t* udp_recv,
                                 udp_broadcast_t* udp_bcast,
                                 Leechees_t** leechees,
                                 const eltextorrent_file_t* torrent,
                                 const char* lan_addr) {
  char buffer[NETWORK_BUFFER_SIZE], sender[INET_ADDRSTRLEN + 6],
      ip[INET_ADDRSTRLEN] = {0};

  int len = udp_broadcast_receiver_receive(udp_recv, buffer, sizeof(buffer),
                                           sender, sizeof(sender));
  if (len <= 0) {
    return;
  }

  memcpy(ip, sender, strcspn(sender, ":"));

  if (!find_leech(leechees, ip) &&
      strncmp((char*)torrent->infohash, buffer, HASH_SIZE) == 0) {
    add_leech(leechees, ip);
    printf("UDP from [%s], added\n", sender);
    udp_broadcast_send(udp_bcast, lan_addr, strlen(lan_addr));
  }
}

static void handle_new_connection(TCPServer_t* tcp_srv, int epoll_fd) {
  TCPClient_t* client = tcp_server_accept(tcp_srv);
  if (!client) {
    return;
  }

  if (add_to_epoll_ptr(epoll_fd, client->socket_fd, client) >= 0) {
    printf("Client connected: [%s:%d]\n", client->ip, client->port);
  } else {
    tcp_server_disclient(tcp_srv, client);
  }
}

static void handle_client_request(TCPClient_t* client,
                                  const char* full_file_path,
                                  char* piece_buffer, size_t piece_size,
                                  Leechees_t** leechees, int epoll_fd) {
  char buffer[NETWORK_BUFFER_SIZE];
  ssize_t received = tcp_server_receive(client, buffer, sizeof(buffer));

  if (received > 0) {
    uint64_t piece_num = (uint64_t)atol(buffer);
    uint32_t offset;

    read_file_piece(full_file_path, piece_num, piece_size,
                    (unsigned char*)piece_buffer, piece_size, &offset);

    printf("Piece %lu requested, size: %u\n", piece_num, offset);

    tcp_server_send(client, (char*)&piece_num, sizeof(piece_num));
    tcp_server_send(client, (char*)&offset, sizeof(offset));
    tcp_server_send(client, piece_buffer, offset);
  } else {
    printf("Client disconnected\n");
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client->socket_fd, NULL);

    Leechees_t* found = find_leech(leechees, client->ip);
    if (found) {
      delete_leech(leechees, found);
    }
  }
}

static char* get_lan_address(char* lan_addr, size_t size) {
  char* lan_ip = get_lan_ipv4();
  if (!lan_ip) {
    return NULL;
  }

  snprintf(lan_addr, size, "%s:%d", lan_ip, SEEDER_TCP_PORT);
  free(lan_ip);

  printf("Listening on: %s\n", lan_addr);
  return lan_addr;
}

void run_seeder_mode(int epoll_fd, int signal_fd, const Config* cfg) {
  char full_file_path[PATH_MAX], lan_addr[INET_ADDRSTRLEN + 7];
  struct epoll_event events[MAX_EPOLL_EVENTS];
  eltextorrent_file_t torrent = {0};
  Leechees_t* leechees = NULL;
  int shutdown = 0;

  if (init_torrent(&torrent, full_file_path, sizeof(full_file_path), cfg) < 0) {
    exit(EXIT_FAILURE);
  }

  char* piece_buffer = malloc(torrent.piece_size);
  if (!piece_buffer) {
    exit(EXIT_FAILURE);
  }

  TCPServer_t* tcp_srv;
  udp_broadcast_t* udp_bcast;
  udp_broadcast_receiver_t* udp_recv;

  if (init_network(&tcp_srv, &udp_bcast, &udp_recv, epoll_fd) < 0 ||
      !get_lan_address(lan_addr, sizeof(lan_addr))) {
    free(piece_buffer);
    exit(EXIT_FAILURE);
  }

  while (!shutdown) {
    int nfds = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENTS, EPOLL_TIMEOUT_MS);
    if (nfds < 0) {
      break;
    }

    for (int i = 0; i < nfds; i++) {
      int fd = events[i].data.fd;

      if (fd == signal_fd) {
        handle_signalfd_event(signal_fd, &shutdown);
      } else if (fd == udp_recv->socket_fd) {
        handle_udp_broadcast(udp_recv, udp_bcast, &leechees, &torrent,
                             lan_addr);
      } else if (fd == tcp_srv->socket_fd) {
        handle_new_connection(tcp_srv, epoll_fd);
      } else {
        handle_client_request(events[i].data.ptr, full_file_path, piece_buffer,
                              torrent.piece_size, &leechees, epoll_fd);
      }
    }
  }

  clean_hashtable(&leechees);
  free(piece_buffer);
  tcp_server_destroy(tcp_srv);
  udp_broadcast_destroy(udp_bcast);
  udp_broadcast_receiver_destroy(udp_recv);
}
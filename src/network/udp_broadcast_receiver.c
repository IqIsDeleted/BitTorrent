#include "udp_broadcast_receiver.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

udp_broadcast_receiver_t* udp_broadcast_receiver_create(int port) {
  udp_broadcast_receiver_t* receiver = malloc(sizeof(udp_broadcast_receiver_t));
  if (!receiver) {
    perror("[udp_broadcast_receiver_create] malloc failed");
    return NULL;
  }

  receiver->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (receiver->socket_fd < 0) {
    perror("[udp_broadcast_receiver_create] socket failed");
    free(receiver);
    return NULL;
  }

  int reuse_addr = 1;
  if (setsockopt(receiver->socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,
                 sizeof(reuse_addr)) < 0) {
    perror("[udp_broadcast_receiver_create] setsockopt failed");
    close(receiver->socket_fd);
    free(receiver);
    return NULL;
  }

  struct sockaddr_in local_addr = {0};
  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons(port);
  local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(receiver->socket_fd, (struct sockaddr*)&local_addr,
           sizeof(local_addr)) < 0) {
    perror("[udp_broadcast_receiver_create] bind failed");
    close(receiver->socket_fd);
    free(receiver);
    return NULL;
  }

  receiver->port = port;
  printf("Listening on port: %d\n", port);
  return receiver;
}

int udp_broadcast_receiver_receive(udp_broadcast_receiver_t* receiver,
                                   char* buffer, size_t buffer_size,
                                   char* sender_ip, size_t ip_buffer_size) {
  if (!receiver || !buffer || buffer_size == 0) {
    fprintf(stderr, "[udp_broadcast_receiver_receive] Wrong parameters\n");
    return -1;
  }

  struct sockaddr_in sender_addr = {0};
  socklen_t sender_addr_len = sizeof(sender_addr);

  int received = recvfrom(receiver->socket_fd, buffer, buffer_size, 0,
                          (struct sockaddr*)&sender_addr, &sender_addr_len);
  if (received < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      perror("[udp_broadcast_receiver_receive] recvfrom failed");
    }
    return -1;
  }

  if (received < (int)buffer_size) {
    buffer[received] = '\0';
  }

  if (sender_ip && ip_buffer_size > 0) {
    const char* ip_str = inet_ntoa(sender_addr.sin_addr);
    snprintf(sender_ip, ip_buffer_size, "%s:%d", ip_str,
             ntohs(sender_addr.sin_port));
  }

  return received;
}

int udp_broadcast_receiver_set_non_blocking(udp_broadcast_receiver_t* receiver,
                                            int on) {
  if (!receiver) {
    return -1;
  }

  int flags = fcntl(receiver->socket_fd, F_GETFL, 0);
  if (flags < 0) {
    perror("[udp_broadcast_receiver_set_non_blocking] fcntl failed");
    return -1;
  }

  if (on) {
    flags |= O_NONBLOCK;
  } else {
    flags &= ~O_NONBLOCK;
  }

  if (fcntl(receiver->socket_fd, F_SETFL, flags) < 0) {
    perror("[udp_broadcast_receiver_set_non_blocking] fcntl failed");
    return -1;
  }

  return 0;
}

int udp_broadcast_receiver_set_timeout(udp_broadcast_receiver_t* receiver,
                                       long s, long ms) {
  if (!receiver) {
    return -1;
  }

  struct timeval tv;
  tv.tv_sec = s;
  tv.tv_usec = ms;

  if (setsockopt(receiver->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv,
                 sizeof(tv)) < 0) {
    perror("[udp_broadcast_receiver_set_timeout] setsockopt failed");
    return -1;
  }

  return 0;
}

void udp_broadcast_receiver_destroy(udp_broadcast_receiver_t* receiver) {
  if (receiver) {
    if (receiver->socket_fd > 0) {
      close(receiver->socket_fd);
    }
    free(receiver);
  }
}
#include "udp_broadcast.h"

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

udp_broadcast_t* udp_broadcast_create(int port) {
  udp_broadcast_t* broadcast = malloc(sizeof(udp_broadcast_t));
  if (!broadcast) {
    perror("[udp_broadcast_create] malloc failed");
    return NULL;
  }

  broadcast->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (broadcast->socket_fd < 0) {
    perror("[udp_broadcast_create] socket failed");
    free(broadcast);
    return NULL;
  }

  int on = 1;
  if (setsockopt(broadcast->socket_fd, SOL_SOCKET, SO_BROADCAST, &on,
                 sizeof(on)) < 0) {
    perror("[udp_broadcast_create] setsockopt failed");
    close(broadcast->socket_fd);
    free(broadcast);
    return NULL;
  }

  broadcast->port = port;
  return broadcast;
}

int udp_broadcast_send(udp_broadcast_t* broadcast, const char* data,
                       size_t length) {
  if (!broadcast || !data) {
    fprintf(stderr, "[udp_broadcast_send] Wrong parameters\n");
    return -1;
  }

  struct sockaddr_in broadcast_addr = {0};
  broadcast_addr.sin_family = AF_INET;
  broadcast_addr.sin_port = htons(broadcast->port);
  broadcast_addr.sin_addr.s_addr = INADDR_BROADCAST;

  int sent = sendto(broadcast->socket_fd, data, length, MSG_DONTWAIT,
                    (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr));
  if (sent < 0) {
    perror("[udp_broadcast_send] sendto failed");
    return -1;
  }

  return sent;
}

void udp_broadcast_destroy(udp_broadcast_t* broadcast) {
  if (broadcast) {
    if (broadcast->socket_fd > 0) {
      close(broadcast->socket_fd);
    }
    free(broadcast);
  }
}
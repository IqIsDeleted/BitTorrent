#include "tcp_client.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"

TCPClient_t* tcp_client_create(void) {
  TCPClient_t* client = malloc(sizeof(TCPClient_t));
  if (!client) {
    ERRNO_MSG("malloc failed");
    return NULL;
  }
  memset(client, 0, sizeof(TCPClient_t));

  client->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (client->socket_fd < 0) {
    ERRNO_MSG("socket failed");
    free(client);
    return NULL;
  }

  return client;
}

int tcp_client_connect(TCPClient_t* client, const char* server_ip,
                       in_port_t server_port) {
  if (!client || !server_ip) {
    STDERR_MSG("Wrong parameters");
    return -1;
  }

  struct sockaddr_in server_addr = {0};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);

  if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
    STDERR_MSG("Invalid server_ip");
    return -1;
  }

  if (connect(client->socket_fd, (struct sockaddr*)&server_addr,
              sizeof(server_addr)) < 0) {
    ERRNO_MSG("connect failed");
    return -1;
  }

  client->connected = 1;
  strncpy(client->ip, server_ip, sizeof(client->ip) - 1);
  client->port = server_port;

  printf("Connected to %s:%d\n", client->ip, client->port);
  return 0;
}

int tcp_client_send(TCPClient_t* client, const char* data, size_t data_size) {
  return tcp_send(client, data, data_size);
}

int tcp_client_receive(TCPClient_t* client, char* buffer, size_t buffer_size) {
  if (!client || !buffer || buffer_size == 0) {
    STDERR_MSG("Wrong parameters");
    return -1;
  }

  if (!client->connected) {
    STDERR_MSG("trying to receive data while not connected");
    return -1;
  }

  size_t total_received = 0;
  while (total_received < buffer_size) {
    ssize_t received = recv(client->socket_fd, buffer + total_received,
                            buffer_size - total_received, 0);
    if (received < 0) {
      if (errno != EAGAIN && errno != EWOULDBLOCK) {
        ERRNO_MSG("recv failed");
      }
      return -1;
    }
    if (received == 0) {
      printf("Server disconnected\n");
      client->connected = 0;
      return 0;
    }
    total_received += received;
  }

  return total_received;
}

int tcp_client_set_non_blocking(const TCPClient_t* client, int on) {
  if (!client) {
    STDERR_MSG("Wrong parameters");
    return -1;
  }

  return socket_set_non_blocking(client->socket_fd, on);
}

void tcp_client_destroy(TCPClient_t* client) {
  if (client) {
    if (client->socket_fd >= 0) {
      close(client->socket_fd);
    }
    free(client);
  }
}

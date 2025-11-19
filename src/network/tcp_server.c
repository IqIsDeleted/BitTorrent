#include "tcp_server.h"

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

TCPServer_t* tcp_server_create(in_port_t port) {
  TCPServer_t* server = malloc(sizeof(TCPServer_t));
  if (!server) {
    ERRNO_MSG("malloc failed");
    return NULL;
  }

  server->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server->socket_fd < 0) {
    ERRNO_MSG("socket failed");
    free(server);
    return NULL;
  }

  int reuse_addr = 1;
  if (setsockopt(server->socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr,
                 sizeof(reuse_addr)) < 0) {
    ERRNO_MSG("setsockopt failed");
    close(server->socket_fd);
    free(server);
    return NULL;
  }

  struct sockaddr_in server_addr = {0};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(server->socket_fd, (struct sockaddr*)&server_addr,
           sizeof(server_addr)) < 0) {
    ERRNO_MSG("bind failed");
    close(server->socket_fd);
    free(server);
    return NULL;
  }

  server->port = port;
  server->client_count = 0;
  memset(server->clients, 0, sizeof(server->clients));

  printf("TCP server created on port: %d\n", port);
  return server;
}

int tcp_server_listen(TCPServer_t* server, int qsize) {
  if (!server) {
    STDERR_MSG("Wrong parameters");
    return -1;
  }

  if (listen(server->socket_fd, qsize) < 0) {
    ERRNO_MSG("listen failed");
    return -1;
  }

  printf("TCP server listening\n");
  return 0;
}

TCPClient_t* tcp_server_accept(TCPServer_t* server) {
  if (!server || server->client_count >= TCP_MAX_CLIENTS) {
    STDERR_MSG("Cannot accept more clients");
    return NULL;
  }

  struct sockaddr_in client_addr = {0};
  socklen_t client_addr_len = sizeof(client_addr);

  int client_socket = accept(server->socket_fd, (struct sockaddr*)&client_addr,
                             &client_addr_len);
  if (client_socket < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      ERRNO_MSG("accept failed");
    }
    return NULL;
  }

  TCPClient_t* new_client = NULL;
  for (uint16_t i = 0; i < TCP_MAX_CLIENTS; i++) {
    if (server->clients[i].connected == 0) {
      new_client = &server->clients[i];
      break;
    }
  }

  if (!new_client) {
    STDERR_MSG("Something really terrible happened");
    return NULL;
  }

  new_client->socket_fd = client_socket;
  new_client->connected = 1;
  new_client->port = ntohs(client_addr.sin_port);
  strncpy(new_client->ip, inet_ntoa(client_addr.sin_addr), INET_ADDRSTRLEN);

  printf("New client from %s:%d\n", new_client->ip, new_client->port);
  server->client_count++;
  return new_client;
}

ssize_t tcp_server_receive(TCPClient_t* client, char* buffer,
                           size_t buffer_size) {
  if (!client || !buffer || buffer_size == 0) {
    STDERR_MSG("Wrong parameters");
    return -1;
  }

  if (!client->connected) {
    STDERR_MSG("Trying to receive data from disconnected client");
    return -1;
  }

  ssize_t received = recv(client->socket_fd, buffer, buffer_size, 0);
  if (received < 0) {
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
      ERRNO_MSG("recv failed");
    }
    return -1;
  }
  if (received == 0) {
    printf("Client %s:%d disconnected\n", client->ip, client->port);
    client->connected = 0;
    return 0;
  }

  buffer[received] = '\0';
  return received;
}

int tcp_server_send(TCPClient_t* client, const char* data, size_t data_size) {
  return tcp_send(client, data, data_size);
}

int tcp_server_disclient(TCPServer_t* server, TCPClient_t* client) {
  if (!server || !client || !client->connected) {
    STDERR_MSG("Wrong parameters");
    return -1;
  }

  if (client->socket_fd >= 0) {
    close(client->socket_fd);
    printf("Client %s:%d disconnected\n", client->ip, client->port);
  }

  client->connected = 0;
  server->client_count--;
  return 0;
}

int tcp_server_set_non_blocking(const TCPServer_t* server, int on) {
  if (!server) {
    STDERR_MSG("Wrong parameters");
    return -1;
  }

  return socket_set_non_blocking(server->socket_fd, on);
}

void tcp_server_destroy(TCPServer_t* server) {
  if (server) {
    for (ssize_t i = 0; i < TCP_MAX_CLIENTS; i++) {
      if (server->clients[i].socket_fd >= 0) {
        close(server->clients[i].socket_fd);
      }
    }

    if (server->socket_fd >= 0) {
      close(server->socket_fd);
    }

    free(server);
  }
}

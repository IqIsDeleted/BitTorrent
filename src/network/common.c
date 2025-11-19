#include "common.h"

int socket_set_non_blocking(int socket_fd, int on) {
  int flags = fcntl(socket_fd, F_GETFL, 0);
  if (flags < 0) {
    ERRNO_MSG("fcntl failed");
    return -1;
  }

  if (on) {
    flags |= O_NONBLOCK;
  } else {
    flags &= ~O_NONBLOCK;
  }

  if (fcntl(socket_fd, F_SETFL, flags) < 0) {
    ERRNO_MSG("fcntl failed");
    return -1;
  }

  return 0;
}

int tcp_send(TCPClient_t* client, const char* data, size_t data_size) {
  if (!client || !data || data_size == 0) {
    STDERR_MSG("Wrong parameters");
    return -1;
  }

  if (!client->connected) {
    STDERR_MSG("trying to send data while not connected");
    return -1;
  }

  size_t total_sent = 0;
  while (total_sent < data_size) {
    ssize_t sent =
        send(client->socket_fd, data + total_sent, data_size - total_sent, 0);
    if (sent < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // Wait for socket to be writable, but for simplicity, retry
        continue;
      }
      ERRNO_MSG("send failed");
      client->connected = 0;
      return -1;
    }
    if (sent == 0) {
      // Connection closed
      client->connected = 0;
      return -1;
    }
    total_sent += sent;
  }

  return total_sent;
}
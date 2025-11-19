#ifndef NETWORK_COMMON_H_
#define NETWORK_COMMON_H_

#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define ERRNO_MSG(msg) printf("[%s] <%s> %s\n", __func__, strerror(errno), msg)
#define STDERR_MSG(msg) fprintf(stderr, "[%s] %s\n", __func__, msg)

typedef struct TCPClient {
  int socket_fd;
  char ip[INET_ADDRSTRLEN];
  in_port_t port;
  int connected;
} TCPClient_t;

/**
 * @brief Enable non-blocking mode for the socket
 * @param socket_fd File descriptor of the socket
 * @param on `1` - enable, `0` - disable
 * @return `0` on success or `-1` on error
 */
int socket_set_non_blocking(int socket_fd, int on);

/**
 * @brief Send data to connected server
 * @param client pointer to Client struct
 * @param data self explanatory
 * @param data_size self explanatory
 * @return Amount of sent bytes,`0` on success or `-1` on error
 */
int tcp_send(TCPClient_t* client, const char* data, size_t data_size);

#endif  // NETWORK_COMMON_H_
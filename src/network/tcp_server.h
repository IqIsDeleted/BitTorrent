#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include <netinet/in.h>
#include <stddef.h>

#include "common.h"

#define TCP_MAX_CLIENTS 64

typedef struct TCPServer {
  int socket_fd;
  in_port_t port;
  TCPClient_t clients[TCP_MAX_CLIENTS];
  uint16_t client_count;
} TCPServer_t;

/**
 * @brief Create and initialize TCP server
 * @param port port on which will listen to
 * @return pointer to Server struct or `NULL` on error
 */
TCPServer_t* tcp_server_create(in_port_t port);

/**
 * @brief Enable listening on server socket
 * @param server pointer to Server struct
 * @param qsize waiting queue size
 * @return `0` on success or `-1` on error
 */
int tcp_server_listen(TCPServer_t* server, int qsize);

/**
 * @brief Accept incoming connection
 * @param server pointer to Server struct
 * @return pointer to Client struct or `NULL` on error
 */
TCPClient_t* tcp_server_accept(TCPServer_t* server);

/**
 * @brief Receive data from connected client
 * @param client pointer to Client struct
 * @param buffer self explanatory
 * @param buffer_size self explanatory
 * @return Amount of received bytes, `0` if connection is closed or `-1` on
 * error
 */
ssize_t tcp_server_receive(TCPClient_t* client, char* buffer,
                           size_t buffer_size);

/**
 * @brief Send data to connected client
 * @param client pointer to Client struct
 * @param data self explanatory
 * @param data_size self explanatory
 * @return Amount of sent bytes,`0` on success or `-1` on error
 */
int tcp_server_send(TCPClient_t* client, const char* data, size_t data_size);

/**
 * @brief Disconnect client
 * @param server pointer to Server struct
 * @param client pointer to Client struct
 * @return `0` on success or `-1` on error
 */
int tcp_server_disclient(TCPServer_t* server, TCPClient_t* client);

/**
 * @brief Enable non-blocking mode for the socket
 * @param server pointer to Server struct
 * @param on `1` - enable, `0` - disable
 * @return `0` on success or `-1` on error
 */
int tcp_server_set_non_blocking(const TCPServer_t* server, int on);

/**
 * @brief Close socket and free memory resources
 * @param server pointer to Server struct
 */
void tcp_server_destroy(TCPServer_t* server);

#endif  // TCP_SERVER_H_
#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_

#include <netinet/in.h>
#include <stddef.h>

#include "common.h"

/**
 * @brief Create and initialize TCP client
 * @return pointer to Client struct or `NULL` on error
 */
TCPClient_t* tcp_client_create(void);

/**
 * @brief Connect to TCP server
 * @param client pointer to Client struct
 * @param server_ip IP address of the server
 * @param server_port port of the server
 * @return `0` on success or `-1` on error
 */
int tcp_client_connect(TCPClient_t* client, const char* server_ip,
                       in_port_t server_port);

/**
 * @brief Send data to connected server
 * @param client pointer to Client struct
 * @param data self explanatory
 * @param data_size self explanatory
 * @return Amount of sent bytes,`0` on success or `-1` on error
 */
int tcp_client_send(TCPClient_t* client, const char* data, size_t data_size);

/**
 * @brief Receive data from connected server
 * @param client pointer to Client struct
 * @param buffer self explanatory
 * @param buffer_size self explanatory
 * @return Amount of received bytes, `0` if connection is closed or `-1` on
 * error
 */
int tcp_client_receive(TCPClient_t* client, char* buffer, size_t buffer_size);

/**
 * @brief Enable non-blocking mode for the socket
 * @param client pointer to Client struct
 * @param on `1` - enable, `0` - disable
 * @return `0` on success or `-1` on error
 */
int tcp_client_set_non_blocking(const TCPClient_t* client, int on);

/**
 * @brief Close socket and free memory resources
 * @param client pointer to Client struct
 */
void tcp_client_destroy(TCPClient_t* client);

#endif  // TCP_CLIENT_H_
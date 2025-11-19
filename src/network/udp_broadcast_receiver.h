#ifndef UDP_BROADCAST_RECEIVER_H_
#define UDP_BROADCAST_RECEIVER_H_

#include <stddef.h>

typedef struct {
  int socket_fd;
  int port;
} udp_broadcast_receiver_t;

/**
 * @brief Initializes UDP socket
 *
 * @param port will be listening on this port
 * @return pointer to the utility struct or `NULL` on error
 */
udp_broadcast_receiver_t* udp_broadcast_receiver_create(int);

/**
 * @brief Receive UDP broadcast message
 * @param  receiver Pointer to the utility struct
 * @param  buffer Memory buffer for the received message
 * @param  buffer_size Size of the memory buffer
 * @param  sender_ip Memory buffer for the IP address of the sender (optional)
 * @param  ip_buffer_size  Size of the IP address buffer (optional)
 * @return Amount of received bytes or `-1` on error
 */
int udp_broadcast_receiver_receive(udp_broadcast_receiver_t*, char*, size_t,
                                   char*, size_t);

/**
 * @brief Enable non-blocking mode for the socket
 * @param  receiver Pointer to the utility struct
 * @param  on `1` - enable, `0` - disable
 * @return `0` on success or `-1` on error
 */
int udp_broadcast_receiver_set_non_blocking(udp_broadcast_receiver_t*, int);

/**
 * @brief Set socket receive timeout
 * @param  receiver Pointer to the utility struct
 * @param  seconds self explanatory
 * @param  microseconds self explanatory
 * @return `0` on success or `-1` on error
 */
int udp_broadcast_receiver_set_timeout(udp_broadcast_receiver_t*, long, long);

/**
 * @brief Close socket and free memory resources
 * @param  broadcast pointer to the utility struct
 */
void udp_broadcast_receiver_destroy(udp_broadcast_receiver_t*);

#endif  // UDP_BROADCAST_RECEIVER_H_
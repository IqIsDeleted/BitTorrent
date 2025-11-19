#ifndef UDP_BROADCAST_H_
#define UDP_BROADCAST_H_

#include <stddef.h>

typedef struct {
  int socket_fd;
  int port;
} udp_broadcast_t;

/**
 * @brief Initializes UDP socket
 *
 * @param port where send broadcast's to
 * @return pointer to the utility struct or `NULL` on error
 */
udp_broadcast_t* udp_broadcast_create(int port);

/**
 * @brief Send broadcast message
 *
 * @param  broadcast pointer to the utility struct
 * @param  data message to send
 * @param  length sizeof of the `data` param
 * @return amount of sent bytes or `-1` on error
 */
int udp_broadcast_send(udp_broadcast_t*, const char*, size_t);

/**
 * @brief Close socket and free memory resources
 * @param  broadcast pointer to the utility struct
 */
void udp_broadcast_destroy(udp_broadcast_t*);

#endif  // UDP_BROADCAST_H_
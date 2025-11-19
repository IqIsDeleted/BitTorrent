#ifndef NETWORK_UTILS_H_
#define NETWORK_UTILS_H_

#include <arpa/inet.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Parses IP:port string format and extracts IP and port components
 *
 * Returns 0 on success, -1 on error.
 * On success: input_str is modified in-place (':' -> '\0'),
 * *port_out is set to parsed port (0-65535),
 * caller keeps the IP as the original pointer (now a NUL-terminated string).
 *
 * @param input_str The input string containing IP:port (will be modified)
 * @param port_out Pointer to store the parsed port number
 * @return 0 on success, -1 on error
 */
int split_ip_port(const char* input_str, int* port_out);

/**
 * @brief Gets the LAN IPv4 address of the current machine
 *
 * Returns dynamically allocated string containing LAN IPv4 address, or NULL on
 * error. Caller is responsible for freeing the returned string.
 *
 * @return Pointer to dynamically allocated string containing LAN IPv4 address,
 *         or NULL on error. Caller must free the returned string.
 */
char* get_lan_ipv4(void);

#endif  // NETWORK_UTILS_H_
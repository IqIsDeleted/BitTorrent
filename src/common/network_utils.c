#define _GNU_SOURCE
#include "network_utils.h"

int split_ip_port(const char* input_str, int* port_out) {
  if (!input_str || !port_out) {
    return -1;
  }

  char* colon = strchr(input_str, ':');
  if (!colon) {
    return -1;
  }

  /* replace ':' with NUL to terminate IP part */
  *colon = '\0';

  /* parse port */
  const char* port_str = colon + 1;
  if (*port_str == '\0') {
    return -1; /* empty port */
  }

  errno = 0;
  char* endptr;
  long p = strtol(port_str, &endptr, 10);
  if (errno == ERANGE || endptr == port_str || *endptr != '\0') {
    return -1;
  }
  if (p < 0 || p > 65535) {
    return -1;
  }

  *port_out = (int)p;
  return 0;
}

char* get_lan_ipv4(void) {
  struct ifaddrs *ifaddr, *ifa;
  char addrbuf[INET_ADDRSTRLEN];

  if (getifaddrs(&ifaddr) == -1) {
    return NULL;
  }

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL) {
      continue;
    }
    if (ifa->ifa_addr->sa_family == AF_INET) {
      if (ifa->ifa_flags & IFF_LOOPBACK) {
        continue;
      }
      struct sockaddr_in* sa = (struct sockaddr_in*)ifa->ifa_addr;
      if (inet_ntop(AF_INET, &sa->sin_addr, addrbuf, sizeof(addrbuf)) == NULL) {
        continue;
      }
      char* res = malloc(strlen(addrbuf) + 1);
      if (res) {
        strcpy(res, addrbuf);
      }
      freeifaddrs(ifaddr);
      return res;
    }
  }

  freeifaddrs(ifaddr);
  return NULL;
}
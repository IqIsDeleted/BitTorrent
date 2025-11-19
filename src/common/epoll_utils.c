#include "epoll_utils.h"

int add_to_epoll_common(int epoll_fd, int fd, struct epoll_event* event) {
  if (epoll_fd < 0 || fd < 0 || !event) {
    return -1;
  }
  return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, event);
}

int add_to_epoll(int epoll_fd, int fd) {
  struct epoll_event event = {0};
  event.events = EPOLLIN;
  event.data.fd = fd;

  return add_to_epoll_common(epoll_fd, fd, &event);
}

int add_to_epoll_ptr(int epoll_fd, int fd, void* ptr) {
  struct epoll_event event = {0};
  event.events = EPOLLIN;
  event.data.ptr = ptr;

  return add_to_epoll_common(epoll_fd, fd, &event);
}
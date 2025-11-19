#ifndef EPOLL_UTILS_H_
#define EPOLL_UTILS_H_

#include <sys/epoll.h>

/**
 * Common helper to add a file descriptor to epoll with a prepared event.
 * @param epoll_fd The epoll file descriptor.
 * @param fd The file descriptor to add.
 * @param event The epoll_event structure to use.
 * @return 0 on success, -1 on error or invalid input.
 */
int add_to_epoll_common(int epoll_fd, int fd, struct epoll_event* event);

/**
 * Adds a file descriptor to the epoll instance for reading.
 * @param epoll_fd The epoll file descriptor.
 * @param fd The file descriptor to add.
 * @return 0 on success, -1 on error or invalid input.
 */
int add_to_epoll(int epoll_fd, int fd);

/**
 * Adds a file descriptor to the epoll instance with associated pointer data.
 * @param epoll_fd The epoll file descriptor.
 * @param fd The file descriptor to add.
 * @param ptr The pointer to associate with the event.
 * @return 0 on success, -1 on error or invalid input.
 */
int add_to_epoll_ptr(int epoll_fd, int fd, void* ptr);

#endif  // EPOLL_UTILS_H_
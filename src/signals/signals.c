#define _POSIX_C_SOURCE 1
#include "signals.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int setup_signal_handlers() {
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT);
  sigaddset(&mask, SIGTERM);
  sigaddset(&mask, SIGPIPE);

  if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0) {
    return -1;
  }

  int signal_fd = signalfd(-1, &mask, SFD_NONBLOCK);
  if (signal_fd < 0) {
    return -1;
  }

  signal(SIGPIPE, SIG_IGN);

  return signal_fd;
}

int handle_signalfd_event(int fd, int* shutdown_requested) {
  struct signalfd_siginfo signal_info;
  ssize_t bytes_read;

  bytes_read = read(fd, &signal_info, sizeof(signal_info));
  if (bytes_read != sizeof(signal_info)) {
    return -1;
  }

  if (signal_info.ssi_signo == SIGINT || signal_info.ssi_signo == SIGTERM) {
    printf("\nShutting down...\n");
    *shutdown_requested = 1;
  }

  return 0;
}

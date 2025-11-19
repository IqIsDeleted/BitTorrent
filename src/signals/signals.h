#ifndef SIGNALS_SIGNALS_H
#define SIGNALS_SIGNALS_H

#include <signal.h>
#include <sys/signalfd.h>

/**
 * @brief Initialize signal handlers and create signalfd
 * @return On success, returns a valid signalfd file descriptor.
 *         On error, returns -1.
 */
int setup_signal_handlers();

/**
 * @brief Process a signal event from signalfd
 * @param fd Signalfd file descriptor returned by setup_signal_handlers()
 * @param shutdown_requested Pointer to integer flag that will be set to 1
 *                          if shutdown is requested (SIGINT/SIGTERM)
 * @return 0 on successful signal processing, -1 on error
 */

int handle_signalfd_event(int fd, int* shutdown_requested);

/**
 * @brief Get the signalfd file descriptor
 * @return The signalfd file descriptor, or -1 if not initialized
 * @note This function requires that setup_signal_handlers() has been called
 * first.
 */
int get_signal_fd(void);

#endif  // SIGNALS_SIGNALS_H

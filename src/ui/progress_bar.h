#ifndef UI_PROGRESS_BAR_H_
#define UI_PROGRESS_BAR_H_

#include <stdint.h>

/**
 * @brief Update progress bar display
 * @param total_bytes Total bytes to download
 * @param current_downloaded Currently downloaded bytes
 */
void update_progress_bar(uint64_t total_bytes, uint64_t current_downloaded);

#endif  // UI_PROGRESS_BAR_H_

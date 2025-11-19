#ifndef PATH_UTILS_H_
#define PATH_UTILS_H_

#include <stddef.h>

/**
 * @brief Builds a full path by concatenating data_path and filename.
 *
 * @param full_path Buffer to store the full path.
 * @param size Size of the full_path buffer.
 * @param data_path The directory path.
 * @param filename The filename.
 * @return 0 on success, -1 if the path is truncated or invalid.
 */
int build_full_path(char* full_path, size_t size, const char* data_path,
                    const char* filename);

#endif  // PATH_UTILS_H_
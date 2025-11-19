#include "path_utils.h"

#include <string.h>

int build_full_path(char* full_path, size_t size, const char* data_path,
                    const char* filename) {
  if (!full_path || !data_path || !filename || size == 0) {
    return -1;
  }

  strncpy(full_path, data_path, size - 1);
  full_path[size - 1] = '\0';
  size_t remaining = size - strlen(full_path) - 1;
  if (remaining > 0) {
    strncat(full_path, "/", remaining);
    remaining = size - strlen(full_path) - 1;
    if (remaining > 0) {
      strncat(full_path, filename, remaining);
    } else {
      return -1;
    }
  } else {
    return -1;
  }
  full_path[size - 1] = '\0';
  return 0;
}
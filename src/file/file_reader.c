#include "file_reader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Calculates the number of pieces.
 *
 * The function calculates the number of pieces in a file.
 * The calculation uses division with the last piece being calculated.
 *
 * @param file FILE pointer to the source file (must be open in binary mode)
 * @param file_size Output parameter for the total file size in bytes
 * @param piece_size Size of each piece in bytes
 *
 * @return Number of pieces on success, -1 on error
 */
static int calculate_pieces_count(FILE* file, uint64_t* file_size,
                                  uint32_t piece_size) {
  if (!file || !file_size || piece_size == 0) {
    fprintf(stderr, "Error: Invalid parameters.\n");
    return -1;
  }

  if (fseek(file, 0, SEEK_END) == -1) {
    perror("fseek error");
    return -1;
  }
  long size = ftell(file);
  if (size == -1) {
    perror("ftell error");
    return -1;
  }
  if (fseek(file, 0, SEEK_SET) == -1) {
    perror("fseek error");
    return -1;
  }

  *file_size = (uint64_t)size;
  return ((size + piece_size - 1) / piece_size);
}

/**
 * @brief Reads a specific piece from a file into provided buffer.
 *
 * @param filename Path to the source file.
 * @param piece_index Zero-based index of the piece to read.
 * @param piece_size Standard piece size in bytes.
 * @param output_buffer Pre-allocated buffer for the piece data.
 * @param buffer_size Size of the output buffer in bytes.
 * @param actual_size Output parameter for actual bytes read.
 *
 * @return If successful, returns 0.  It returns -1 on failure.
 *
 * @note The output_buffer is zero-initialized before reading.
 * @note For the last piece, actual_size may be less than piece_size.
 */
int read_file_piece(const char* filename, int piece_index, uint32_t piece_size,
                    uint8_t* output_buffer, size_t buffer_size,
                    uint32_t* actual_size) {
  if (!filename || piece_index < 0 || piece_size == 0) {
    return -1;
  }

  if (output_buffer && buffer_size < piece_size) {
    return -1;
  }

  FILE* file = fopen(filename, "rb");
  if (!file) {
    perror("fopen error.\n");
    return -1;
  }

  uint64_t file_size = 0;
  int pieces_count = calculate_pieces_count(file, &file_size, piece_size);

  if (pieces_count == -1 || piece_index >= pieces_count) {
    fclose(file);
    perror("calculate_pieces_count error.\n");
    return -1;
  }

  uint64_t offset = (uint64_t)piece_index * (uint64_t)piece_size;
  uint32_t current_piece_size = piece_size;

  if ((uint64_t)piece_index == (uint64_t)(pieces_count - 1)) {
    uint64_t remaining = file_size - offset;
    current_piece_size = (uint32_t)remaining;

    if (remaining > UINT32_MAX) {
      fclose(file);
      perror("remaining error.\n");
      return -1;
    }
  }

  if (output_buffer) {
    for (size_t i = 0; i < buffer_size; i++) {
      output_buffer[i] = 0;
    }
  }

  if (fseek(file, offset, SEEK_SET) != 0) {
    perror("fseek error.\n");
    fclose(file);
    return -1;
  }

  if (output_buffer) {
    size_t bytes_read = fread(output_buffer, 1, current_piece_size, file);
    if (bytes_read != current_piece_size) {
      perror("fread error.\n");
      fclose(file);
      return -1;
    }
  }

  if (actual_size) {
    *actual_size = current_piece_size;
  }

  fclose(file);
  return 0;
}
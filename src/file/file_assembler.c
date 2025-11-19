#include "file_assembler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static FILE* k_output_file = NULL;
static char* k_current_filename = NULL;

/**
 * @brief Initializes file assembly for a new file.
 *
 * @param output_filename Path to the output file to create/overwrite.
 * @param total_file_size Total expected size of the final file in bytes.
 *
 * @return If successful, returns 0.  It returns -1 on failure.
 *
 * @note Must be called first before any other functions in this module.
 * @note If the file exists, it will be truncated and overwritten.
 */
int file_assembler_init(const char* output_filename, uint64_t total_file_size) {
  if (!output_filename) {
    fprintf(stderr, "Error: Invalid filename\n");
    return -1;
  }

  k_current_filename = malloc(strlen(output_filename) + 1);
  if (!k_current_filename) {
    perror("malloc failed");
    return -1;
  }

  size_t i = 0;
  while (output_filename[i] != '\0') {
    k_current_filename[i] = output_filename[i];
    i++;
  }
  k_current_filename[i] = '\0';

  k_output_file = fopen(output_filename, "wb");
  if (!k_output_file) {
    perror("fopen failed");
    free(k_current_filename);
    k_current_filename = NULL;
    return -1;
  }

  if (ftruncate(fileno(k_output_file), total_file_size) != 0) {
    perror("ftruncate failed");
    fclose(k_output_file);
    free(k_current_filename);
    k_current_filename = NULL;
    k_output_file = NULL;
    return -1;
  }

  return 0;
}

/**
 * @brief Writes a single piece to its correct position in the file.
 *
 * @param piece_index Zero-based index of the piece (0, 1, 2, ...).
 * @param piece_data Pointer to the binary data of the piece.
 * @param piece_size Actual size of this piece in bytes (for last piece, may be
 * less than piece_length).
 * @param piece_length Standard piece length in bytes (used for offset
 * calculation).
 *
 * @return If successful, returns 0.  It returns -1 on failure.
 *
 * @note Must be called after file_assembler_init() and before
 * file_assembler().
 */
int write_piece_to_file(int piece_index, const uint8_t* piece_data,
                        uint32_t piece_size, uint32_t piece_length) {
  if (!k_output_file) {
    fprintf(stderr, "Error: File assembler not initialized\n");
    return -1;
  }

  if (!piece_data || piece_index < 0) {
    fprintf(stderr, "Error: Invalid piece parameters\n");
    return -1;
  }

  uint64_t offset = (uint64_t)piece_index * (uint64_t)piece_length;

  if (fseek(k_output_file, offset, SEEK_SET) != 0) {
    perror("fseek failed");
    return -1;
  }

  size_t written = fwrite(piece_data, 1, piece_size, k_output_file);
  if (written != piece_size) {
    perror("fwrite failed");
    return -1;
  }

  if (fflush(k_output_file) != 0) {
    perror("fflush failed");
    return -1;
  }

  return 0;
}

/**
 * @brief Finalizes file assembly and verifies completeness.
 *
 * @param expected_size Expected total size of the assembled file in bytes.
 *
 * @return If successful, returns 0.  It returns -1 on failure.
 *
 * @note Must be called after all pieces have been written via
 * write_piece_to_file().
 * @note After this call, no more pieces can be written to the file.
 */
int file_assembler(uint64_t expected_size) {
  if (!k_output_file) {
    fprintf(stderr, "Error: File assembler not initialized\n");
    return -1;
  }

  if (fseek(k_output_file, 0, SEEK_END) != 0) {
    perror("fseek failed");
    return -1;
  }

  long actual_size = ftell(k_output_file);
  if (actual_size == -1) {
    perror("ftell failed");
    fclose(k_output_file);
    k_output_file = NULL;
    free(k_current_filename);
    k_current_filename = NULL;
    return -1;
  }

  if ((uint64_t)actual_size != expected_size) {
    fprintf(stderr, "File size mismatch: expected %lu, got %ld\n",
            expected_size, actual_size);
    fclose(k_output_file);
    k_output_file = NULL;
    free(k_current_filename);
    k_current_filename = NULL;
    return -1;
  }

  if (fflush(k_output_file) != 0) {
    perror("fflush failed");
    fclose(k_output_file);
    k_output_file = NULL;
    free(k_current_filename);
    k_current_filename = NULL;
    return -1;
  }

  fclose(k_output_file);
  k_output_file = NULL;

  free(k_current_filename);
  k_current_filename = NULL;
  return 0;
}

/**
 * @brief Aborts file assembly and cleans up resources.
 *
 * @return If successful, returns 0.  It returns -1 on failure.
 *
 * @note Can be called at any time after file_assembler_init() to cancel
 * assembly.
 * @note Closes the file and removes the partially assembled file from disk.
 */
int file_assembler_abort() {
  if (k_output_file) {
    fclose(k_output_file);
    k_output_file = NULL;
  }

  if (k_current_filename) {
    if (remove(k_current_filename) != 0) {
      perror("remove failed");
      free(k_current_filename);
      k_current_filename = NULL;
      return -1;
    }
    free(k_current_filename);
    k_current_filename = NULL;
  }

  return 0;
}

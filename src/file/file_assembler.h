/**
 * @file file_assembler.h
 * @brief File assembly from downloaded pieces - disk management module
 *
 * @usage
 * The functions in this module must be called in the following sequence:
 * 1. file_assembler_init() - Initialize the output file
 * 2. write_piece_to_file() - Write pieces (0 or more times, in any order)
 * 3. file_assembler() - Finalize and verify the complete file
 *
 * If any error occurs during steps 1-2, call file_assembler_abort() to cleanup.
 */

#ifndef FILE_ASSEMBLER_H_
#define FILE_ASSEMBLER_H_

#include <stdint.h>

int file_assembler_init(const char* output_filename, uint64_t total_file_size);
int write_piece_to_file(int piece_index, const uint8_t* piece_data,
                        uint32_t piece_size, uint32_t piece_length);
int file_assembler(uint64_t expected_size);
int file_assembler_abort();

#endif
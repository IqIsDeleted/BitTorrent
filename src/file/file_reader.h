#ifndef FILE_READER_H_
#define FILE_READER_H_

#include <stddef.h>
#include <stdint.h>

int read_file_piece(const char* filename, int piece_index, uint32_t piece_size,
                    uint8_t* output_buffer, size_t buffer_size,
                    uint32_t* actual_size);

#endif  // FILE_READER_H_
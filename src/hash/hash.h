#ifndef HASH_HASH_H_
#define HASH_HASH_H_

#include <stddef.h>
#include <stdint.h>

#include "../bit_torrent.h"

/**
 * @brief Verify piece data against stored hash
 * @param torrent Torrent file structure
 * @param data Piece data to verify
 * @param length Data length
 * @param index Piece index
 * @return 1 if hashes match, 0 otherwise
 */
int verify_piece_hash(eltextorrent_file_t* torrent, const uint8_t* data,
                      size_t length, int index);

/**
 * @brief Compare two hashes for equality
 * @param hash1 First hash to compare
 * @param hash2 Second hash to compare
 * @return 1 if hashes are equal, 0 otherwise
 */
int compare_hashes(const uint8_t* hash1, const uint8_t* hash2);

#endif  // HASH_HASH_H_

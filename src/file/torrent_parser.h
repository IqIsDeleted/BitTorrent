#ifndef TORRENT_PARSER_H_
#define TORRENT_PARSER_H_

#include "../bit_torrent.h"

#define PIECE_SIZE_64KB (64 * 1024)

/**
 * @brief Frees and resets torrent structure resources.
 *
 * @param torrent Pointer to the torrent structure to free
 */
void torrent_free(eltextorrent_file_t* torrent);

/**
 * @brief Loads torrent structure from a .torrent file.
 *
 * @param torrent Pointer to the torrent structure to populate with loaded data.
 * @param torrent_filename Input filename for the torrent file.
 *
 * @return If successful, returns 0.  It returns -1 on failure.
 *
 * @note This function allocates memory for torrent->pieces_hashes.
 * @warning Caller must call torrent_free() to avoid memory leaks.
 *
 * @note This function cleans the torrent structure before loading.
 */
int torrent_loader(eltextorrent_file_t* torrent, const char* torrent_filename);

#endif  // TORRENT_PARSER_H_
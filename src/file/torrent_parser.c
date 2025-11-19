#include "torrent_parser.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void torrent_free(eltextorrent_file_t* torrent) {
  if (torrent) {
    free(torrent->pieces_hashes);

    for (int i = 0; i < HASH_SIZE; i++) {
      torrent->infohash[i] = 0;
    }
    torrent->file_size = 0;
    for (int i = 0; i < NAME_MAX + 1; i++) {
      torrent->name[i] = 0;
    }
    torrent->piece_size = 0;
    torrent->pieces_count = 0;
    torrent->pieces_hashes = NULL;
  }
}

int torrent_loader(eltextorrent_file_t* torrent, const char* torrent_filename) {
  if (!torrent || !torrent_filename) {
    fprintf(stderr, "Error: Invalid parameters\n");
    return -1;
  }
  FILE* file = NULL;
  file = fopen(torrent_filename, "rb");
  if (!file) {
    perror("fopen failed in torrent_load");
    return -1;
  }
  torrent_free(torrent);

  if (fread(torrent->infohash, 1, HASH_SIZE, file) != HASH_SIZE) {
    goto fread_error;
  }
  if (fread(&torrent->file_size, 1, sizeof(torrent->file_size), file) !=
      sizeof(torrent->file_size)) {
    goto fread_error;
  }
  if (fread(torrent->name, 1, NAME_MAX + 1, file) != NAME_MAX + 1) {
    goto fread_error;
  }
  if (fread(&torrent->piece_size, 1, sizeof(torrent->piece_size), file) !=
      sizeof(torrent->piece_size)) {
    goto fread_error;
  }
  if (fread(&torrent->pieces_count, 1, sizeof(torrent->pieces_count), file) !=
      sizeof(torrent->pieces_count)) {
    goto fread_error;
  }

  torrent->pieces_hashes = malloc(torrent->pieces_count * HASH_SIZE);
  if (!torrent->pieces_hashes) {
    perror("Malloc failed in torrent_loader.");
    fclose(file);
    return -1;
  }
  if (fread(torrent->pieces_hashes, 1, torrent->pieces_count * HASH_SIZE,
            file) != torrent->pieces_count * HASH_SIZE) {
    goto fread_error;
  }

  fclose(file);
  return 0;

fread_error:
  perror("fread failed in torrent_loader");
  torrent_free(torrent);
  fclose(file);
  return -1;
}

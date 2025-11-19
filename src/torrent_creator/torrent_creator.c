#define _GNU_SOURCE
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../bit_torrent.h"

#define PIECE_SIZE_64KB (64 * 1024)

/**
 * @brief Saves the torrent structure to a file.
 *
 * @param torrent Pointer to the torrent structure to save.
 * @param torrent_filename Output filename for the torrent file.
 * @return If successful, returns 0.  It returns -1 on failure.
 */
int torrent_save(const eltextorrent_file_t* torrent, const char* torrent_name) {
  char torrent_filename[NAME_MAX + 20];
  FILE* file = NULL;

  snprintf(torrent_filename, sizeof(torrent_filename), "%s.torrent",
           torrent_name);
  file = fopen(torrent_filename, "wb");
  if (!file) {
    return -1;
  }

  // Encode and write infohash as base64
  {
    u_char base64[HASH_SIZE + 3];
    EVP_EncodeBlock(base64, torrent->infohash, HASH_SIZE);
    base64[HASH_SIZE] = '\0';
    fwrite(base64, 1, strlen((char*)base64), file);
    printf("INFOHASH: [%s]\n", base64);
  }

  fwrite(&torrent->file_size, sizeof(torrent->file_size), 1, file);
  fwrite(torrent->name, 1, NAME_MAX + 1, file);
  fwrite(&torrent->piece_size, sizeof(torrent->piece_size), 1, file);
  fwrite(&torrent->pieces_count, sizeof(torrent->pieces_count), 1, file);

  // Encode and write pieces_hashes as base64
  for (uint32_t i = 0; i < torrent->pieces_count; i++) {
    u_char base64[HASH_SIZE + 3];
    EVP_EncodeBlock(base64, &torrent->pieces_hashes[i * HASH_SIZE], HASH_SIZE);
    base64[HASH_SIZE] = '\0';
    fwrite(base64, 1, strlen((char*)base64), file);
    printf("PIECE <%4u> HASH: [%s]\n", i, base64);
  }

  fclose(file);
  return 0;
}

/**
 * @brief Calculates the hash of one pieces.
 *
 * @param data Pointer to the input data buffer.
 * @param length Length of data in bytes.
 * @param output_hash Output buffer for the resulting hash.
 *
 * @return 0 on success, -1 on error
 */
int calculate_piece_hash(const uint8_t* data, size_t length,
                         uint8_t* output_hash) {
  EVP_MD_CTX* ctx = EVP_MD_CTX_new();
  int result = 0;
  unsigned int hash_len = HASH_SIZE;
  if (!ctx) {
    return -1;
  }

  if (EVP_DigestInit_ex(ctx, EVP_sha1(), NULL) != 1 ||
      EVP_DigestUpdate(ctx, data, length) != 1 ||
      EVP_DigestFinal_ex(ctx, output_hash, &hash_len) != 1) {
    result = -1;
  }

  EVP_MD_CTX_free(ctx);
  return result;
}

/**
 * @brief Calculates the info hash for the torrent file.
 *
 * @param torrent Pointer to the torrent structure.
 * @param infohash Output buffer for the resulting hash.
 *
 * @return 0 on success, -1 on error
 */
int calculate_infohash(const eltextorrent_file_t* torrent, uint8_t* infohash) {
  EVP_MD_CTX* ctx = EVP_MD_CTX_new();
  int result = 0;
  unsigned int hash_len = HASH_SIZE;
  if (!ctx) {
    return -1;
  }

  if (EVP_DigestInit_ex(ctx, EVP_sha1(), NULL) != 1 ||
      EVP_DigestUpdate(ctx, &torrent->file_size, sizeof(torrent->file_size)) !=
          1 ||
      EVP_DigestUpdate(ctx, torrent->name, strlen(torrent->name)) != 1 ||
      EVP_DigestUpdate(ctx, &torrent->piece_size,
                       sizeof(torrent->piece_size)) != 1 ||
      EVP_DigestUpdate(ctx, torrent->pieces_hashes,
                       torrent->pieces_count * HASH_SIZE) != 1 ||
      EVP_DigestFinal_ex(ctx, infohash, &hash_len) != 1) {
    result = -1;
  }

  EVP_MD_CTX_free(ctx);
  return result;
}

/**
 * @brief Frees resources allocated for a torrent structure.
 *
 * @param torrent Pointer to the torrent structure to free
 */
void torrent_free(eltextorrent_file_t* torrent) {
  if (torrent) {
    free(torrent->pieces_hashes);
    torrent->pieces_hashes = NULL;
  }
}

/**
 * @brief Calculates the number of pieces.
 *
 * The function calculates the number of pieces in a file.
 * The calculation uses division with the last piece being calculated.
 *
 * @param file FILE pointer to the source file (must be open in binary mode)
 * @param file_size Output parameter for the total file size in bytes
 * @param piece_size Size of each piece in bytes
 * @return Number of pieces on success, -1 on error
 */
int calculate_pieces_count(FILE* file, uint64_t* file_size,
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
 * @brief Creates a torrent file from a source file.
 *
 * @param filename Path to the source file.
 * @param piece_size Size of each piece in bytes.
 * @param torrent_name Name for the resulting torrent file.
 * @return If successful, returns 0.  It returns -1 on failure.
 */
int create_torrent_file(const char* filename, uint32_t piece_size,
                        const char* torrent_name) {
  FILE* file = fopen(filename, "rb");
  eltextorrent_file_t torrent = {0};
  int pieces_count = 0;
  uint64_t file_size;
  uint8_t* buffer = NULL;

  if (!file) {
    fprintf(stderr, "!file %s\n", filename);
    return -1;
  }

  pieces_count = calculate_pieces_count(file, &file_size, piece_size);
  if ((int)pieces_count == -1) {
    fclose(file);
    return -1;
  }

  torrent.file_size = file_size;
  torrent.piece_size = piece_size;
  torrent.pieces_count = (uint32_t)pieces_count;
  const char* base = strrchr(filename, '/');
  if (base) {
    base++;
  } else {
    base = filename;
  }
  strncpy(torrent.name, base, sizeof(torrent.name) - 1);
  torrent.name[sizeof(torrent.name) - 1] = '\0';

  torrent.pieces_hashes = malloc(pieces_count * HASH_SIZE);

  buffer = malloc(piece_size);
  if (!buffer || !torrent.pieces_hashes) {
    free(buffer);
    free(torrent.pieces_hashes);
    perror("Malloc error");
    fclose(file);
    return -1;
  }

  for (int i = 0; i < pieces_count; i++) {
    size_t bytes_read = fread(buffer, 1, piece_size, file);
    if (calculate_piece_hash(buffer, bytes_read,
                             &torrent.pieces_hashes[i * HASH_SIZE]) == -1) {
      fprintf(stderr, "Error calculating piece hash for piece.\n");
      free(buffer);
      torrent_free(&torrent);
      fclose(file);
      return -1;
    }
  }

  if (calculate_infohash(&torrent, torrent.infohash) == -1) {
    fprintf(stderr, "Error calculating infohash\n");
    free(buffer);
    torrent_free(&torrent);
    fclose(file);
    return -1;
  }

  if (torrent_save(&torrent, torrent_name) == -1) {
    fprintf(stderr, "The resulting file is corrupted.\n");
    free(buffer);
    torrent_free(&torrent);
    fclose(file);
    return -1;
  }

  free(buffer);
  torrent_free(&torrent);
  fclose(file);

  return 0;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Enter the path to the file as an argument.\n");
  }

  if (create_torrent_file(argv[1], PIECE_SIZE_64KB, "torrent_file") == 0) {
    printf("Torrent file created.\n");
  } else {
    printf("Error creating torrent file.\n");
  }

  return 0;
}

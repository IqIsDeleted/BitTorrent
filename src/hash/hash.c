#include "hash.h"

#include <openssl/err.h>
#include <openssl/evp.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void calculate_sha1_hash(const uint8_t* data, size_t length,
                                uint8_t* output_hash) {
  if (!data || !output_hash) {
    return;
  }

  if (length == 0) {
    return;
  }

  EVP_MD_CTX* sha1_ctx = EVP_MD_CTX_new();
  if (!sha1_ctx) {
    return;
  }

  if (1 != EVP_DigestInit_ex(sha1_ctx, EVP_sha1(), NULL)) {
    EVP_MD_CTX_free(sha1_ctx);
    return;
  }

  if (1 != EVP_DigestUpdate(sha1_ctx, data, length)) {
    EVP_MD_CTX_free(sha1_ctx);
    return;
  }

  if (1 != EVP_DigestFinal_ex(sha1_ctx, output_hash, NULL)) {
    EVP_MD_CTX_free(sha1_ctx);
    return;
  }

  EVP_MD_CTX_free(sha1_ctx);

  unsigned char base64[28];
  EVP_EncodeBlock(base64, output_hash, HASH_SIZE);
  base64[27] = '\0';
  memcpy(output_hash, base64, HASH_SIZE);
}

static const uint8_t* get_piece_hash(eltextorrent_file_t* torrent, int index) {
  if (index < 0) {
    return NULL;
  }

  if (!torrent || !torrent->pieces_hashes ||
      (uint32_t)index >= torrent->pieces_count) {
    return NULL;
  }

  return &torrent->pieces_hashes[index * HASH_SIZE];
}

int compare_hashes(const uint8_t* hash1, const uint8_t* hash2) {
  if (hash1 == NULL || hash2 == NULL) {
    return 0;
  }

  return memcmp(hash1, hash2, HASH_SIZE) == 0;
}

int verify_piece_hash(eltextorrent_file_t* torrent, const uint8_t* data,
                      size_t length, int piece_index) {
  if (piece_index < 0) {
    fprintf(stderr, "[verify_piece_hash] Bad piece index\n");
    return 0;
  }

  if (!torrent || !data || length == 0 ||
      (uint32_t)piece_index >= torrent->pieces_count) {
    fprintf(stderr, "[verify_piece_hash] Bad args\n");
    return 0;
  }

  uint8_t calculated_hash[HASH_SIZE + 1];

  calculate_sha1_hash(data, length, calculated_hash);

  const uint8_t* piece_hash = get_piece_hash(torrent, piece_index);
  if (!piece_hash) {
    fprintf(stderr, "[verify_piece_hash] no piece_hash\n");
    return 0;
  }

  return compare_hashes(calculated_hash, piece_hash);
}

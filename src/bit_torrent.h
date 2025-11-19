#ifndef BIT_TORRENT_H_
#define BIT_TORRENT_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define HASH_SIZE 20
#define NAME_MAX 255
#define MAX_EPOLL_EVENTS 128
#define PIECE_INDEX_BUF_SIZE 32
#define SEEDER_TCP_PORT 6000
#define UDP_BROADCAST_PORT 5001
#define UDP_RECEIVE_PORT 5000
#define EPOLL_TIMEOUT_MS 1000
#define TIMER_INTERVAL_SEC 1
#define NETWORK_BUFFER_SIZE 1024

struct seeder_info {
  int fd;
  int invalid_request;
};

typedef struct dl_list_node {
  struct seeder_info data;
  struct dl_list_node* prev;
  struct dl_list_node* next;
} dl_list_node_t;

typedef struct double_linked_list {
  dl_list_node_t* head;
  dl_list_node_t* tail;
  size_t size;
} dl_list_t;

/**
 * @brief Torrent file metadata structure
 */
typedef struct {
  uint8_t infohash[HASH_SIZE]; /**< Torrent file hash for peer discovery */
  uint64_t file_size;          /**< Original file size in bytes */
  char name[NAME_MAX + 1];     /**< Original filename */
  uint32_t piece_size;         /**< Piece size in bytes (64KB) */
  uint32_t pieces_count;       /**< Total number of pieces */
  uint8_t* pieces_hashes;      /**< Array of piece SHA1 hashes (pieces_count *
                                  HASH_SIZE) */
} eltextorrent_file_t;

struct piece {
  int index;
  uint32_t piece_size;
  uint8_t* pieces;
};

typedef struct queue_node {
  struct piece data;
  struct queue_node* next;
} queue_node_t;

typedef struct {
  queue_node_t* head;
  queue_node_t* tail;
  size_t size;
} queue_t;

#endif  // BIT_TORRENT_H_

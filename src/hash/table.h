/**
 * @file table.h
 * @brief Hashtable module for managing leechers (peers) in the BitTorrent
 * application.
 *
 * This module provides a hashtable implementation using uthash to store and
 * manage information about connected leechers (peers) identified by their IP
 * addresses. It supports adding, finding, deleting, and cleaning up leech
 * entries.
 */

#ifndef HASH_TABLE_H_
#define HASH_TABLE_H_

#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>

#include "../thirdparty/uthash.h"

/**
 * @struct Leechees_t
 * @brief Structure representing a leecher (peer) in the hashtable.
 *
 * Each entry in the hashtable represents a connected peer identified by their
 * IP address. The structure uses uthash for efficient hash-based storage and
 * lookup.
 */
typedef struct Leechees {
  char id[INET_ADDRSTRLEN]; /**< IP address of the leecher */
  UT_hash_handle hh;        /**< uthash handle for hash table operations */
} Leechees_t;

/**
 * @brief Add a leecher to the hashtable or update existing entry.
 *
 * If the IP address is not already in the hashtable, a new entry is created.
 * If it exists, the entry is updated with the new IP address.
 *
 * @param leechees Pointer to the hashtable head pointer.
 * @param ipaddr IP address string to add (must be null-terminated).
 */
void add_leech(Leechees_t** leechees, const char* ipaddr);

/**
 * @brief Find a leecher in the hashtable by IP address.
 *
 * Searches the hashtable for an entry matching the given IP address.
 *
 * @param leechees Pointer to the hashtable head pointer.
 * @param ipaddr IP address string to search for (must be null-terminated).
 * @return Pointer to the found Leechees_t entry, or NULL if not found.
 */
Leechees_t* find_leech(Leechees_t** leechees, const char* ipaddr);

/**
 * @brief Delete a specific leecher from the hashtable.
 *
 * Removes the specified entry from the hashtable and frees its memory.
 *
 * @param leechees Pointer to the hashtable head pointer.
 * @param leech Pointer to the Leechees_t entry to delete.
 */
void delete_leech(Leechees_t** leechees, Leechees_t* leech);

/**
 * @brief Clean up and free all entries in the hashtable.
 *
 * Iterates through the entire hashtable, deleting and freeing all entries.
 * After this operation, the hashtable is empty.
 *
 * @param leechees Pointer to the hashtable head pointer.
 */
void clean_hashtable(Leechees_t** leechees);

#endif  // HASH_TABLE_H_
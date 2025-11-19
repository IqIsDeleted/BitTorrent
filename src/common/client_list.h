/**
 * @file client_list.h
 * @brief Header file for managing a linked list of TCP clients.
 *
 * This module provides functions to create, manipulate, and destroy a linked
 * list of TCP clients for use in network applications.
 */

#ifndef CLIENT_LIST_H_
#define CLIENT_LIST_H_

#include "../network/tcp_client.h"

/**
 * @brief Node in a linked list of TCP clients.
 *
 * Each node contains a pointer to a TCPClient_t and a pointer to the next node
 * in the list.
 */
typedef struct ClientNode {
  TCPClient_t* client;     /**< Pointer to the TCP client. */
  struct ClientNode* next; /**< Pointer to the next node in the list. */
} ClientNode;

/**
 * @brief Creates a new empty client list.
 *
 * @return Pointer to the head of the list (NULL for empty list).
 */
ClientNode* client_list_create();

/**
 * @brief Destroys the entire client list, freeing all nodes and disconnecting
 * clients.
 *
 * This function iterates through the list, destroys each TCP client, and frees
 * the memory allocated for each node.
 *
 * @param head Pointer to the head of the list.
 */
void client_list_destroy(ClientNode* head);

/**
 * @brief Adds a new client to the front of the list.
 *
 * @param head Current head of the list.
 * @param client Pointer to the TCPClient_t to add.
 * @return New head of the list.
 */
ClientNode* client_list_add(ClientNode* head, TCPClient_t* client);

/**
 * @brief Removes a client from the list.
 *
 * @param head Current head of the list.
 * @param client Pointer to the TCPClient_t to remove.
 * @return New head of the list.
 */
ClientNode* client_list_remove(ClientNode* head, const TCPClient_t* client);

/**
 * @brief Finds a client by file descriptor.
 *
 * @param head Head of the list.
 * @param fd File descriptor to search for.
 * @return Pointer to the TCPClient_t if found, NULL otherwise.
 */
TCPClient_t* client_list_find(ClientNode* head, int fd);

#endif  // CLIENT_LIST_H_

#include "client_list.h"

#include <stdlib.h>

ClientNode* client_list_create() { return NULL; }

void client_list_destroy(ClientNode* head) {
  while (head) {
    ClientNode* temp = head;
    head = head->next;
    tcp_client_destroy(temp->client);
    free(temp);
  }
}

ClientNode* client_list_add(ClientNode* head, TCPClient_t* client) {
  ClientNode* new_node = malloc(sizeof(ClientNode));
  if (!new_node) {
    fprintf(stderr, "[client_list_add] Failed to allocate new memory\n");
    return head;
  }
  new_node->client = client;
  new_node->next = head;
  return new_node;
}

ClientNode* client_list_remove(ClientNode* head, const TCPClient_t* client) {
  if (!head) {
    return NULL;
  }
  if (head->client == client) {
    ClientNode* new_head = head->next;
    free(head);
    return new_head;
  }
  ClientNode* current = head;
  while (current->next && current->next->client != client) {
    current = current->next;
  }
  if (current->next) {
    ClientNode* to_remove = current->next;
    current->next = to_remove->next;
    free(to_remove);
  }
  return head;
}

TCPClient_t* client_list_find(ClientNode* head, int fd) {
  ClientNode* current = head;
  while (current) {
    if (current->client->socket_fd == fd) {
      return current->client;
    }
    current = current->next;
  }
  return NULL;
}

#include "table.h"

#include <stdlib.h>
#include <string.h>

void add_leech(Leechees_t** leechees, const char* ipaddr) {
  Leechees_t* s;

  HASH_FIND_STR(*leechees, ipaddr, s);
  if (s == NULL) {
    s = malloc(sizeof *s);
    if (s == NULL) {
      return;
    }
    strncpy(s->id, ipaddr, INET_ADDRSTRLEN);
    s->id[INET_ADDRSTRLEN - 1] = '\0';
    HASH_ADD_STR(*leechees, id, s);
  }
}

Leechees_t* find_leech(Leechees_t** leechees, const char* ipaddr) {
  Leechees_t* s;
  HASH_FIND_STR(*leechees, ipaddr, s);
  return s;
}

void delete_leech(Leechees_t** leechees, Leechees_t* leech) {
  HASH_DEL(*leechees, leech);
  free(leech);
}

void clean_hashtable(Leechees_t** leechees) {
  Leechees_t *current_leech, *tmp;

  HASH_ITER(hh, *leechees, current_leech, tmp) {
    HASH_DEL(*leechees, current_leech);
    free(current_leech);
  }
}
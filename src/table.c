#include <assert.h>
#include <stdlib.h>

#include "arcana.h"

struct arcana_token_table {
  size_t len;
  size_t cap;
};

#define TABLE_DEFAULT_LEN 1

arcana_token_table_t *arcana_token_table_init() {
  size_t len =
      sizeof(char *) * TABLE_DEFAULT_LEN + sizeof(arcana_token_table_t);
  arcana_token_table_t *table = malloc(len);

  if (!table)
    return NULL;

  table->cap = TABLE_DEFAULT_LEN;
  table->len = 0;
  return table;
}

void arcana_token_table_deinit(arcana_token_table_t *table) { free(table); }

size_t arcana_token_table_len(arcana_token_table_t *table) {
  assert(table);
  return table->len;
}

const char **arcana_token_table_data(arcana_token_table_t *table) {
  assert(table);
  return (const char **)(table + 1);
}

void arcana_token_table_push(arcana_token_table_t **table, const char *sym) {
  assert(table);
  assert(*table);

  arcana_token_table_t *t = *table;

  if (t->len >= t->cap) {
    t->cap *= 2;
    t = realloc(t,
                sizeof(arcana_token_table_t) + sizeof(const char *) * t->cap);
    *table = t;
  }

  *(arcana_token_table_data(t) + t->len) = sym;
  t->len += 1;
}

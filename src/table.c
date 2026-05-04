#include <assert.h>
#include <stdlib.h>

#include "arcana.h"
#include "types.h"

#define TABLE_DEFAULT_LEN 1

arcana_table *arcana_table_init() {
  size_t len = sizeof(char *) * TABLE_DEFAULT_LEN + sizeof(arcana_table);
  arcana_table *table = malloc(len);

  if (!table)
    return NULL;

  table->cap = TABLE_DEFAULT_LEN;
  table->len = 0;
  return table;
}

void arcana_table_deinit(arcana_table *table) { free(table); }

size_t arcana_table_len(arcana_table *table) {
  assert(table);
  return table->len;
}

const char **arcana_table_data(arcana_table *table) {
  assert(table);
  return (const char **)(table + 1);
}

void arcana_table_push(arcana_table **table, const char *sym) {
  assert(table);
  assert(*table);

  arcana_table *t = *table;

  if (t->len >= t->cap) {
    t->cap *= 2;
    t = realloc(t, sizeof(arcana_table) + sizeof(const char *) * t->cap);
    *table = t;
  }

  *(arcana_table_data(t) + t->len) = sym;
  t->len += 1;
}

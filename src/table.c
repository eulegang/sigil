#include <assert.h>
#include <stdlib.h>

#include "sigil.h"
#include "types.h"

#define TABLE_DEFAULT_LEN 1

sigil_table *sigil_table_init() {
  size_t len = sizeof(char *) * TABLE_DEFAULT_LEN + sizeof(sigil_table);
  sigil_table *table = malloc(len);

  if (!table)
    return NULL;

  table->cap = TABLE_DEFAULT_LEN;
  table->len = 0;
  return table;
}

void sigil_table_deinit(sigil_table *table) { free(table); }

size_t sigil_table_len(sigil_table *table) {
  assert(table);
  return table->len;
}

const char **sigil_table_data(sigil_table *table) {
  assert(table);
  return (const char **)(table + 1);
}

void sigil_table_push(sigil_table **table, const char *sym) {
  assert(table);
  assert(*table);

  sigil_table *t = *table;

  if (t->len >= t->cap) {
    t->cap *= 2;
    t = realloc(t, sizeof(sigil_table) + sizeof(const char *) * t->cap);
    *table = t;
  }

  *(sigil_table_data(t) + t->len) = sym;
  t->len += 1;
}

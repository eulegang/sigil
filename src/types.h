#ifndef _arcana_types_H
#define _arcana_types_H

#include "arcana.h"
#include <stddef.h>
#include <stdint.h>

struct arcana_ast {
  size_t len;
  size_t cap;
  uint16_t nodes;
  uint16_t data;
};

struct arcana_table {
  size_t len;
  size_t cap;
};

struct arcana_parser {
  size_t len;
  arcana_terminal terminal;
  arcana_subparser init;
};

#endif

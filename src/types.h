#ifndef _sigil_types_H
#define _sigil_types_H

#include "sigil.h"
#include <stddef.h>
#include <stdint.h>

struct sigil_ast {
  size_t len;
  size_t cap;
  uint16_t nodes;
  uint16_t data;
};

struct sigil_table {
  size_t len;
  size_t cap;
};

struct sigil_parser {
  size_t len;
  sigil_terminal terminal;
  sigil_subparser init;
};

struct sigil_overlay {
  size_t len;
  uint16_t entries;
  uint16_t data;
};

#endif

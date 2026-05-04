#ifndef _MONKEY_H
#define _MONKEY_H

#include "arcana.h"
#include <cstddef>
#include <cstdint>

extern arcana_parser *monkey_parser;
struct monkey_slice {
  uint16_t base;
  uint16_t len;
};

enum class monkey_token_type : uint32_t {
  let,
  assign,
  semi,
  ident,
  number,

  lparen,
  rparen,
  plus,
  minus,
  mult,
  div,
};

enum class monkey_node_type : uint16_t {
  let,
  ident,
  lit,

  sub,
  add,
  mult,
  div,
};

enum class monkey_perc : size_t {
  LOWEST,

  SUM,
  PROD,

  HIGHEST,
};

ssize_t monkey_tokenizer(size_t cur, arcana_slice content,
                         arcana_token_type *type);

arcana_state monkey_parse_file(arcana_state);

void monkey_debug_tree(arcana_node node, void *data, size_t level,
                       arcana_slice content, void *);

void monkey_init_system(void) __attribute__((constructor));
void monkey_deinit_system(void) __attribute__((destructor));

#endif

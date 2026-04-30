#ifndef _MONKEY_H
#define _MONKEY_H

#include "arcana.h"
#include <cstddef>
#include <cstdint>

struct monkey_slice {
  uint16_t base;
  uint16_t len;
};

enum monkey_token_type : uint32_t {
  monkey_token_type_let,
  monkey_token_type_assign,
  monkey_token_type_semi,
  monkey_token_type_ident,
  monkey_token_type_number,

  monkey_token_type_lparen,
  monkey_token_type_rparen,
  monkey_token_type_plus,
  monkey_token_type_minus,
  monkey_token_type_mult,
  monkey_token_type_div,

};

enum class monkey_node_type : uint16_t {
  let,
  ident,
  lit,
};

ssize_t monkey_tokenizer(size_t cur, arcana_slice content,
                         arcana_token_type *type);

arcana_parser_state monkey_parse_file(arcana_parser_state);

void monkey_debug_tree(arcana_parse_node node, void *data, size_t level,
                       arcana_slice content);

#endif

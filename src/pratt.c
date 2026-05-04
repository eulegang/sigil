#include "arcana.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define SLOT_PREFIX 0
#define SLOT_INFIX 1
#define SLOT_POSTFIX 2
#define SLOT_PERC 3

#define slot(base, section, T) (T *)(base + 1) + (section * sizeof(T))

struct arcana_parser_pratt {
  size_t len;
  bool (*is_terminal)(arcana_token_type);
};

arcana_parser_pratt *
arcana_parser_pratt_init(size_t cap, bool (*is_terminal)(arcana_token_type)) {
  size_t len = sizeof(arcana_parser_pratt) +
               4 * cap * sizeof(arcana_parser_pratt_prefix_parser);

  arcana_parser_pratt *res = malloc(len);
  if (res == NULL)
    return res;

  res->len = len;
  res->is_terminal = is_terminal;
  memset(res + 1, 0, len - sizeof(arcana_parser_pratt));

  return res;
}

void arcana_parser_pratt_deinit(arcana_parser_pratt *self) { free(self); }

void arcana_parser_pratt_add_prefix(arcana_parser_pratt *self,
                                    arcana_token_type token,
                                    arcana_parser_pratt_prefix_parser parser) {

  assert(token < self->len);

  arcana_parser_pratt_prefix_parser *base =
      slot(self, SLOT_PREFIX, arcana_parser_pratt_prefix_parser);
  base[token] = parser;
}

void arcana_parser_pratt_add_infix(arcana_parser_pratt *self,
                                   arcana_token_type token,
                                   arcana_parser_pratt_infix_parser parser) {

  assert(token < self->len);

  arcana_parser_pratt_infix_parser *base =
      slot(self, SLOT_INFIX, arcana_parser_pratt_infix_parser);
  base[token] = parser;
}

void arcana_parser_pratt_add_postfix(
    arcana_parser_pratt *self, arcana_token_type token,
    arcana_parser_pratt_postfix_parser parser) {

  assert(token < self->len);

  arcana_parser_pratt_postfix_parser *base =
      slot(self, SLOT_POSTFIX, arcana_parser_pratt_postfix_parser);
  base[token] = parser;
}

void arcana_parser_pratt_add_precedence(arcana_parser_pratt *self,
                                        arcana_token_type token, size_t perc) {
  assert(token < self->len);

  size_t *base = slot(self, SLOT_PERC, size_t);
  base[token] = perc;
}

size_t arcana_parser_pratt_precedence(arcana_parser_pratt *self,
                                      arcana_token_type token) {
  assert(token < self->len);

  size_t *base = slot(self, SLOT_PERC, size_t);
  return base[token];
}

arcana_parser_state arcana_parser_pratt_parse(arcana_parser_pratt *self,
                                              arcana_parser_state state,
                                              size_t perc) {
  arcana_token token = arcana_parser_token(state);

  arcana_parser_pratt_postfix_parser *postfix =
      slot(self, SLOT_POSTFIX, arcana_parser_pratt_postfix_parser) + token.type;

  if (*postfix) {
    return (*postfix)(state);
  }

  arcana_parser_pratt_prefix_parser *prefix =
      slot(self, SLOT_PREFIX, arcana_parser_pratt_prefix_parser) + token.type;

  if (!*prefix) {
    state.status |= 1;
    return state;
  }

  state = (*prefix)(state);

  arcana_token peek_token = arcana_parser_peek_token(state, 0);
  size_t cur_perc = *(slot(self, SLOT_PERC, size_t) + peek_token.type);
  while (!self->is_terminal(peek_token.type) && perc < cur_perc) {

    arcana_parser_pratt_infix_parser *infix =
        slot(self, SLOT_INFIX, arcana_parser_pratt_infix_parser) +
        peek_token.type;

    if (!*infix) {
      return state;
    }

    state = (*infix)(state, state.subroot);

    if (state.status) {
      return state;
    }

    peek_token = arcana_parser_peek_token(state, 0);
    cur_perc = *(slot(self, SLOT_PERC, size_t) + peek_token.type);
  }

  return state;
}

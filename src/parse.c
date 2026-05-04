#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "arcana.h"
#include "types.h"

arcana_parser *arcana_parser_init(size_t cap, arcana_terminal is_terminal,
                                  arcana_subparser init) {
  size_t len = sizeof(arcana_parser) + 4 * cap * sizeof(arcana_subparser);

  arcana_parser *res = malloc(len);
  if (res == NULL)
    return res;

  res->len = len;
  res->terminal = is_terminal;
  res->init = init;

  memset(res + 1, 0, len - sizeof(arcana_parser));

  return res;
}

void arcana_parser_deinit(arcana_parser *self) { free(self); }

arcana_parser_slot *arcana_parser_slots(arcana_parser *self) {
  return (arcana_parser_slot *)(self + 1);
}

arcana_state arcana_parser_parse_expr(arcana_parser *self, arcana_state state,
                                      size_t perc) {
  arcana_token token = arcana_state_token(state);

  arcana_parser_slot *slot = arcana_parser_slots(self) + token.type;

  if (slot->postfix) {
    return (slot->postfix)(state);
  }

  if (!slot->prefix) {
    state.status |= 1;
    return state;
  }

  state = (slot->prefix)(state);

  arcana_token peek_token = arcana_state_peek(state, 0);
  arcana_parser_slot *peek_slot = arcana_parser_slots(self) + peek_token.type;
  while (!self->terminal(peek_token.type) && perc < peek_slot->perc) {
    if (!peek_slot->infix) {
      return state;
    }

    state = (peek_slot->infix)(state, state.subroot);

    if (state.status) {
      return state;
    }

    peek_token = arcana_state_peek(state, 0);
    peek_slot = arcana_parser_slots(self) + peek_token.type;
  }

  return state;
}

arcana_ast *arcana_parser_parse(arcana_parser *parser, arcana_tokens *tokens) {
  size_t len = arcana_pages * getpagesize();

  arcana_ast *res =
      mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

  if (res == MAP_FAILED) {
    return NULL;
  }

  res->len = 0;
  res->cap = len;

  arcana_state state = {
      .tokens = tokens,
      .ast = res,

      .token_cursor = 0,
      .node_cursor = 0,
      .data_cursor = 0,
      .subroot = 0,
      .status = 0,
  };

  arcana_state last = parser->init(state);

  if (last.status) {
    arcana_ast_deinit(res);
    return NULL;
  }

  if (!arcana_state_done(last)) {
    arcana_ast_deinit(res);
    return NULL;
  }

  res->nodes = last.node_cursor;
  res->data = last.data_cursor;

  return res;
}

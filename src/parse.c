#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "sigil.h"
#include "types.h"

sigil_parser *sigil_parser_init(size_t cap, sigil_terminal is_terminal,
                                sigil_subparser init) {
  size_t len = sizeof(sigil_parser) + 4 * cap * sizeof(sigil_subparser);

  sigil_parser *res = malloc(len);
  if (res == NULL)
    return res;

  res->len = len;
  res->terminal = is_terminal;
  res->init = init;

  memset(res + 1, 0, len - sizeof(sigil_parser));

  return res;
}

void sigil_parser_deinit(sigil_parser *self) { free(self); }

sigil_parser_slot *sigil_parser_slots(sigil_parser *self) {
  return (sigil_parser_slot *)(self + 1);
}

sigil_state sigil_parser_parse_expr(sigil_parser *self, sigil_state state,
                                    size_t perc) {
  sigil_token token = sigil_state_token(state);

  sigil_parser_slot *slot = sigil_parser_slots(self) + token.type;

  if (slot->postfix) {
    return (slot->postfix)(state);
  }

  if (!slot->prefix) {
    state.status |= 1;
    return state;
  }

  state = (slot->prefix)(state);

  sigil_token peek_token = sigil_state_peek(state, 0);
  sigil_parser_slot *peek_slot = sigil_parser_slots(self) + peek_token.type;
  while (!self->terminal(peek_token.type) && perc < peek_slot->perc) {
    if (!peek_slot->infix) {
      return state;
    }

    state = (peek_slot->infix)(state, state.subroot);

    if (state.status) {
      return state;
    }

    peek_token = sigil_state_peek(state, 0);
    peek_slot = sigil_parser_slots(self) + peek_token.type;
  }

  return state;
}

sigil_ast *sigil_parser_parse(sigil_parser *parser, sigil_tokens *tokens,
                              sigil_parser_error *error) {
  size_t len = sigil_pages * getpagesize();

  sigil_ast *res =
      mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

  if (res == MAP_FAILED) {
    return NULL;
  }

  res->len = 0;
  res->cap = len;

  sigil_state state = {
      .tokens = tokens,
      .ast = res,

      .token_cursor = 0,
      .node_cursor = 1,
      .data_cursor = 0,
      .subroot = 0,
      .status = 0,
  };

  sigil_state last = parser->init(state);

  if (last.status) {
    if (error) {
      error->status = last.status;
      error->token = last.token_cursor;
    }

    sigil_ast_deinit(res);
    return NULL;
  }

  if (!sigil_state_done(last)) {
    if (error) {
      error->status = last.status;
      error->token = last.token_cursor;
    }

    sigil_ast_deinit(res);
    return NULL;
  }

  res->nodes = last.node_cursor;
  res->data = last.data_cursor;

  return res;
}

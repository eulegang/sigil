#include "sigil.h"
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "types.h"

sigil_token sigil_state_token(sigil_state state) {
  return sigil_tokens_data(state.tokens)[state.token_cursor];
}

sigil_token sigil_state_peek(sigil_state state, size_t off) {
  return sigil_tokens_data(state.tokens)[state.token_cursor + off];
}

sigil_state sigil_state_expect_token(sigil_state state,
                                     sigil_token_type token_type) {
  sigil_token token = sigil_state_token(state);
  if (token.type != token_type) {
    state.status = 1;
  }
  return state;
}

uint16_t sigil_state_malloc(sigil_state *state, size_t size) {
  uint16_t cursor = state->data_cursor;
  state->data_cursor += size;
  return cursor;
}

uint16_t sigil_state_alloc_node(sigil_state *state) {
  return state->node_cursor++;
}

sigil_node *sigil_state_node(sigil_state state, uint16_t idx) {
  return sigil_ast_nodes(state.ast) + idx;
}

void *sigil_state_data(sigil_state state, uint16_t idx) {
  return (void *)((char *)sigil_ast_data(state.ast) + idx);
}

void sigil_state_next(sigil_state *state) {
  if (state->token_cursor >= sigil_tokens_len(state->tokens)) {
    state->status |= 2;
  } else {
    state->token_cursor += 1;
  }
}

bool sigil_state_done(sigil_state state) {
  return state.token_cursor >= sigil_tokens_len(state.tokens);
}

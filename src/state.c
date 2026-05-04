#include "arcana.h"
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "types.h"

arcana_token arcana_state_token(arcana_state state) {
  return arcana_tokens_data(state.tokens)[state.token_cursor];
}

arcana_token arcana_state_peek(arcana_state state, size_t off) {
  return arcana_tokens_data(state.tokens)[state.token_cursor + off];
}

arcana_state arcana_state_expect_token(arcana_state state,
                                       arcana_token_type token_type) {
  arcana_token token = arcana_state_token(state);
  if (token.type != token_type) {
    state.status = 1;
  }
  return state;
}

uint16_t arcana_state_malloc(arcana_state *state, size_t size) {
  uint16_t cursor = state->data_cursor;
  state->data_cursor += size;
  return cursor;
}

uint16_t arcana_state_alloc_node(arcana_state *state) {
  return state->node_cursor++;
}

void arcana_state_next(arcana_state *state) {
  if (state->token_cursor >= arcana_tokens_len(state->tokens)) {
    state->status |= 2;
  } else {
    state->token_cursor += 1;
  }
}

bool arcana_state_done(arcana_state state) {
  return state.token_cursor >= arcana_tokens_len(state.tokens);
}

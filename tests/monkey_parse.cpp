
#include "monkey.h"
#include "sigil.h"
#include <cstdint>

#include <cstdio>

#ifndef NDEBUG
#define debug(msg, ...)                                                        \
  do {                                                                         \
    printf(msg "\n" __VA_OPT__(, ) __VA_ARGS__);                               \
  } while (0)

#else
#define debug(msg, ...)
#endif

#define token(T) (sigil_token_type) monkey_token_type::T

sigil_parser *monkey_parser;

sigil_state monkey_parse_let(sigil_state state) {
  sigil_node *nodes = sigil_ast_nodes(state.ast);
  void *data = sigil_ast_data(state.ast);

  uint16_t let_node = sigil_state_alloc_node(&state);
  uint16_t ident_node = sigil_state_alloc_node(&state);

  uint16_t ident_addr = sigil_state_malloc(&state, sizeof(monkey_slice));

  if (state.status) {
    return state;
  }

  nodes[let_node] = {
      .child = ident_node,
      .next = 0,
      .offset = 0xFFFF,
      .type = (uint16_t)monkey_node_type::let,
  };

  nodes[ident_node] = {
      .child = 0,
      .next = 0,
      .offset = ident_addr,
      .type = (uint16_t)monkey_node_type::ident,
  };

  state = sigil_state_expect_token(state, (uint16_t)monkey_token_type::let);
  if (state.status) {
    return state;
  }

  sigil_state_next(&state);
  if (state.status) {
    return state;
  }

  state = sigil_state_expect_token(state, (uint16_t)monkey_token_type::ident);
  if (state.status) {
    return state;
  }

  sigil_token token = sigil_tokens_data(state.tokens)[state.token_cursor];
  *(monkey_slice *)((char *)data + ident_addr) = {
      .base = token.off,
      .len = token.len,
  };

  sigil_state_next(&state);
  if (state.status) {
    return state;
  }

  state = sigil_state_expect_token(state, (uint16_t)monkey_token_type::assign);
  if (state.status) {
    return state;
  }

  sigil_state_next(&state);
  if (state.status) {
    return state;
  }

  sigil_state expr_state = sigil_parser_parse_expr(monkey_parser, state,
                                                   (size_t)monkey_perc::LOWEST);

  if (expr_state.status) {
    return expr_state;
  }

  nodes[ident_node].next = expr_state.subroot;

  state =
      sigil_state_expect_token(expr_state, (uint16_t)monkey_token_type::semi);
  if (state.status) {
    return state;
  }

  sigil_state_next(&state);
  if (state.status) {
    return state;
  }

  state.subroot = let_node;

  return state;
}

sigil_state monkey_parse_file(sigil_state state) {
  uint16_t last = 0xFFFF;

  while (!sigil_state_done(state)) {
    state = monkey_parse_let(state);

    if (last != 0xFFFF) {
      sigil_node *node = sigil_ast_nodes(state.ast) + last;
      node->next = state.subroot;
      last = state.subroot;
    } else {
      sigil_node *node = sigil_ast_nodes(state.ast);
      node[0] = node[state.subroot];
      last = 0;
    }

    if (state.status) {
      return state;
    }
  }

  return state;
}

sigil_state monkey_parse_paren(sigil_state state) {
  sigil_state_next(&state);

  state = sigil_parser_parse_expr(monkey_parser, state,
                                  (size_t)monkey_perc::LOWEST);
  if (state.status)
    return state;

  state = sigil_state_expect_token(state,
                                   (sigil_token_type)monkey_token_type::rparen);

  if (state.status)
    return state;

  sigil_state_next(&state);

  return state;
}

sigil_state monkey_parse_infix(sigil_state state, uint16_t id) {
  sigil_node *nodes = sigil_ast_nodes(state.ast);
  sigil_token token = sigil_state_token(state);

  uint16_t node = sigil_state_alloc_node(&state);
  uint16_t addr = sigil_state_malloc(&state, sizeof(monkey_slice));

  if (state.status) {
    return state;
  }

  nodes[node] = {
      .child = id,
      .next = 0,
      .offset = addr,
      .type = 0,
  };

  switch ((monkey_token_type)token.type) {
  case monkey_token_type::plus:
    nodes[node].type = (uint16_t)monkey_node_type::add;
    break;

  case monkey_token_type::mult:
    nodes[node].type = (uint16_t)monkey_node_type::mult;
    break;

  case monkey_token_type::let:
  case monkey_token_type::assign:
  case monkey_token_type::semi:
  case monkey_token_type::ident:
  case monkey_token_type::number:
  case monkey_token_type::lparen:
  case monkey_token_type::rparen:
  case monkey_token_type::minus:
  case monkey_token_type::div:
    state.status |= 4;
    return state;
    break;
  }

  size_t perc = sigil_parser_slots(monkey_parser)[token.type].perc;
  sigil_state_next(&state);

  sigil_state next = sigil_parser_parse_expr(monkey_parser, state, perc);
  if (next.status) {
    return next;
  }

  nodes[id].next = next.subroot;

  next.subroot = node;
  return next;
}

sigil_state monkey_parse_ident(sigil_state state) {
  sigil_node *nodes = sigil_ast_nodes(state.ast);
  void *data = sigil_ast_data(state.ast);

  uint16_t ident_node = sigil_state_alloc_node(&state);
  uint16_t ident_addr = sigil_state_malloc(&state, sizeof(monkey_slice));

  if (state.status) {
    return state;
  }

  nodes[ident_node] = {
      .child = 0,
      .next = 0,
      .offset = ident_addr,
      .type = (uint16_t)monkey_node_type::ident,
  };

  sigil_token token = sigil_tokens_data(state.tokens)[state.token_cursor];
  *(monkey_slice *)((char *)data + ident_addr) = {
      .base = token.off,
      .len = token.len,
  };

  sigil_state_next(&state);
  if (state.status) {
    return state;
  }

  state.subroot = ident_node;
  return state;
}

sigil_state monkey_parse_number(sigil_state state) {
  sigil_node *nodes = sigil_ast_nodes(state.ast);
  void *data = sigil_ast_data(state.ast);
  sigil_token token = sigil_state_token(state);

  uint16_t node = sigil_state_alloc_node(&state);
  uint16_t addr = sigil_state_malloc(&state, sizeof(monkey_slice));

  if (state.status) {
    return state;
  }

  nodes[node] = {
      .child = 0,
      .next = 0,
      .offset = addr,
      .type = (uint16_t)monkey_node_type::lit,
  };

  monkey_slice *slice = (monkey_slice *)((char *)data + addr);
  slice->base = token.off;
  slice->len = token.len;

  sigil_state_next(&state);

  state.subroot = node;

  return state;
}

bool monkey_is_terminal(sigil_token_type token) {
  return token == (sigil_token_type)monkey_token_type::semi;
}

void monkey_init_system(void) {
  monkey_parser = sigil_parser_init(((size_t)monkey_token_type::div + 1),
                                    monkey_is_terminal, monkey_parse_file);

  sigil_parser_slot *slots = sigil_parser_slots(monkey_parser);

  slots[(sigil_token_type)monkey_token_type::number].prefix =
      monkey_parse_number;
  slots[(sigil_token_type)monkey_token_type::lparen].prefix =
      monkey_parse_paren;
  slots[(sigil_token_type)monkey_token_type::ident].prefix = monkey_parse_ident;

  slots[(sigil_token_type)monkey_token_type::plus] = {
      .prefix = NULL,
      .postfix = NULL,
      .infix = monkey_parse_infix,
      .perc = (size_t)monkey_perc::SUM,
  };

  slots[(sigil_token_type)monkey_token_type::mult] = {
      .prefix = NULL,
      .postfix = NULL,
      .infix = monkey_parse_infix,
      .perc = (size_t)monkey_perc::PROD,
  };
}

void monkey_deinit_system(void) { sigil_parser_deinit(monkey_parser); }

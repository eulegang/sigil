
#include "arcana.h"
#include "monkey.h"
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

#define token(T) (arcana_token_type) monkey_token_type::T

static arcana_parser_pratt *monkey_parser;

arcana_parser_state monkey_parse_let(arcana_parser_state state) {
  arcana_parse_node *nodes = arcana_parser_ast_nodes(state.ast);
  void *data = arcana_parser_ast_data(state.ast);

  uint16_t let_node = arcana_parser_alloc_node(&state);
  uint16_t ident_node = arcana_parser_alloc_node(&state);

  uint16_t ident_addr = arcana_parser_ast_malloc(&state, sizeof(monkey_slice));

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

  state = arcana_parser_expect_token(state, (uint16_t)monkey_token_type::let);
  if (state.status) {
    return state;
  }

  arcana_parser_ast_next_token(&state);
  if (state.status) {
    return state;
  }

  state = arcana_parser_expect_token(state, (uint16_t)monkey_token_type::ident);
  if (state.status) {
    return state;
  }

  arcana_token token = arcana_tokens_data(state.tokens)[state.token_cursor];
  *(monkey_slice *)((char *)data + ident_addr) = {
      .base = token.off,
      .len = token.len,
  };

  arcana_parser_ast_next_token(&state);
  if (state.status) {
    return state;
  }

  state =
      arcana_parser_expect_token(state, (uint16_t)monkey_token_type::assign);
  if (state.status) {
    return state;
  }

  arcana_parser_ast_next_token(&state);
  if (state.status) {
    return state;
  }

  arcana_parser_state expr_state = arcana_parser_pratt_parse(
      monkey_parser, state, (size_t)monkey_perc::LOWEST);
  // monkey_parse_expr(state);
  if (expr_state.status) {
    return expr_state;
  }

  nodes[ident_node].next = expr_state.subroot;

  state =
      arcana_parser_expect_token(expr_state, (uint16_t)monkey_token_type::semi);
  if (state.status) {
    return state;
  }

  arcana_parser_ast_next_token(&state);
  if (state.status) {
    return state;
  }

  state.subroot = let_node;

  return state;
}

arcana_parser_state monkey_parse_file(arcana_parser_state state) {
  uint16_t last = 0xFFFF;

  while (!arcana_parser_state_done(state)) {
    state = monkey_parse_let(state);

    if (last != 0xFFFF) {
      arcana_parse_node *node = arcana_parser_ast_nodes(state.ast) + last;
      node->next = state.subroot;
    }

    last = state.subroot;

    if (state.status) {
      return state;
    }
  }

  return state;
}

arcana_parser_state monkey_parse_paren(arcana_parser_state state) {
  arcana_parser_ast_next_token(&state);

  state = arcana_parser_pratt_parse(monkey_parser, state,
                                    (size_t)monkey_perc::LOWEST);
  if (state.status)
    return state;

  state = arcana_parser_expect_token(
      state, (arcana_token_type)monkey_token_type::rparen);

  if (state.status)
    return state;

  arcana_parser_ast_next_token(&state);

  return state;
}

arcana_parser_state monkey_parse_infix(arcana_parser_state state, uint16_t id) {
  arcana_parse_node *nodes = arcana_parser_ast_nodes(state.ast);
  arcana_token token = arcana_parser_token(state);

  uint16_t node = arcana_parser_alloc_node(&state);
  uint16_t addr = arcana_parser_ast_malloc(&state, sizeof(monkey_slice));

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

  size_t prec = arcana_parser_pratt_precedence(monkey_parser, token.type);
  arcana_parser_ast_next_token(&state);

  arcana_parser_state next =
      arcana_parser_pratt_parse(monkey_parser, state, prec);
  if (next.status) {
    return next;
  }

  nodes[id].next = next.subroot;

  next.subroot = node;
  return next;
}

arcana_parser_state monkey_parse_ident(arcana_parser_state state) {
  arcana_parse_node *nodes = arcana_parser_ast_nodes(state.ast);
  void *data = arcana_parser_ast_data(state.ast);

  uint16_t ident_node = arcana_parser_alloc_node(&state);
  uint16_t ident_addr = arcana_parser_ast_malloc(&state, sizeof(monkey_slice));

  if (state.status) {
    return state;
  }

  nodes[ident_node] = {
      .child = 0,
      .next = 0,
      .offset = ident_addr,
      .type = (uint16_t)monkey_node_type::ident,
  };

  arcana_token token = arcana_tokens_data(state.tokens)[state.token_cursor];
  *(monkey_slice *)((char *)data + ident_addr) = {
      .base = token.off,
      .len = token.len,
  };

  arcana_parser_ast_next_token(&state);
  if (state.status) {
    return state;
  }

  state.subroot = ident_node;
  return state;
}

arcana_parser_state monkey_parse_number(arcana_parser_state state) {
  arcana_parse_node *nodes = arcana_parser_ast_nodes(state.ast);
  void *data = arcana_parser_ast_data(state.ast);
  arcana_token token = arcana_parser_token(state);

  uint16_t node = arcana_parser_alloc_node(&state);
  uint16_t addr = arcana_parser_ast_malloc(&state, sizeof(monkey_slice));

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

  arcana_parser_ast_next_token(&state);

  state.subroot = node;

  return state;
}

bool monkey_is_terminal(arcana_token_type token) {
  return token == (arcana_token_type)monkey_token_type::semi;
}

void monkey_init_system(void) {
  monkey_parser = arcana_parser_pratt_init(((size_t)monkey_token_type::div + 1),
                                           monkey_is_terminal);

  arcana_parser_pratt_add_prefix(monkey_parser,
                                 (arcana_token_type)monkey_token_type::number,
                                 monkey_parse_number);

  arcana_parser_pratt_add_prefix(monkey_parser,
                                 (arcana_token_type)monkey_token_type::lparen,
                                 monkey_parse_paren);
  arcana_parser_pratt_add_prefix(monkey_parser,
                                 (arcana_token_type)monkey_token_type::ident,
                                 monkey_parse_ident);

  arcana_parser_pratt_add_infix(monkey_parser,
                                (arcana_token_type)monkey_token_type::plus,
                                monkey_parse_infix);

  arcana_parser_pratt_add_infix(monkey_parser,
                                (arcana_token_type)monkey_token_type::mult,
                                monkey_parse_infix);

  arcana_parser_pratt_add_precedence(monkey_parser,
                                     (arcana_token_type)monkey_token_type::plus,
                                     (size_t)monkey_perc::SUM);

  arcana_parser_pratt_add_precedence(monkey_parser,
                                     (arcana_token_type)monkey_token_type::mult,
                                     (size_t)monkey_perc::PROD);
}

void monkey_deinit_system(void) { arcana_parser_pratt_deinit(monkey_parser); }

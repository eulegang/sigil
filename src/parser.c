#include "arcana.h"
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

struct arcana_parser {
  arcana_parser_branch_prefix prefix;
};

struct arcana_parser_ast {
  size_t len;
  size_t cap;
  uint16_t nodes;
  uint16_t data;
};

arcana_parser *arcana_parser_init(arcana_parser_branch_prefix prefix) {
  arcana_parser *res = malloc(sizeof(arcana_parser));

  res->prefix = prefix;

  return res;
}

void arcana_parser_deinit(arcana_parser *parser) { free(parser); }

arcana_parser_ast *arcana_parser_parse(arcana_parser *parser,
                                       arcana_tokens_t *tokens) {
  size_t len = arcana_pages * getpagesize();

  arcana_parser_ast *res =
      mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

  if (res == MAP_FAILED) {
    return NULL;
  }

  res->len = 0;
  res->cap = len;

  arcana_parser_state state = {
      .tokens = tokens,
      .ast = res,

      .token_cursor = 0,
      .node_cursor = 0,
      .data_cursor = 0,
      .subroot = 0,
      .status = 0,
  };

  arcana_parser_state last = parser->prefix(state);

  if (last.status) {
    arcana_parser_ast_deinit(res);
    return NULL;
  }

  if (!arcana_parser_state_done(last)) {
    arcana_parser_ast_deinit(res);
    return NULL;
  }

  res->nodes = last.node_cursor;
  res->data = last.data_cursor;

  return res;
}

void arcana_parser_ast_deinit(arcana_parser_ast *ast) { munmap(ast, ast->cap); }

arcana_token arcana_parser_token(arcana_parser_state state) {
  return arcana_tokens_data(state.tokens)[state.token_cursor];
}

arcana_token arcana_parser_peek_token(arcana_parser_state state, size_t off) {
  return arcana_tokens_data(state.tokens)[state.token_cursor + off];
}

arcana_parser_state arcana_parser_expect_token(arcana_parser_state state,
                                               arcana_token_type token_type) {
  arcana_token token = arcana_parser_token(state);
  if (token.type != token_type) {
    state.status = 1;
  }
  return state;
}

uint16_t arcana_parser_ast_malloc(arcana_parser_state *state, size_t size) {
  uint16_t cursor = state->data_cursor;
  state->data_cursor += size;
  return cursor;
}

uint16_t arcana_parser_alloc_node(arcana_parser_state *state) {
  return state->node_cursor++;
}

arcana_parse_node *arcana_parser_ast_nodes(arcana_parser_ast *ast) {
  return (void *)(ast + 1);
}

uint16_t arcana_parser_ast_node_count(arcana_parser_ast *ast) {
  return ast->nodes;
}

uint16_t arcana_parser_ast_data_size(arcana_parser_ast *ast) {
  return ast->data;
}

void *arcana_parser_ast_data(arcana_parser_ast *ast) {
  return (void *)ast + (ast->cap / 2);
}

void arcana_parser_ast_next_token(arcana_parser_state *state) {
  if (state->token_cursor >= arcana_tokens_len(state->tokens)) {
    state->status |= 2;
  } else {
    state->token_cursor += 1;
  }
}

bool arcana_parser_state_done(arcana_parser_state state) {
  return state.token_cursor >= arcana_tokens_len(state.tokens);
}

void arcana_parser_ast_visit_recur(arcana_parser_ast *ast, arcana_slice content,
                                   void *ctx, arcana_parse_node node,
                                   size_t level,
                                   arcana_parser_ast_visit_fn fn) {
  void *addr = NULL;

  if (node.offset != 0xFFFF) {
    addr = arcana_parser_ast_data(ast) + node.offset;
  }

  fn(node, addr, level, content, ctx);

  if (node.child != 0) {
    arcana_parser_ast_visit_recur(ast, content, ctx,
                                  arcana_parser_ast_nodes(ast)[node.child],
                                  level + 1, fn);
  }

  if (node.next != 0) {
    arcana_parser_ast_visit_recur(
        ast, content, ctx, arcana_parser_ast_nodes(ast)[node.next], level, fn);
  }
}

void arcana_parser_ast_visit(arcana_parser_ast *ast, arcana_slice content,
                             void *ctx, arcana_parser_ast_visit_fn fn) {
  if (ast->nodes == 0) {
    return;
  }

  uint16_t cur = 0;
  arcana_parse_node *base = arcana_parser_ast_nodes(ast);
  arcana_parse_node node = base[cur];

  arcana_parser_ast_visit_recur(ast, content, ctx, node, 0, fn);
}

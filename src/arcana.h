#ifndef _arcana_H
#define _arcana_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Slice of content
 */

typedef struct {
  const char *data;
  size_t len;
} arcana_slice;

arcana_slice arcana_slice_advance(arcana_slice, size_t);

/**
 * Tokens
 */

typedef uint32_t arcana_token_type;

typedef struct {
  arcana_token_type type;
  uint16_t off;
  uint16_t len;
} arcana_token;

typedef struct {
  uint16_t line;
  uint16_t column;
} arcana_linemeta;

typedef ssize_t (*arcana_tokenizer)(size_t cur, arcana_slice content,
                                    arcana_token_type *type);

typedef struct {
  arcana_slice content;
  arcana_tokenizer tokenizer;
} arcana_tokens_options;

typedef struct {
  uint32_t err;
  uint32_t pos;
} arcana_tokens_error;

#define ARCANA_TOKENS_ERROR_MAP 1
#define ARCANA_TOKENS_ERROR_OVERFLOW 2
#define ARCANA_TOKENS_ERROR_INVALID 3

extern size_t arcana_pages;

typedef struct arcana_tokens arcana_tokens;
arcana_tokens *arcana_tokens_init(arcana_tokens_options, arcana_tokens_error *);

void arcana_tokens_deinit(arcana_tokens *);

size_t arcana_tokens_len(arcana_tokens *);
size_t arcana_tokens_capacity(arcana_tokens *);
arcana_token *arcana_tokens_data(arcana_tokens *);
arcana_linemeta *arcana_tokens_linemeta(arcana_tokens *);
arcana_slice arcana_tokens_slice(arcana_tokens *, uint16_t);

/**
 * Token Table
 */

typedef struct arcana_table arcana_table;

arcana_table *arcana_table_init();
void arcana_table_deinit(arcana_table *);

const char **arcana_table_data(arcana_table *);
size_t arcana_table_len(arcana_table *);
void arcana_table_push(arcana_table **, const char *);

/**
 * Ast
 */

typedef struct {
  uint16_t child;
  uint16_t next;
  uint16_t offset;
  uint16_t type;
} arcana_node;

typedef struct arcana_ast arcana_ast;

void arcana_ast_deinit(arcana_ast *);

arcana_node *arcana_ast_nodes(arcana_ast *);
uint16_t arcana_ast_node_count(arcana_ast *);
uint16_t arcana_ast_data_size(arcana_ast *);
void *arcana_ast_data(arcana_ast *);

typedef void (*arcana_ast_visit_fn)(arcana_node node, void *data, size_t level,
                                    arcana_slice content, void *ctx);

void arcana_ast_visit(arcana_ast *ast, arcana_slice content, void *ctx,
                      arcana_ast_visit_fn fn);

/*
 * State
 */

typedef struct arcana_state {
  arcana_tokens *tokens;
  arcana_ast *ast;

  uint16_t token_cursor;
  uint16_t node_cursor;
  uint16_t data_cursor;
  uint16_t subroot;
  uint16_t status;
} arcana_state;

arcana_token arcana_state_token(arcana_state);
arcana_token arcana_state_peek(arcana_state, size_t);
arcana_state arcana_state_expect_token(arcana_state, arcana_token_type);

uint16_t arcana_state_malloc(arcana_state *, size_t);
uint16_t arcana_state_alloc_node(arcana_state *);

arcana_node *arcana_state_node(arcana_state, uint16_t);
void *arcana_state_data(arcana_state, uint16_t);

void arcana_state_next(arcana_state *);
bool arcana_state_done(arcana_state);

/*
 * Parser
 */

typedef struct arcana_parser arcana_parser;
typedef arcana_state (*arcana_subparser)(arcana_state);
typedef arcana_state (*arcana_binparser)(arcana_state, uint16_t);
typedef bool (*arcana_terminal)(arcana_token_type);
typedef struct {
  arcana_subparser prefix;
  arcana_subparser postfix;
  arcana_binparser infix;
  size_t perc;
} arcana_parser_slot;

typedef struct {
  uint16_t token;
  uint16_t status;
} arcana_parser_error;

arcana_parser *arcana_parser_init(size_t cap, arcana_terminal,
                                  arcana_subparser);

void arcana_parser_deinit(arcana_parser *);

arcana_parser_slot *arcana_parser_slots(arcana_parser *);

arcana_ast *arcana_parser_parse(arcana_parser *, arcana_tokens *,
                                arcana_parser_error *);
arcana_state arcana_parser_parse_expr(arcana_parser *, arcana_state, size_t);

/*
 * Overlay
 */

typedef struct arcana_overlay arcana_overlay;

arcana_overlay *arcana_overlay_init(arcana_ast *, size_t pages);
void arcana_overlay_deinit(arcana_overlay *);

void *arcana_overlay_alloc(arcana_overlay *, uint16_t node, size_t);
void *arcana_overlay_resolve(arcana_overlay *, uint16_t node);

/*
 * Lexer Util
 */

ssize_t arcana_util_take_while(arcana_slice, bool (*)(char));
ssize_t arcana_util_keyword(arcana_slice, const char *);
ssize_t arcana_util_take_stateful(arcana_slice, void *, bool (*)(void *, char));

#ifdef __cplusplus
}
#endif
#endif

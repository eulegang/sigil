#ifndef _arcana_H
#define _arcana_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

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

typedef struct arcana_tokens arcana_tokens_t;
arcana_tokens_t *arcana_tokens_init(arcana_tokens_options,
                                    arcana_tokens_error *);

void arcana_tokens_deinit(arcana_tokens_t *);

size_t arcana_tokens_len(arcana_tokens_t *);
size_t arcana_tokens_capacity(arcana_tokens_t *);
arcana_token *arcana_tokens_data(arcana_tokens_t *);
arcana_linemeta *arcana_tokens_linemeta(arcana_tokens_t *);
arcana_slice arcana_tokens_slice(arcana_tokens_t *, uint16_t);

/**
 * Token Table
 */

typedef struct arcana_token_table arcana_token_table_t;

arcana_token_table_t *arcana_token_table_init();
void arcana_token_table_deinit(arcana_token_table_t *);

const char **arcana_token_table_data(arcana_token_table_t *);
size_t arcana_token_table_len(arcana_token_table_t *);
void arcana_token_table_push(arcana_token_table_t **, const char *);

/*
 * Parser
 */

typedef struct arcana_parser_ast arcana_parser_ast;

typedef struct arcana_parser_state {
  arcana_tokens_t *tokens;
  arcana_parser_ast *ast;

  uint16_t token_cursor;
  uint16_t node_cursor;
  uint16_t data_cursor;
  uint16_t subroot;
  uint16_t status;
} arcana_parser_state;

typedef struct arcana_parser arcana_parser;

typedef arcana_parser_state (*arcana_parser_branch_prefix)(arcana_parser_state);

typedef struct {
  uint16_t child;
  uint16_t next;
  uint16_t offset;
  uint16_t type;
} arcana_parse_node;

arcana_parser *arcana_parser_init(arcana_parser_branch_prefix);
void arcana_parser_deinit(arcana_parser *);

arcana_parser_ast *arcana_parser_parse(arcana_parser *, arcana_tokens_t *);
void arcana_parser_ast_deinit(arcana_parser_ast *);

arcana_token arcana_parser_token(arcana_parser_state);
arcana_token arcana_parser_peek_token(arcana_parser_state, size_t);
arcana_parser_state arcana_parser_expect_token(arcana_parser_state,
                                               arcana_token_type);

uint16_t arcana_parser_ast_malloc(arcana_parser_state *, size_t);
uint16_t arcana_parser_alloc_node(arcana_parser_state *);

void arcana_parser_ast_next_token(arcana_parser_state *);
bool arcana_parser_state_done(arcana_parser_state);

arcana_parse_node *arcana_parser_ast_nodes(arcana_parser_ast *);
uint16_t arcana_parser_ast_node_count(arcana_parser_ast *);
uint16_t arcana_parser_ast_data_size(arcana_parser_ast *);
void *arcana_parser_ast_data(arcana_parser_ast *);

typedef void (*arcana_parser_ast_visit_fn)(arcana_parse_node node, void *data,
                                           size_t level, arcana_slice content,
                                           void *ctx);
void arcana_parser_ast_visit(arcana_parser_ast *ast, arcana_slice content,
                             void *ctx, arcana_parser_ast_visit_fn fn);

/*
 * Pratt parsing
 */

typedef struct arcana_parser_pratt arcana_parser_pratt;
typedef arcana_parser_state (*arcana_parser_pratt_prefix_parser)(
    arcana_parser_state);
typedef arcana_parser_state (*arcana_parser_pratt_infix_parser)(
    arcana_parser_state, uint16_t);
typedef arcana_parser_state (*arcana_parser_pratt_postfix_parser)(
    arcana_parser_state);

arcana_parser_pratt *
arcana_parser_pratt_init(size_t cap, bool (*is_terminal)(arcana_token_type));
void arcana_parser_pratt_deinit(arcana_parser_pratt *);

void arcana_parser_pratt_add_prefix(arcana_parser_pratt *, arcana_token_type,
                                    arcana_parser_pratt_prefix_parser);

void arcana_parser_pratt_add_infix(arcana_parser_pratt *, arcana_token_type,
                                   arcana_parser_pratt_infix_parser);

void arcana_parser_pratt_add_postfix(arcana_parser_pratt *, arcana_token_type,
                                     arcana_parser_pratt_postfix_parser);

void arcana_parser_pratt_add_precedence(arcana_parser_pratt *,
                                        arcana_token_type, size_t);

size_t arcana_parser_pratt_precedence(arcana_parser_pratt *, arcana_token_type);

arcana_parser_state arcana_parser_pratt_parse(arcana_parser_pratt *,
                                              arcana_parser_state, size_t);

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

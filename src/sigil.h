#ifndef _sigil_H
#define _sigil_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
#include <memory>
extern "C" {
#endif

/**
 * Slice of content
 */

typedef struct {
  const char *data;
  size_t len;
} sigil_slice;

sigil_slice sigil_slice_advance(sigil_slice, size_t);

/**
 * Tokens
 */

typedef uint32_t sigil_token_type;

typedef struct {
  sigil_token_type type;
  uint16_t off;
  uint16_t len;
} sigil_token;

typedef struct {
  uint16_t line;
  uint16_t column;
} sigil_linemeta;

typedef ssize_t (*sigil_tokenizer)(size_t cur, sigil_slice content,
                                   sigil_token_type *type);

typedef struct {
  sigil_slice content;
  sigil_tokenizer tokenizer;
} sigil_tokens_options;

typedef struct {
  uint32_t err;
  uint32_t pos;
} sigil_tokens_error;

#define sigil_TOKENS_ERROR_MAP 1
#define sigil_TOKENS_ERROR_OVERFLOW 2
#define sigil_TOKENS_ERROR_INVALID 3

extern size_t sigil_pages;

typedef struct sigil_tokens sigil_tokens;
sigil_tokens *sigil_tokens_init(sigil_tokens_options, sigil_tokens_error *);

void sigil_tokens_deinit(sigil_tokens *);

size_t sigil_tokens_len(sigil_tokens *);
size_t sigil_tokens_capacity(sigil_tokens *);
sigil_token *sigil_tokens_data(sigil_tokens *);
sigil_linemeta *sigil_tokens_linemeta(sigil_tokens *);
sigil_slice sigil_tokens_slice(sigil_tokens *, uint16_t);

/**
 * Token Table
 */

typedef struct sigil_table sigil_table;

sigil_table *sigil_table_init();
void sigil_table_deinit(sigil_table *);

const char **sigil_table_data(sigil_table *);
size_t sigil_table_len(sigil_table *);
void sigil_table_push(sigil_table **, const char *);

/**
 * Ast
 */

typedef struct {
  uint16_t child;
  uint16_t next;
  uint16_t offset;
  uint16_t type;
} sigil_node;

typedef struct sigil_ast sigil_ast;

void sigil_ast_deinit(sigil_ast *);

sigil_node *sigil_ast_nodes(sigil_ast *);
uint16_t sigil_ast_node_count(sigil_ast *);
uint16_t sigil_ast_data_size(sigil_ast *);
void *sigil_ast_data(sigil_ast *);

typedef void (*sigil_ast_visit_fn)(uint16_t id, sigil_node node, void *data,
                                   size_t level, void *ctx);

void sigil_ast_visit(sigil_ast *ast, void *ctx, sigil_ast_visit_fn fn);

/*
 * State
 */

typedef struct sigil_state {
  sigil_tokens *tokens;
  sigil_ast *ast;

  uint16_t token_cursor;
  uint16_t node_cursor;
  uint16_t data_cursor;
  uint16_t subroot;
  uint16_t status;
} sigil_state;

sigil_token sigil_state_token(sigil_state);
sigil_token sigil_state_peek(sigil_state, size_t);
sigil_state sigil_state_expect_token(sigil_state, sigil_token_type);

uint16_t sigil_state_malloc(sigil_state *, size_t);
uint16_t sigil_state_alloc_node(sigil_state *);

sigil_node *sigil_state_node(sigil_state, uint16_t);
void *sigil_state_data(sigil_state, uint16_t);

void sigil_state_next(sigil_state *);
bool sigil_state_done(sigil_state);

/*
 * Parser
 */

typedef struct sigil_parser sigil_parser;
typedef sigil_state (*sigil_subparser)(sigil_state);
typedef sigil_state (*sigil_binparser)(sigil_state, uint16_t);
typedef bool (*sigil_terminal)(sigil_token_type);
typedef struct {
  sigil_subparser prefix;
  sigil_subparser postfix;
  sigil_binparser infix;
  size_t perc;
} sigil_parser_slot;

typedef struct {
  uint16_t token;
  uint16_t status;
} sigil_parser_error;

sigil_parser *sigil_parser_init(size_t cap, sigil_terminal, sigil_subparser);

void sigil_parser_deinit(sigil_parser *);

sigil_parser_slot *sigil_parser_slots(sigil_parser *);

sigil_ast *sigil_parser_parse(sigil_parser *, sigil_tokens *,
                              sigil_parser_error *);
sigil_state sigil_parser_parse_expr(sigil_parser *, sigil_state, size_t);

/*
 * Overlay
 */

typedef struct sigil_overlay sigil_overlay;

sigil_overlay *sigil_overlay_init(sigil_ast *, size_t pages);
void sigil_overlay_deinit(sigil_overlay *);

void *sigil_overlay_alloc(sigil_overlay *, uint16_t node, size_t);
void *sigil_overlay_resolve(sigil_overlay *, uint16_t node);

/*
 * Lexer Util
 */

ssize_t sigil_util_take_while(sigil_slice, bool (*)(char));
ssize_t sigil_util_keyword(sigil_slice, const char *);
ssize_t sigil_util_take_stateful(sigil_slice, void *, bool (*)(void *, char));

#ifdef __cplusplus
}
namespace sigil {
template <typename T> struct Tokens final {
  static_assert(sizeof(T) == 4);

  struct Token final {
    T type;
    uint16_t off;
    uint16_t len;
  };

  using Ptr = std::unique_ptr<sigil_tokens, decltype(&sigil_tokens_deinit)>;
  using Idx = uint16_t;

  Ptr ptr;

  Tokens(sigil_tokens *tokens) : ptr{tokens, sigil_tokens_deinit} {}
  Tokens(std::string_view content, sigil_tokenizer tokenizer)
      : ptr{nullptr, sigil_tokens_deinit} {
    sigil_slice buf = {.data = content.data(), .len = content.length()};

    sigil_tokens_options opts = {
        .content = buf,
        .tokenizer = tokenizer,
    };

    sigil_tokens_error err;
    sigil_tokens *tokens = sigil_tokens_init(opts, &err);

    if (!tokens)
      throw err;

    ptr = Ptr(tokens, sigil_tokens_deinit);
  }

  size_t length() { return sigil_tokens_len(ptr.get()); }
  size_t capacity() { return sigil_tokens_capacity(ptr.get()); }

  Token &operator[](Idx idx) const {
    return *(Token *)(sigil_tokens_data(ptr.get()) + idx);
  }

  sigil_linemeta linemeta(Idx idx) {
    return sigil_tokens_linemeta(ptr.get())[idx];
  }

  std::string_view content(Idx idx) {
    sigil_slice slice = sigil_tokens_slice(ptr.get(), idx);
    return std::string_view(slice.data, slice.len);
  }
};

template <typename T> struct Ast {
  static_assert(sizeof(T) == 2);

  using Idx = uint16_t;

  struct Node final {
    Idx child;
    Idx next;
    Idx offset;
    T type;
  };

  using Ptr = std::unique_ptr<sigil_ast, decltype(&sigil_ast_deinit)>;

  Ptr ptr;

  Ast(sigil_ast *ast) : ptr{ast, sigil_ast_deinit} {}

  uint16_t node_count() const { return sigil_ast_node_count(ptr.get()); }
  uint16_t data_size() const { return sigil_ast_data_size(ptr.get()); }

  Node &operator[](Idx idx) {
    return *(Node *)(sigil_ast_nodes(ptr.get()) + idx);
  }

  template <typename D> D *data(Idx idx) {
    if (idx == 0xFFFF)
      return NULL;

    return (D *)((char *)sigil_ast_data(ptr.get()) + idx);
  }

  void visit(void *ctx, sigil_ast_visit_fn fn) {
    sigil_ast_visit(ptr.get(), ctx, fn);
  }

  // template <typename Ctx>
  // void visit(Ctx *ctx, void (*fn)(uint16_t id, Node node, void *data,
  //                                 size_t level, Ctx *ctx)) {
  //   sigil_ast_visit(ptr.get(), ctx, fn);
  // }
};

template <typename T> struct Overlay {
  using Ptr = std::unique_ptr<sigil_overlay, decltype(&sigil_overlay_deinit)>;
  Ptr ptr;

  Overlay() : ptr{Ptr(nullptr, sigil_overlay_deinit)} {}

  Overlay(sigil_ast *ast, size_t pages)
      : ptr{Ptr(sigil_overlay_init(ast, pages), sigil_overlay_deinit)} {}

  T *alloc(uint16_t node_id) {
    return (T *)sigil_overlay_alloc(ptr.get(), node_id, sizeof(T));
  }

  T *resolve(uint16_t node_id) {
    return (T *)sigil_overlay_resolve(ptr.get(), node_id);
  }
};
} // namespace sigil

#endif

#endif

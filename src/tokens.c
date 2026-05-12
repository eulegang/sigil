#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "sigil.h"

struct sigil_tokens {
  size_t len;
  sigil_slice content;
  size_t cap;
};

size_t sigil_pages = 16;

bool sigil_process(sigil_tokens *, sigil_tokens_options, sigil_tokens_error *);

sigil_tokens *sigil_tokens_init(sigil_tokens_options opts,
                                sigil_tokens_error *error) {
  assert(opts.content.data != NULL);
  assert(opts.content.len != 0);

  size_t len = sigil_pages * getpagesize();
  sigil_tokens *res =
      mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

  if (res == MAP_FAILED) {
    if (error) {
      error->err = sigil_TOKENS_ERROR_MAP;
      error->pos = 0;
    }
    return NULL;
  }

  res->len = 0;
  res->cap = len;
  res->content = opts.content;

  if (!sigil_process(res, opts, error)) {
    sigil_tokens_deinit(res);
    return NULL;
  }

  return res;
}

void sigil_tokens_deinit(sigil_tokens *tokens) {
  if (tokens)
    munmap(tokens, tokens->cap);
}

void calc_meta(const char *base, size_t len, uint16_t *line, uint16_t *col) {
  for (size_t i = 0; i < len; i++) {
    if (base[i] == '\n') {
      *line += 1;
      *col = 1;
    } else {
      *col += 1;
    }
  }
}

bool sigil_process(sigil_tokens *tokens, sigil_tokens_options opts,
                   sigil_tokens_error *error) {
  uint16_t cur = 0;
  sigil_token_type type;

  size_t cap = sigil_tokens_capacity(tokens);

  sigil_token *base = sigil_tokens_data(tokens);
  sigil_linemeta *meta = sigil_tokens_linemeta(tokens);

  uint16_t line = 1;
  uint16_t col = 1;

  do {
    if (tokens->len >= cap) {
      if (error) {
        error->err = sigil_TOKENS_ERROR_OVERFLOW;
        error->pos = cur;
      }
      return false;
    }

    ssize_t inc = opts.tokenizer(cur, opts.content, &type);
    if (inc == 0) {
      if (error) {
        error->err = sigil_TOKENS_ERROR_INVALID;
        error->pos = cur;
      }
      return false;
    } else if (inc < 0) {
      calc_meta(opts.content.data + cur, -inc, &line, &col);
      cur += -inc;
    } else {
      if (cur + inc > opts.content.len) {
        if (error) {
          error->err = sigil_TOKENS_ERROR_OVERFLOW;
          error->pos = cur;
        }
        return false;
      }

      base[tokens->len] = (sigil_token){
          .type = type,
          .off = cur,
          .len = inc,
      };

      meta[tokens->len] = (sigil_linemeta){
          .column = col,
          .line = line,
      };

      tokens->len++;

      calc_meta(opts.content.data + cur, inc, &line, &col);

      cur += inc;
    }

  } while (cur < opts.content.len);

  return true;
}

size_t sigil_tokens_len(sigil_tokens *table) { return table->len; }

sigil_token *sigil_tokens_data(sigil_tokens *table) {
  return (sigil_token *)((void *)table + sizeof(sigil_tokens));
}

sigil_linemeta *sigil_tokens_linemeta(sigil_tokens *tokens) {
  return (sigil_linemeta *)((void *)tokens + sizeof(sigil_tokens) +
                            sizeof(sigil_token) *
                                sigil_tokens_capacity(tokens));
}

size_t sigil_tokens_capacity(sigil_tokens *tokens) {
  return (tokens->cap - sizeof(sigil_tokens)) /
         (sizeof(sigil_token) + sizeof(sigil_linemeta));
}

sigil_slice sigil_tokens_slice(sigil_tokens *tokens, uint16_t index) {
  sigil_token *elem = sigil_tokens_data(tokens);
  elem += index;

  const char *data = tokens->content.data + (size_t)elem->off;
  size_t len = elem->len;

  return (sigil_slice){
      .data = data,
      .len = len,
  };
}

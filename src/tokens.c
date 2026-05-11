#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "arcana.h"

struct arcana_tokens {
  size_t len;
  arcana_slice content;
  size_t cap;
};

size_t arcana_pages = 16;

bool arcana_process(arcana_tokens *, arcana_tokens_options,
                    arcana_tokens_error *);

arcana_tokens *arcana_tokens_init(arcana_tokens_options opts,
                                  arcana_tokens_error *error) {
  assert(opts.content.data != NULL);
  assert(opts.content.len != 0);

  size_t len = arcana_pages * getpagesize();
  arcana_tokens *res =
      mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

  if (res == MAP_FAILED) {
    if (error) {
      error->err = ARCANA_TOKENS_ERROR_MAP;
      error->pos = 0;
    }
    return NULL;
  }

  res->len = 0;
  res->cap = len;
  res->content = opts.content;

  if (!arcana_process(res, opts, error)) {
    arcana_tokens_deinit(res);
    return NULL;
  }

  return res;
}

void arcana_tokens_deinit(arcana_tokens *tokens) {
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

bool arcana_process(arcana_tokens *tokens, arcana_tokens_options opts,
                    arcana_tokens_error *error) {
  uint16_t cur = 0;
  arcana_token_type type;

  size_t cap = arcana_tokens_capacity(tokens);

  arcana_token *base = arcana_tokens_data(tokens);
  arcana_linemeta *meta = arcana_tokens_linemeta(tokens);

  uint16_t line = 1;
  uint16_t col = 1;

  do {
    if (tokens->len >= cap) {
      if (error) {
        error->err = ARCANA_TOKENS_ERROR_OVERFLOW;
        error->pos = cur;
      }
      return false;
    }

    ssize_t inc = opts.tokenizer(cur, opts.content, &type);
    if (inc == 0) {
      if (error) {
        error->err = ARCANA_TOKENS_ERROR_INVALID;
        error->pos = cur;
      }
      return false;
    } else if (inc < 0) {
      calc_meta(opts.content.data + cur, -inc, &line, &col);
      cur += -inc;
    } else {
      if (cur + inc > opts.content.len) {
        if (error) {
          error->err = ARCANA_TOKENS_ERROR_OVERFLOW;
          error->pos = cur;
        }
        return false;
      }

      base[tokens->len] = (arcana_token){
          .type = type,
          .off = cur,
          .len = inc,
      };

      meta[tokens->len] = (arcana_linemeta){
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

size_t arcana_tokens_len(arcana_tokens *table) { return table->len; }

arcana_token *arcana_tokens_data(arcana_tokens *table) {
  return (arcana_token *)((void *)table + sizeof(arcana_tokens));
}

arcana_linemeta *arcana_tokens_linemeta(arcana_tokens *tokens) {
  return (arcana_linemeta *)((void *)tokens + sizeof(arcana_tokens) +
                             sizeof(arcana_token) *
                                 arcana_tokens_capacity(tokens));
}

size_t arcana_tokens_capacity(arcana_tokens *tokens) {
  return (tokens->cap - sizeof(arcana_tokens)) /
         (sizeof(arcana_token) + sizeof(arcana_linemeta));
}

arcana_slice arcana_tokens_slice(arcana_tokens *tokens, uint16_t index) {
  arcana_token *elem = arcana_tokens_data(tokens);
  elem += index;

  const char *data = tokens->content.data + (size_t)elem->off;
  size_t len = elem->len;

  return (arcana_slice){
      .data = data,
      .len = len,
  };
}

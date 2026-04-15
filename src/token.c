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

bool arcana_process(arcana_tokens_t *, arcana_tokens_options opts);

arcana_tokens_t *arcana_tokens_init(arcana_tokens_options opts) {
  assert(opts.content.data != NULL);
  assert(opts.content.len != 0);

  size_t len = arcana_pages * getpagesize();
  arcana_tokens_t *res =
      mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

  if (res == MAP_FAILED) {
    return NULL;
  }

  res->len = 0;
  res->cap = len;
  res->content = opts.content;

  if (!arcana_process(res, opts)) {
    arcana_tokens_deinit(res);
    return NULL;
  }

  return res;
}

void arcana_tokens_deinit(arcana_tokens_t *tokens) {
  munmap(tokens, tokens->len);
}

bool arcana_process(arcana_tokens_t *tokens, arcana_tokens_options opts) {
  uint16_t cur = 0;
  arcana_token_type type;

  arcana_token *base = arcana_tokens_data(tokens);
  do {
    ssize_t inc = opts.tokenizer(cur, opts.content, &type);
    if (inc == 0) {
      return false;
    } else if (inc < 0) {
      cur += -inc;
    } else {
      arcana_token t = {
          .type = type,
          .off = cur,
          .len = inc,
      };

      base[tokens->len++] = t;

      cur += inc;
    }

  } while (cur < opts.content.len);

  return true;
}

size_t arcana_tokens_len(arcana_tokens_t *table) { return table->len; }
arcana_token *arcana_tokens_data(arcana_tokens_t *table) {
  return (arcana_token *)((void *)table + sizeof(arcana_tokens_t));
}

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

extern size_t arcana_pages;

typedef struct arcana_tokens arcana_tokens_t;
typedef struct arcana_token_table arcana_token_table_t;

arcana_tokens_t *arcana_tokens_init(arcana_tokens_options);
void arcana_tokens_deinit(arcana_tokens_t *);

size_t arcana_tokens_len(arcana_tokens_t *);
size_t arcana_tokens_capacity(arcana_tokens_t *);
arcana_token *arcana_tokens_data(arcana_tokens_t *);
arcana_linemeta *arcana_tokens_linemeta(arcana_tokens_t *);
arcana_slice arcana_tokens_slice(arcana_tokens_t *, uint16_t);

arcana_token_table_t *arcana_token_table_init();
void arcana_token_table_deinit(arcana_token_table_t *);

const char **arcana_token_table_data(arcana_token_table_t *);
size_t arcana_token_table_len(arcana_token_table_t *);
void arcana_token_table_push(arcana_token_table_t **, const char *);

arcana_slice arcana_slice_advance(arcana_slice, size_t);
ssize_t arcana_util_skip_while(arcana_slice, bool (*)(char));
ssize_t arcana_util_keyword(arcana_slice, const char *);

#ifdef __cplusplus
}
#endif
#endif

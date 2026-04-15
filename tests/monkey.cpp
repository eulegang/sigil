
#include "monkey.h"
#include "arcana.h"

arcana_slice pull_word(arcana_slice content, uint16_t offset) {
  arcana_slice s = {.data = content.data + (size_t)offset, .len = 0};

  while (offset + s.len < content.len) {
    char c = s.data[s.len];

    bool skip = false;

    skip |= 'a' <= c && c <= 'z';
    skip |= 'A' <= c && c <= 'Z';
    skip |= '0' <= c && c <= '9';

    if (!skip)
      break;

    s.len += 1;
  }

  return s;
}

arcana_slice pull_number(arcana_slice content, uint16_t offset) {
  arcana_slice s = {.data = content.data + (size_t)offset, .len = 0};

  while (offset + s.len < content.len) {
    char c = s.data[s.len];

    bool skip = false;

    skip |= '0' <= c && c <= '9';

    if (!skip)
      break;

    s.len += 1;
  }

  return s;
}

arcana_token_table_t *monkey_table() {
  static arcana_token_table_t *table = NULL;

  if (!table) {
    table = arcana_token_table_init();
    arcana_token_table_push(&table, "let");
  }

  return table;
}

ssize_t monkey_tokenizer(size_t cur, arcana_slice content,
                         arcana_token_type *type) {
  const char ch = content.data[cur];
  switch (ch) {
  case ' ':
    return -1;
  case '=':
    *type = monkey_token_type_assign;
    return 1;

  case ';':
    *type = monkey_token_type_semi;
    return 1;

  case 'l':
    if (content.data[cur + 1] == 'e' && content.data[cur + 2] == 't') {
      *type = monkey_token_type_let;
      return 3;
    }
    break;
  }

  if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_') {
    arcana_slice c = pull_word(content, cur);
    *type = monkey_token_type_ident;
    return c.len;
  }

  if ('0' <= ch && ch <= '9') {
    arcana_slice c = pull_number(content, cur);
    *type = monkey_token_type_number;
    return c.len;
  }

  return 0;
}

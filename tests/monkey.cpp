
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

  arcana_slice window = arcana_slice_advance(content, cur);
  ssize_t i = 0;

  const char ch = content.data[cur];
  switch (ch) {
  case ' ':
  case '\n':
    return -1;
  case '=':
    *type = monkey_token_type_assign;
    return 1;

  case ';':
    *type = monkey_token_type_semi;
    return 1;

  case '(':
    *type = monkey_token_type_lparen;
    return 1;

  case ')':
    *type = monkey_token_type_lparen;
    return 1;

  case '+':
    *type = monkey_token_type_plus;
    return 1;

  case '-':
    *type = monkey_token_type_minus;
    return 1;

  case '*':
    *type = monkey_token_type_mult;
    return 1;

  case '/':
    *type = monkey_token_type_div;
    return 1;

  case 'l':
    if ((i = arcana_util_keyword(window, "let"))) {
      *type = monkey_token_type_let;
      return i;
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

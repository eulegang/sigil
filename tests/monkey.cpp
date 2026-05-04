
#include "monkey.h"
#include "arcana.h"

#define token(T) (arcana_token_type) monkey_token_type::T

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

arcana_table *monkey_table() {
  static arcana_table *table = NULL;

  if (!table) {
    table = arcana_table_init();
    arcana_table_push(&table, "let");
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
    *type = token(assign);
    return 1;

  case ';':
    *type = token(semi);
    return 1;

  case '(':
    *type = token(lparen);
    return 1;

  case ')':
    *type = token(rparen);
    return 1;

  case '+':
    *type = token(plus);
    return 1;

  case '-':
    *type = token(minus);
    return 1;

  case '*':
    *type = token(mult);
    return 1;

  case '/':
    *type = token(div);
    return 1;

  case 'l':
    if ((i = arcana_util_keyword(window, "let"))) {
      *type = token(let);
      return i;
    }
    break;
  }

  if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_') {
    arcana_slice c = pull_word(content, cur);
    *type = token(ident);
    return c.len;
  }

  if ('0' <= ch && ch <= '9') {
    arcana_slice c = pull_number(content, cur);
    *type = token(number);
    return c.len;
  }

  return 0;
}

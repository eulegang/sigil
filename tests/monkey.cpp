
#include "monkey.h"
#include "sigil.h"

#define token(T) (sigil_token_type) monkey_token_type::T

sigil_slice pull_word(sigil_slice content, uint16_t offset) {
  sigil_slice s = {.data = content.data + (size_t)offset, .len = 0};

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

sigil_slice pull_number(sigil_slice content, uint16_t offset) {
  sigil_slice s = {.data = content.data + (size_t)offset, .len = 0};

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

sigil_table *monkey_table() {
  static sigil_table *table = NULL;

  if (!table) {
    table = sigil_table_init();
    sigil_table_push(&table, "let");
  }

  return table;
}

ssize_t monkey_tokenizer(size_t cur, sigil_slice content,
                         sigil_token_type *type) {

  sigil_slice window = sigil_slice_advance(content, cur);
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
    if ((i = sigil_util_keyword(window, "let"))) {
      *type = token(let);
      return i;
    }
    break;
  }

  if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_') {
    sigil_slice c = pull_word(content, cur);
    *type = token(ident);
    return c.len;
  }

  if ('0' <= ch && ch <= '9') {
    sigil_slice c = pull_number(content, cur);
    *type = token(number);
    return c.len;
  }

  return 0;
}

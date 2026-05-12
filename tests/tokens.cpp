#include <gtest/gtest.h>

#include "monkey.h"
#include "sigil.h"
#include "util.h"

const char *buffer = "let x = 1;\nlet y = 2;";

TEST(token, tokens) {
  sigil_slice content = {.data = buffer, .len = strlen(buffer)};

  sigil_tokens_options opts = {
      .content = content,
      .tokenizer = monkey_tokenizer,
  };

  sigil_tokens *tokens = sigil_tokens_init(opts, NULL);
  ASSERT_NE(tokens, nullptr);

  EXPECT_EQ(sigil_tokens_len(tokens), 10);

  sigil_token *base = sigil_tokens_data(tokens);
  sigil_linemeta *meta = sigil_tokens_linemeta(tokens);

  EXPECT_EQ((monkey_token_type)base[0].type, monkey_token_type::let);
  EXPECT_EQ(base[0].off, 0);
  EXPECT_EQ(base[0].len, 3);
  EXPECT_EQ(sigil_tokens_slice(tokens, 0), "let");

  EXPECT_EQ(meta[0].line, 1);
  EXPECT_EQ(meta[0].column, 1);

  EXPECT_EQ((monkey_token_type)base[1].type, monkey_token_type::ident);
  EXPECT_EQ(base[1].off, 4);
  EXPECT_EQ(base[1].len, 1);
  EXPECT_EQ(sigil_tokens_slice(tokens, 1), "x");

  EXPECT_EQ(meta[1].line, 1);
  EXPECT_EQ(meta[1].column, 5);

  EXPECT_EQ((monkey_token_type)base[2].type, monkey_token_type::assign);
  EXPECT_EQ(base[2].off, 6);
  EXPECT_EQ(base[2].len, 1);
  EXPECT_EQ(sigil_tokens_slice(tokens, 2), "=");

  EXPECT_EQ(meta[2].line, 1);
  EXPECT_EQ(meta[2].column, 7);

  EXPECT_EQ((monkey_token_type)base[3].type, monkey_token_type::number);
  EXPECT_EQ(base[3].off, 8);
  EXPECT_EQ(base[3].len, 1);
  EXPECT_EQ(sigil_tokens_slice(tokens, 3), "1");

  EXPECT_EQ(meta[3].line, 1);
  EXPECT_EQ(meta[3].column, 9);

  EXPECT_EQ((monkey_token_type)base[4].type, monkey_token_type::semi);
  EXPECT_EQ(base[4].off, 9);
  EXPECT_EQ(base[4].len, 1);
  EXPECT_EQ(sigil_tokens_slice(tokens, 4), ";");

  EXPECT_EQ(meta[4].line, 1);
  EXPECT_EQ(meta[4].column, 10);

  EXPECT_EQ((monkey_token_type)base[5].type, monkey_token_type::let);
  EXPECT_EQ(base[5].off, 11);
  EXPECT_EQ(base[5].len, 3);
  EXPECT_EQ(sigil_tokens_slice(tokens, 5), "let");

  EXPECT_EQ(meta[5].line, 2);
  EXPECT_EQ(meta[5].column, 1);

  EXPECT_EQ((monkey_token_type)base[6].type, monkey_token_type::ident);
  EXPECT_EQ(base[6].off, 15);
  EXPECT_EQ(base[6].len, 1);
  EXPECT_EQ(sigil_tokens_slice(tokens, 6), "y");

  EXPECT_EQ(meta[6].line, 2);
  EXPECT_EQ(meta[6].column, 5);

  EXPECT_EQ((monkey_token_type)base[7].type, monkey_token_type::assign);
  EXPECT_EQ(base[7].off, 17);
  EXPECT_EQ(base[7].len, 1);
  EXPECT_EQ(sigil_tokens_slice(tokens, 7), "=");

  EXPECT_EQ(meta[7].line, 2);
  EXPECT_EQ(meta[7].column, 7);

  EXPECT_EQ((monkey_token_type)base[8].type, monkey_token_type::number);
  EXPECT_EQ(base[8].off, 19);
  EXPECT_EQ(base[8].len, 1);
  EXPECT_EQ(sigil_tokens_slice(tokens, 8), "2");

  EXPECT_EQ(meta[8].line, 2);
  EXPECT_EQ(meta[8].column, 9);

  EXPECT_EQ((monkey_token_type)base[9].type, monkey_token_type::semi);
  EXPECT_EQ(base[9].off, 20);
  EXPECT_EQ(base[9].len, 1);
  EXPECT_EQ(sigil_tokens_slice(tokens, 9), ";");

  EXPECT_EQ(meta[9].line, 2);
  EXPECT_EQ(meta[9].column, 10);

  EXPECT_LT(sigil_tokens_capacity(tokens), 100000);
  EXPECT_GT(sigil_tokens_capacity(tokens), 1000);

  sigil_tokens_deinit(tokens);
}

TEST(token, capacity) {
  sigil_slice content = {.data = buffer, .len = strlen(buffer)};

  sigil_tokens_options opts = {
      .content = content,
      .tokenizer = monkey_tokenizer,
  };

  sigil_tokens *tokens = sigil_tokens_init(opts, NULL);

  ASSERT_NE(tokens, nullptr);

  EXPECT_LT(sigil_tokens_capacity(tokens), 100000);
  EXPECT_GT(sigil_tokens_capacity(tokens), 1000);

  sigil_tokens_deinit(tokens);
}

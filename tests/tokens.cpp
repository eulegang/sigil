#include <gtest/gtest.h>

#include "monkey.h"
#include "sigil.h"
#include "util.h"

TEST(token, tokens) {
  std::string_view buffer = "let x = 1;\nlet y = 2;";

  sigil::Tokens<monkey_token_type> tokens(buffer, monkey_tokenizer);

  EXPECT_EQ(tokens.length(), 10);

  EXPECT_EQ(tokens[0].type, monkey_token_type::let);
  EXPECT_EQ(tokens[0].off, 0);
  EXPECT_EQ(tokens[0].len, 3);
  EXPECT_EQ(tokens.content(0), "let");

  EXPECT_EQ(tokens.linemeta(0).line, 1);
  EXPECT_EQ(tokens.linemeta(0).column, 1);

  EXPECT_EQ(tokens[1].type, monkey_token_type::ident);
  EXPECT_EQ(tokens[1].off, 4);
  EXPECT_EQ(tokens[1].len, 1);
  EXPECT_EQ(tokens.content(1), "x");

  EXPECT_EQ(tokens.linemeta(1).line, 1);
  EXPECT_EQ(tokens.linemeta(1).column, 5);

  EXPECT_EQ(tokens[2].type, monkey_token_type::assign);
  EXPECT_EQ(tokens[2].off, 6);
  EXPECT_EQ(tokens[2].len, 1);
  EXPECT_EQ(tokens.content(2), "=");

  EXPECT_EQ(tokens.linemeta(2).line, 1);
  EXPECT_EQ(tokens.linemeta(2).column, 7);

  EXPECT_EQ(tokens[3].type, monkey_token_type::number);
  EXPECT_EQ(tokens[3].off, 8);
  EXPECT_EQ(tokens[3].len, 1);
  EXPECT_EQ(tokens.content(3), "1");

  EXPECT_EQ(tokens.linemeta(3).line, 1);
  EXPECT_EQ(tokens.linemeta(3).column, 9);

  EXPECT_EQ(tokens[4].type, monkey_token_type::semi);
  EXPECT_EQ(tokens[4].off, 9);
  EXPECT_EQ(tokens[4].len, 1);
  EXPECT_EQ(tokens.content(4), ";");

  EXPECT_EQ(tokens.linemeta(4).line, 1);
  EXPECT_EQ(tokens.linemeta(4).column, 10);

  EXPECT_EQ(tokens[5].type, monkey_token_type::let);
  EXPECT_EQ(tokens[5].off, 11);
  EXPECT_EQ(tokens[5].len, 3);
  EXPECT_EQ(tokens.content(5), "let");

  EXPECT_EQ(tokens.linemeta(5).line, 2);
  EXPECT_EQ(tokens.linemeta(5).column, 1);

  EXPECT_EQ(tokens[6].type, monkey_token_type::ident);
  EXPECT_EQ(tokens[6].off, 15);
  EXPECT_EQ(tokens[6].len, 1);
  EXPECT_EQ(tokens.content(6), "y");

  EXPECT_EQ(tokens.linemeta(6).line, 2);
  EXPECT_EQ(tokens.linemeta(6).column, 5);

  EXPECT_EQ(tokens[7].type, monkey_token_type::assign);
  EXPECT_EQ(tokens[7].off, 17);
  EXPECT_EQ(tokens[7].len, 1);
  EXPECT_EQ(tokens.content(7), "=");

  EXPECT_EQ(tokens.linemeta(7).line, 2);
  EXPECT_EQ(tokens.linemeta(7).column, 7);

  EXPECT_EQ(tokens[8].type, monkey_token_type::number);
  EXPECT_EQ(tokens[8].off, 19);
  EXPECT_EQ(tokens[8].len, 1);
  EXPECT_EQ(tokens.content(8), "2");

  EXPECT_EQ(tokens.linemeta(8).line, 2);
  EXPECT_EQ(tokens.linemeta(8).column, 9);

  EXPECT_EQ(tokens[9].type, monkey_token_type::semi);
  EXPECT_EQ(tokens[9].off, 20);
  EXPECT_EQ(tokens[9].len, 1);
  EXPECT_EQ(tokens.content(9), ";");

  EXPECT_EQ(tokens.linemeta(9).line, 2);
  EXPECT_EQ(tokens.linemeta(9).column, 10);
}

TEST(token, capacity) {
  std::string_view buffer = "let x = 1;\nlet y = 2;";
  sigil::Tokens<monkey_token_type> tokens{buffer, monkey_tokenizer};

  EXPECT_LT(tokens.capacity(), 100000);
  EXPECT_GT(tokens.capacity(), 1000);
}

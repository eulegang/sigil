#include <gtest/gtest.h>

#include "arcana.h"
#include "monkey.h"

TEST(token, table) {
  arcana_token_table_t *table = arcana_token_table_init();
  ASSERT_NE(table, nullptr);

  arcana_token_table_push(&table, "let");
  arcana_token_table_push(&table, "semi");
  arcana_token_table_push(&table, "open");
  arcana_token_table_push(&table, "close");
  arcana_token_table_push(&table, "assign");

  EXPECT_EQ(arcana_token_table_len(table), 5);

  EXPECT_EQ(arcana_token_table_data(table)[0], "let");
  EXPECT_EQ(arcana_token_table_data(table)[1], "semi");
  EXPECT_EQ(arcana_token_table_data(table)[2], "open");
  EXPECT_EQ(arcana_token_table_data(table)[3], "close");
  EXPECT_EQ(arcana_token_table_data(table)[4], "assign");

  arcana_token_table_deinit(table);
}

TEST(token, tokens) {
  const char *buffer = "let x = 1;";

  arcana_slice content = {.data = buffer, .len = strlen(buffer)};

  arcana_tokens_options opts = {
      .content = content,
      .tokenizer = monkey_tokenizer,
  };

  arcana_tokens_t *tokens = arcana_tokens_init(opts);
  ASSERT_NE(tokens, nullptr);

  EXPECT_EQ(arcana_tokens_len(tokens), 5);

  arcana_token *base = arcana_tokens_data(tokens);
  EXPECT_EQ(base[0].type, monkey_token_type_let);
  EXPECT_EQ(base[0].off, 0);
  EXPECT_EQ(base[0].len, 3);

  EXPECT_EQ(base[1].type, monkey_token_type_ident);
  EXPECT_EQ(base[1].off, 4);
  EXPECT_EQ(base[1].len, 1);

  EXPECT_EQ(base[2].type, monkey_token_type_assign);
  EXPECT_EQ(base[2].off, 6);
  EXPECT_EQ(base[2].len, 1);

  EXPECT_EQ(base[3].type, monkey_token_type_number);
  EXPECT_EQ(base[3].off, 8);
  EXPECT_EQ(base[3].len, 1);

  EXPECT_EQ(base[4].type, monkey_token_type_semi);
  EXPECT_EQ(base[4].off, 9);
  EXPECT_EQ(base[4].len, 1);

  arcana_tokens_deinit(tokens);
}

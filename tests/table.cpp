#include <gtest/gtest.h>

#include "sigil.h"

TEST(token_table, insertion) {
  sigil_token_table_t *table = sigil_token_table_init();
  ASSERT_NE(table, nullptr);

  sigil_token_table_push(&table, "let");
  sigil_token_table_push(&table, "semi");
  sigil_token_table_push(&table, "open");
  sigil_token_table_push(&table, "close");
  sigil_token_table_push(&table, "assign");

  EXPECT_EQ(sigil_token_table_len(table), 5);

  EXPECT_EQ(sigil_token_table_data(table)[0], "let");
  EXPECT_EQ(sigil_token_table_data(table)[1], "semi");
  EXPECT_EQ(sigil_token_table_data(table)[2], "open");
  EXPECT_EQ(sigil_token_table_data(table)[3], "close");
  EXPECT_EQ(sigil_token_table_data(table)[4], "assign");

  sigil_token_table_deinit(table);
}

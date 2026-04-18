#include <gtest/gtest.h>

#include "arcana.h"

TEST(token_table, insertion) {
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

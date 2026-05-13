#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <sstream>

#include "monkey.h"
#include "sigil.h"
#include "util.h"

TEST(parse, basic) {
  std::string_view buffer = "let x = 1;\nlet y = 12;";
  sigil::Tokens<monkey_token_type> tokens{buffer, monkey_tokenizer};
  sigil::Ast<monkey_node_type> ast{monkey_parser, tokens};

  ASSERT_EQ(ast.node_count(), 7);
  ASSERT_EQ(ast.data_size(), 16);

  EXPECT_EQ(ast[0].child, 2);
  EXPECT_EQ(ast[0].next, 4);
  EXPECT_EQ(ast[0].offset, 0xFFFF);
  EXPECT_EQ(ast[0].type, monkey_node_type::let);

  EXPECT_EQ(ast[1].child, 2);
  EXPECT_EQ(ast[1].next, 0);
  EXPECT_EQ(ast[1].offset, 0xFFFF);
  EXPECT_EQ(ast[1].type, monkey_node_type::let);

  EXPECT_EQ(ast[2].child, 0);
  EXPECT_EQ(ast[2].next, 3);
  EXPECT_EQ(ast[2].offset, 0);
  EXPECT_EQ(ast[2].type, monkey_node_type::ident);

  EXPECT_EQ(ast[3].child, 0);
  EXPECT_EQ(ast[3].next, 0);
  EXPECT_EQ(ast[3].offset, 4);
  EXPECT_EQ(ast[3].type, monkey_node_type::lit);

  EXPECT_EQ(ast[4].child, 5);
  EXPECT_EQ(ast[4].next, 0);
  EXPECT_EQ(ast[4].offset, 0xFFFF);
  EXPECT_EQ(ast[4].type, monkey_node_type::let);

  EXPECT_EQ(ast[5].child, 0);
  EXPECT_EQ(ast[5].next, 6);
  EXPECT_EQ(ast[5].offset, 8);
  EXPECT_EQ(ast[5].type, monkey_node_type::ident);

  EXPECT_EQ(ast[6].child, 0);
  EXPECT_EQ(ast[6].next, 0);
  EXPECT_EQ(ast[6].offset, 12);
  EXPECT_EQ(ast[6].type, monkey_node_type::lit);

  EXPECT_EQ(ast.data<monkey_slice>(ast[0].offset), nullptr);
  EXPECT_EQ(ast.data<monkey_slice>(ast[1].offset), nullptr);
  EXPECT_EQ(ast.data<monkey_slice>(ast[4].offset), nullptr);

  ASSERT_NE(ast.data<monkey_slice>(ast[2].offset), nullptr);
  EXPECT_EQ(ast.data<monkey_slice>(ast[2].offset)->base, 4);
  EXPECT_EQ(ast.data<monkey_slice>(ast[2].offset)->len, 1);

  ASSERT_NE(ast.data<monkey_slice>(ast[3].offset), nullptr);
  EXPECT_EQ(ast.data<monkey_slice>(ast[3].offset)->base, 8);
  EXPECT_EQ(ast.data<monkey_slice>(ast[3].offset)->len, 1);

  ASSERT_NE(ast.data<monkey_slice>(ast[5].offset), nullptr);
  EXPECT_EQ(ast.data<monkey_slice>(ast[5].offset)->base, 15);
  EXPECT_EQ(ast.data<monkey_slice>(ast[5].offset)->len, 1);

  ASSERT_NE(ast.data<monkey_slice>(ast[6].offset), nullptr);
  EXPECT_EQ(ast.data<monkey_slice>(ast[6].offset)->base, 19);
  EXPECT_EQ(ast.data<monkey_slice>(ast[6].offset)->len, 2);
}

TEST(parse, expr) {
  std::string_view buffer =
      "let x = 1;\nlet y = 12;\nlet z = (x + 1) + 3 * (y + 1);";

  sigil::Tokens<monkey_token_type> tokens{buffer, monkey_tokenizer};
  sigil::Ast<monkey_node_type> ast{monkey_parser, tokens};

  std::shared_ptr<std::stringstream> output =
      std::make_shared<std::stringstream>();

  DebugCtx ctx = {
      .out = output.get(),
      .buffer = buffer,
  };

  ast.visit(&ctx, monkey_debug_tree);
  std::string s = output->str();

  EXPECT_EQ(s, "let\n"
               "  ident (x)\n"
               "  lit (1)\n"
               "let\n"
               "  ident (y)\n"
               "  lit (12)\n"
               "let\n"
               "  ident (z)\n"
               "  +\n"
               "    +\n"
               "      ident (x)\n"
               "      lit (1)\n"
               "    *\n"
               "      lit (3)\n"
               "      +\n"
               "        ident (y)\n"
               "        lit (1)\n"
               "");
}

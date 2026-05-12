#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <sstream>

#include "monkey.h"
#include "sigil.h"
#include "util.h"

TEST(parse, basic) {
  static const char *buffer = "let x = 1;\nlet y = 12;";

  sigil_slice content = {.data = buffer, .len = strlen(buffer)};

  sigil_tokens_options opts = {
      .content = content,
      .tokenizer = monkey_tokenizer,
  };

  sigil_tokens *tokens = sigil_tokens_init(opts, NULL);
  ASSERT_NE(tokens, nullptr);

  sigil_ast *ast = sigil_parser_parse(monkey_parser, tokens, NULL);
  ASSERT_NE(ast, nullptr);

  std::span<sigil_node> nodes{
      sigil_ast_nodes(ast),
      sigil_ast_node_count(ast),
  };

  ASSERT_EQ(nodes.size(), 7);
  ASSERT_EQ(sigil_ast_data_size(ast), 16);

  EXPECT_EQ(nodes[0].child, 2);
  EXPECT_EQ(nodes[0].next, 4);
  EXPECT_EQ(nodes[0].offset, 0xFFFF);
  EXPECT_EQ(nodes[0].type, (uint16_t)monkey_node_type::let);

  EXPECT_EQ(nodes[1].child, 2);
  EXPECT_EQ(nodes[1].next, 0);
  EXPECT_EQ(nodes[1].offset, 0xFFFF);
  EXPECT_EQ(nodes[1].type, (uint16_t)monkey_node_type::let);

  EXPECT_EQ(nodes[2].child, 0);
  EXPECT_EQ(nodes[2].next, 3);
  EXPECT_EQ(nodes[2].offset, 0);
  EXPECT_EQ(nodes[2].type, (uint16_t)monkey_node_type::ident);

  EXPECT_EQ(nodes[3].child, 0);
  EXPECT_EQ(nodes[3].next, 0);
  EXPECT_EQ(nodes[3].offset, 4);
  EXPECT_EQ(nodes[3].type, (uint16_t)monkey_node_type::lit);

  EXPECT_EQ(nodes[4].child, 5);
  EXPECT_EQ(nodes[4].next, 0);
  EXPECT_EQ(nodes[4].offset, 0xFFFF);
  EXPECT_EQ(nodes[4].type, (uint16_t)monkey_node_type::let);

  EXPECT_EQ(nodes[5].child, 0);
  EXPECT_EQ(nodes[5].next, 6);
  EXPECT_EQ(nodes[5].offset, 8);
  EXPECT_EQ(nodes[5].type, (uint16_t)monkey_node_type::ident);

  EXPECT_EQ(nodes[6].child, 0);
  EXPECT_EQ(nodes[6].next, 0);
  EXPECT_EQ(nodes[6].offset, 12);
  EXPECT_EQ(nodes[6].type, (uint16_t)monkey_node_type::lit);

  monkey_slice *addr = (monkey_slice *)sigil_ast_data(ast);

  EXPECT_EQ(addr[0].base, 4);
  EXPECT_EQ(addr[0].len, 1);

  EXPECT_EQ(addr[1].base, 8);
  EXPECT_EQ(addr[1].len, 1);

  EXPECT_EQ(addr[2].base, 15);
  EXPECT_EQ(addr[2].len, 1);

  EXPECT_EQ(addr[3].base, 19);
  EXPECT_EQ(addr[3].len, 2);

  sigil_ast_deinit(ast);
  sigil_tokens_deinit(tokens);
}

TEST(parse, expr) {
  static const char *buffer =
      "let x = 1;\nlet y = 12;\nlet z = (x + 1) + 3 * (y + 1);";
  std::string_view sv{buffer};

  sigil_slice content = {.data = buffer, .len = strlen(buffer)};

  sigil_tokens_options opts = {
      .content = content,
      .tokenizer = monkey_tokenizer,
  };

  sigil_tokens_error err;
  sigil_tokens *tokens = sigil_tokens_init(opts, &err);
  ASSERT_NE(tokens, nullptr)
      << std::format("position {} {}", err.pos, sv.substr(err.pos));

  sigil_ast *ast = sigil_parser_parse(monkey_parser, tokens, NULL);
  ASSERT_NE(ast, nullptr);

  std::shared_ptr<std::stringstream> output =
      std::make_shared<std::stringstream>();
  sigil_ast_visit(ast, content, output.get(), monkey_debug_tree);
  std::span<sigil_node> nodes{
      sigil_ast_nodes(ast),
      sigil_ast_node_count(ast),
  };

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

  ASSERT_EQ(nodes.size(), 18);
  ASSERT_EQ(sigil_ast_data_size(ast), 56);

  sigil_ast_deinit(ast);
  sigil_tokens_deinit(tokens);
}

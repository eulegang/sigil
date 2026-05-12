
#include "monkey.h"
#include "sigil.h"
#include <iostream>
#include <string>

void monkey_debug_tree(uint16_t, sigil::Ast<monkey_node_type>::Node node,
                       void *addr, size_t level, DebugCtx *ctx) {

  std::ostream &out = *ctx->out;
  std::string_view content = ctx->buffer;

  out << std::string(2 * level, ' ');

  switch (node.type) {
  case monkey_node_type::let:
    out << "let" << std::endl;
    break;

  case monkey_node_type::ident: {
    monkey_slice ms = *(monkey_slice *)addr;
    std::string_view slice = content.substr(ms.base, ms.len);

    out << "ident" << " (" << slice << ")" << std::endl;
  } break;

  case monkey_node_type::lit: {
    monkey_slice ms = *(monkey_slice *)addr;
    std::string_view slice = content.substr(ms.base, ms.len);
    out << "lit" << " (" << slice << ")" << std::endl;
  } break;

  case monkey_node_type::sub:
    out << "-" << std::endl;
    break;

  case monkey_node_type::add:
    out << "+" << std::endl;
    break;
  case monkey_node_type::mult:
    out << "*" << std::endl;
    break;
  case monkey_node_type::div:
    out << "/" << std::endl;
    break;
  }
}


#include "monkey.h"
#include "sigil.h"
#include <iostream>
#include <string>

void monkey_debug_tree(sigil_node node, void *addr, size_t level,
                       sigil_slice content, void *ctx) {

  std::ostream &out = *(std::ostream *)ctx;

  auto ty = (monkey_node_type)node.type;

  out << std::string(2 * level, ' ');

  switch (ty) {
  case monkey_node_type::let:
    out << "let" << std::endl;
    break;

  case monkey_node_type::ident: {
    monkey_slice ms = *(monkey_slice *)addr;
    std::string_view slice{content.data + ms.base, ms.len};

    out << "ident" << " (" << slice << ")" << std::endl;
  } break;

  case monkey_node_type::lit: {
    monkey_slice ms = *(monkey_slice *)addr;
    std::string_view slice{content.data + ms.base, ms.len};
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

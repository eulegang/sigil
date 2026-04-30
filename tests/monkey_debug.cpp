
#include "arcana.h"
#include "monkey.h"
#include <iostream>
#include <string>

void monkey_debug_tree(arcana_parse_node node, void *addr, size_t level,
                       arcana_slice content) {
  auto ty = (monkey_node_type)node.type;

  std::cout << std::string(2 * level, ' ');

  switch (ty) {
  case monkey_node_type::let:
    std::cout << "let" << std::endl;
    break;

  case monkey_node_type::ident: {
    monkey_slice ms = *(monkey_slice *)addr;
    std::string_view slice{content.data + ms.base, ms.len};

    std::cout << "ident" << " (" << slice << ")" << std::endl;
  } break;

  case monkey_node_type::lit: {
    monkey_slice ms = *(monkey_slice *)addr;
    std::string_view slice{content.data + ms.base, ms.len};
    std::cout << "lit" << " (" << slice << ")" << std::endl;
  } break;
  }
}

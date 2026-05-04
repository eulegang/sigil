#include "arcana.h"
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "types.h"

void arcana_ast_deinit(arcana_ast *ast) { munmap(ast, ast->cap); }

arcana_node *arcana_ast_nodes(arcana_ast *ast) { return (void *)(ast + 1); }
uint16_t arcana_ast_node_count(arcana_ast *ast) { return ast->nodes; }
uint16_t arcana_ast_data_size(arcana_ast *ast) { return ast->data; }
void *arcana_ast_data(arcana_ast *ast) { return (void *)ast + (ast->cap / 2); }

void arcana_ast_visit_recur(arcana_ast *ast, arcana_slice content, void *ctx,
                            arcana_node node, size_t level,
                            arcana_ast_visit_fn fn) {
  void *addr = NULL;

  if (node.offset != 0xFFFF) {
    addr = arcana_ast_data(ast) + node.offset;
  }

  fn(node, addr, level, content, ctx);

  if (node.child != 0) {
    arcana_ast_visit_recur(ast, content, ctx, arcana_ast_nodes(ast)[node.child],
                           level + 1, fn);
  }

  if (node.next != 0) {
    arcana_ast_visit_recur(ast, content, ctx, arcana_ast_nodes(ast)[node.next],
                           level, fn);
  }
}

void arcana_ast_visit(arcana_ast *ast, arcana_slice content, void *ctx,
                      arcana_ast_visit_fn fn) {
  if (ast->nodes == 0) {
    return;
  }

  uint16_t cur = 0;
  arcana_node *base = arcana_ast_nodes(ast);
  arcana_node node = base[cur];

  arcana_ast_visit_recur(ast, content, ctx, node, 0, fn);
}

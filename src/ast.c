#include "sigil.h"
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#include "types.h"

void sigil_ast_deinit(sigil_ast *ast) {
  if (ast)
    munmap(ast, ast->cap);
}

sigil_node *sigil_ast_nodes(sigil_ast *ast) { return (void *)(ast + 1); }
uint16_t sigil_ast_node_count(sigil_ast *ast) { return ast->nodes; }
uint16_t sigil_ast_data_size(sigil_ast *ast) { return ast->data; }
void *sigil_ast_data(sigil_ast *ast) { return (void *)ast + (ast->cap / 2); }

void sigil_ast_visit_recur(sigil_ast *ast, void *ctx, uint16_t id, size_t level,
                           sigil_ast_visit_fn fn) {
  void *addr = NULL;
  sigil_node *base = sigil_ast_nodes(ast);
  sigil_node node = base[id];

  if (node.offset != 0xFFFF) {
    addr = sigil_ast_data(ast) + node.offset;
  }

  fn(id, base[id], addr, level, ctx);

  if (node.child != 0) {
    sigil_ast_visit_recur(ast, ctx, node.child, level + 1, fn);
  }

  if (node.next != 0) {
    sigil_ast_visit_recur(ast, ctx, node.next, level, fn);
  }
}

void sigil_ast_visit(sigil_ast *ast, void *ctx, sigil_ast_visit_fn fn) {
  if (ast->nodes == 0) {
    return;
  }

  uint16_t cur = 0;
  sigil_node *base = sigil_ast_nodes(ast);

  sigil_ast_visit_recur(ast, ctx, cur, 0, fn);
}

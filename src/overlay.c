#include "sigil.h"
#include "types.h"
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

typedef struct sigil_overlay sigil_overlay;

sigil_overlay *sigil_overlay_init(sigil_ast *ast, size_t pages) {
  size_t len = pages * getpagesize();
  sigil_overlay *self =
      mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);

  if (self == MAP_FAILED) {
    return NULL;
  }

  self->len = len;
  self->entries = ast->nodes;
  self->data = 0;
  memset(self + 1, 0xFF, ast->nodes * 2);

  return self;
}

void sigil_overlay_deinit(sigil_overlay *self) {
  if (self)
    munmap(self, self->len);
}

void *sigil_overlay_alloc(sigil_overlay *self, uint16_t node, size_t size) {
  uint16_t *entries = (uint16_t *)(self + 1);
  if (entries[node] != 0xFFFF) {
    return NULL;
  }

  entries[node] = self->data;
  self->data += size;

  char *base = (char *)(entries + self->entries);

  return (void *)(base + entries[node]);
}

void *sigil_overlay_resolve(sigil_overlay *self, uint16_t node) {
  uint16_t *entries = (uint16_t *)(self + 1);
  uint16_t offset = entries[node];

  if (offset == 0xFFFF) {
    return NULL;
  }

  char *base = (char *)(entries + self->entries);
  return (void *)(base + offset);
}

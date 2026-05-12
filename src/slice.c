#include "sigil.h"

sigil_slice sigil_slice_advance(sigil_slice slice, size_t inc) {
  return (sigil_slice){
      .data = slice.data + inc,
      .len = slice.len - inc,
  };
}

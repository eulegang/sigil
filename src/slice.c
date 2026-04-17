#include "arcana.h"

arcana_slice arcana_slice_advance(arcana_slice slice, size_t inc) {
  return (arcana_slice){
      .data = slice.data + inc,
      .len = slice.len - inc,
  };
}

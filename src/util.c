#include "arcana.h"

ssize_t arcana_util_take_while(arcana_slice slice, bool (*pred)(char)) {
  ssize_t amount = 0;
  for (; slice.len; slice.data++, slice.len--, amount++) {
    if (!pred(*slice.data))
      break;
  }

  return amount;
}

ssize_t arcana_util_take_stateful(arcana_slice slice, void *state,
                                  bool (*iter)(void *, char)) {

  ssize_t amount = 0;
  for (; slice.len; slice.data++, slice.len--, amount++) {
    if (!iter(state, *slice.data))
      break;
  }

  return amount;
}

ssize_t arcana_util_keyword(arcana_slice slice, const char *keyword) {
  ssize_t res = 0;

  while (*keyword) {
    if (!slice.len)
      return 0;

    if (*slice.data != *keyword)
      return 0;

    keyword++;
    slice = arcana_slice_advance(slice, 1);
    res++;
  }

  return res;
}

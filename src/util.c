#include "sigil.h"

ssize_t sigil_util_take_while(sigil_slice slice, bool (*pred)(char)) {
  ssize_t amount = 0;
  for (; slice.len; slice.data++, slice.len--, amount++) {
    if (!pred(*slice.data))
      break;
  }

  return amount;
}

ssize_t sigil_util_take_stateful(sigil_slice slice, void *state,
                                 bool (*iter)(void *, char)) {

  ssize_t amount = 0;
  for (; slice.len; slice.data++, slice.len--, amount++) {
    if (!iter(state, *slice.data))
      break;
  }

  return amount;
}

ssize_t sigil_util_keyword(sigil_slice slice, const char *keyword) {
  ssize_t res = 0;

  while (*keyword) {
    if (!slice.len)
      return 0;

    if (*slice.data != *keyword)
      return 0;

    keyword++;
    slice = sigil_slice_advance(slice, 1);
    res++;
  }

  return res;
}

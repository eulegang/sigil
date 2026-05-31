#include <gtest/gtest.h>

#include "sigil.h"

TEST(util_keyword, normal_match) {
  const char *subject = "priv thingy";
  sigil_slice slice = {
      .data = subject,
      .len = strlen(subject),
  };
  EXPECT_EQ(sigil_util_keyword(slice, "priv"), 4);
}

TEST(util_keyword, unmatch) {
  const char *subject = "thingy";
  sigil_slice slice = {
      .data = subject,
      .len = strlen(subject),
  };
  EXPECT_EQ(sigil_util_keyword(slice, "priv"), 0);
}

TEST(util_keyword, end) {
  const char *subject = "priv";
  sigil_slice slice = {
      .data = subject,
      .len = strlen(subject),
  };
  EXPECT_EQ(sigil_util_keyword(slice, "priv"), 4);
}

TEST(util_keyword, longer) {
  const char *subject = "private";
  sigil_slice slice = {
      .data = subject,
      .len = strlen(subject),
  };
  EXPECT_EQ(sigil_util_keyword(slice, "priv"), 0);
}

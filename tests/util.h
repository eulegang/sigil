#pragma once

#include <string_view>

#include "sigil.h"

bool operator==(const sigil_slice, std::string_view);
std::ostream &operator<<(std::ostream &os, const sigil_slice);

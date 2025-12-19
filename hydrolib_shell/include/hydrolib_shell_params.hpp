#pragma once

#include "hydrolib_cstring.hpp"

namespace hydrolib::shell {
static constexpr int kMaxArgsCount = 20;
static constexpr int kMaxCommandLength = kMaxArgsCount * 5;

using CommandString = strings::CString<kMaxCommandLength>;
}  // namespace hydrolib::shell

#pragma once

#include <optional>
#include <string_view>

#include "hydrolib_cat.hpp"
#include "hydrolib_echo.hpp"

namespace hydrolib::shell {
class CommandMap {
 public:
  using CommandType = decltype(&hydrolib::shell::Echo);

 public:
  std::optional<CommandType> operator[](std::string_view command) {
    if (command == "echo") {
      return hydrolib::shell::Echo;
    } else if (command == "cat") {
      return hydrolib::shell::Cat;
    }
    return std::nullopt;
  }
};

inline CommandMap command_map;
}  // namespace hydrolib::shell

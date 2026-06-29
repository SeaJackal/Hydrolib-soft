#pragma once

#include <algorithm>
#include <ranges>
#include <span>
#include <string_view>
#include <utility>

#include "hydrolib_shell_params.hpp"

namespace hydrolib::shell::parser {

inline auto SplitToArgs(std::string_view command_str) {
  auto to_string_view = [](auto word) -> std::string_view {
    auto first = std::ranges::begin(word);
    return std::string_view(
        &*first, static_cast<std::size_t>(std::ranges::distance(word)));
  };
  return command_str | std::views::split(' ') |
         std::views::filter([](auto word) { return !word.empty(); }) |
         std::views::transform(to_string_view);
}

using ArgsView = decltype(SplitToArgs(std::declval<std::string_view>()));

class CommandInfo {
 public:
  explicit CommandInfo(ArgsView args);

  [[nodiscard]] std::string_view GetCommand() const;
  [[nodiscard]] std::span<const std::string_view> GetArgs() const;

 private:
  std::string_view command_;
  std::array<std::string_view, kMaxArgsCount> args_{};
  int args_number_;
};

inline CommandInfo::CommandInfo(ArgsView args)
    : command_(*args.begin()),
      args_number_(static_cast<int>(std::ranges::copy(args, args_.begin()).out -
                                    args_.begin())) {}

inline std::string_view CommandInfo::GetCommand() const { return command_; }

inline std::span<const std::string_view> CommandInfo::GetArgs() const {
  return std::span(args_).subspan(0, args_number_);
}

}  // namespace hydrolib::shell::parser
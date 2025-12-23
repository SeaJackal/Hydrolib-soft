#pragma once

#include <unistd.h>

#include <concepts>
#include <cstring>
#include <optional>
#include <string_view>

#include "hydrolib_func_concepts.hpp"
#include "hydrolib_shell_params.hpp"

namespace hydrolib::shell {

template <typename T, typename KeyType, typename ValueType>
concept ReadableMapConcept = requires(T map, KeyType key) {
  { map[key] } -> std::convertible_to<ValueType>;
};

template <typename T, typename Func>
concept CommandMapConcept =
    ReadableMapConcept<T, std::string_view, std::optional<Func>> &&
    concepts::func::FuncConcept<Func, int, int, char *[]>;

class ExecutionManager {
 public:
  static void SetRunningFalse();
  static bool IsRunning();

 private:
  static void SetRunningTrue();

  static inline bool g_is_running = false;
};

template <typename Func, CommandMapConcept<Func> Map>
class Interpreter {
  friend ExecutionManager;

 public:
  constexpr Interpreter(Map &handlers);

 public:
  int Process(CommandString command);

 private:
  Map &handlers_;
};

template <typename Func, CommandMapConcept<Func> Map>
constexpr Interpreter<Func, Map>::Interpreter(Map &handlers)
    : handlers_(handlers) {}

template <typename Func, CommandMapConcept<Func> Map>
int Interpreter<Func, Map>::Process(CommandString command) {
  int argc = 0;
  char *argv[kMaxArgsCount];
  if (command[0] != ' ') {
    argv[0] = static_cast<char *>(command);
    argc++;
  }
  for (int pos = 0; pos < command.GetLength(); pos++) {
    if (command[pos] == ' ') {
      command[pos] = '\0';
      pos++;
      while (command[pos] == ' ') {
        pos++;
      }
      if (command[pos]) {
        if (argc > kMaxArgsCount) {
          return -1;
        }
        argv[argc] = static_cast<char *>(command) + pos;
        argc++;
      }
    }
  }
  if (handlers_[static_cast<std::string_view>(argv[0])]) {
    g_is_running = true;  // TODO: Add tests
    optind = 1;
    opterr = 0;
    auto result =
        (*handlers_[static_cast<std::string_view>(argv[0])])(argc, argv);
    g_is_running = false;
    return result;
  }
  return -2;
}

void ExecutionManager::SetRunningFalse() { g_is_running = false; }
bool ExecutionManager::IsRunning() { return g_is_running; }
void ExecutionManager::SetRunningTrue() { g_is_running = true; }
}  // namespace hydrolib::shell

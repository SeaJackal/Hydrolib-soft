#pragma once

#include "hydrolib_return_codes.hpp"
#include "hydrolib_shell_interpreter.hpp"
#include "hydrolib_shell_ostream.hpp"
#include "hydrolib_shell_terminal.hpp"

namespace hydrolib::shell {
template <concepts::stream::ByteFullStreamConcept Stream, typename Func,
          CommandMapConcept<Func> Map>
class Shell {
 public:
  static constexpr std::string_view kPrompt = "\r\nhydrosh > ";

 public:
  constexpr Shell(Stream &stream, Map &handlers);

 public:
  hydrolib::ReturnCode Process();

 private:
  Stream &stream_;
  Terminal<Stream> terminal_;
  Interpreter<Func, Map> interpreter_;
  StreamWrapper<Stream> cout_wrapper_;

  int last_error_code_;
};

inline Ostream cout{};

template <concepts::stream::ByteFullStreamConcept Stream, typename Func,
          CommandMapConcept<Func> Map>
constexpr Shell<Stream, Func, Map>::Shell(Stream &stream, Map &handlers)
    : stream_(stream),
      terminal_(stream),
      interpreter_(handlers),
      cout_wrapper_(stream),
      last_error_code_(0) {
  cout = Ostream(cout_wrapper_);
}

template <concepts::stream::ByteFullStreamConcept Stream, typename Func,
          CommandMapConcept<Func> Map>
hydrolib::ReturnCode Shell<Stream, Func, Map>::Process() {
  auto terminal_result = terminal_.Process();
  if (terminal_result != hydrolib::ReturnCode::OK) {
    return terminal_result;
  }
  auto command = terminal_.GetCommand();
  last_error_code_ = interpreter_.Process(command);
  write(stream_, kPrompt.data(), kPrompt.length());
  if (last_error_code_ != 0) {
    return hydrolib::ReturnCode::ERROR;
  } else {
    return hydrolib::ReturnCode::OK;
  }
}

}  // namespace hydrolib::shell

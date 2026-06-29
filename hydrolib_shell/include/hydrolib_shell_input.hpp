#pragma once

#include <string_view>

#include "hydrolib_cstring.hpp"
#include "hydrolib_return_codes.hpp"
#include "hydrolib_shell_params.hpp"
#include "hydrolib_stream_concepts.hpp"

namespace hydrolib::shell {

template <concepts::stream::ByteFullStreamConcept Stream>
class Input {
 public:
  constexpr explicit Input(Stream& stream);
  Input(const Input&) = delete;
  Input(Input&&) = delete;
  Input& operator=(const Input&) = delete;
  Input& operator=(Input&&) = delete;
  ~Input() = default;

  [[nodiscard]] Expected<std::string_view> Process();

 private:
  hydrolib::ReturnCode InterpretChar(char last_char);

  Stream& stream_;

  strings::CString<kMaxCommandLength> command_str_;
  bool reset_flag_ = false;
};

template <concepts::stream::ByteFullStreamConcept Stream>
constexpr Input<Stream>::Input(Stream& stream) : stream_(stream) {}

template <concepts::stream::ByteFullStreamConcept Stream>
Expected<std::string_view> Input<Stream>::Process() {
  char last_char = 0;
  const auto read_length = read(stream_, &last_char, 1);
  if (read_length == 0) {
    return {static_cast<std::string_view>(command_str_),
            hydrolib::ReturnCode::NO_DATA};
  }
  if (read_length < 0) {
    return hydrolib::ReturnCode::ERROR;
  }
  const auto result = InterpretChar(last_char);
  return {static_cast<std::string_view>(command_str_), result};
}

template <concepts::stream::ByteFullStreamConcept Stream>
hydrolib::ReturnCode Input<Stream>::InterpretChar(char last_char) {
  constexpr char kBackspaceChar = 0x08;
  switch (last_char) {
    case kBackspaceChar:
      if (command_str_.GetLength() != 0) {
        command_str_.Pop(1);
      }
      break;
    case '\n':
      return hydrolib::ReturnCode::OK;
    default:
      // if (current_command_string_length_ >= kMaxCommandLength) {
      //   return hydrolib::ReturnCode::OVERFLOW;
      // }
      command_str_.Push(&last_char, 1);
      break;
  }
  return hydrolib::ReturnCode::NO_DATA;
}
}  // namespace hydrolib::shell

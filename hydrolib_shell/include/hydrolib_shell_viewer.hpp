#pragma once

#include <cassert>
#include <string_view>

#include "hydrolib_cstring.hpp"
#include "hydrolib_return_codes.hpp"
#include "hydrolib_shell_params.hpp"
#include "hydrolib_stream_concepts.hpp"

namespace hydrolib::shell {
template <concepts::stream::ByteFullStreamConcept Stream>
class Viewer {
 public:
  static constexpr std::string_view kWelcome = "hydrosh > ";

  constexpr explicit Viewer(Stream& stream);
  Viewer(const Viewer&) = delete;
  Viewer(Viewer&&) = delete;
  Viewer& operator=(const Viewer&) = delete;
  Viewer& operator=(Viewer&&) = delete;
  ~Viewer() = default;

  ReturnCode Process(std::string_view str);
  ReturnCode Enter();
  ReturnCode Welcome();

 private:
  ReturnCode WriteToStream(std::string_view str);

  Stream& stream_;

  strings::CString<kMaxCommandLength> last_str_;
};

template <concepts::stream::ByteFullStreamConcept Stream>
constexpr Viewer<Stream>::Viewer(Stream& stream) : stream_(stream) {}

template <concepts::stream::ByteFullStreamConcept Stream>
ReturnCode Viewer<Stream>::Process(std::string_view str) {
  std::string_view writing_str;
  if (last_str_.GetLength() > str.size()) {
    constexpr std::string_view kDeleteLastStr = "\b \b";
    writing_str = kDeleteLastStr;
    last_str_.Pop(1);
  } else if (last_str_.GetLength() < str.size()) {
    writing_str = str.substr(str.size() - 1, 1);
    last_str_.Push(writing_str.data(), 1);
  } else {
    assert(str == last_str_);
  }
  return WriteToStream(writing_str);
}

template <concepts::stream::ByteFullStreamConcept Stream>
ReturnCode Viewer<Stream>::Enter() {
  return WriteToStream("\n\r");
}

template <concepts::stream::ByteFullStreamConcept Stream>
ReturnCode Viewer<Stream>::Welcome() {
  return WriteToStream(kWelcome);
}

template <concepts::stream::ByteFullStreamConcept Stream>
ReturnCode Viewer<Stream>::WriteToStream(std::string_view str) {
  auto result = write(stream_, str.data(), str.size());
  if (result == -1) {
    return ReturnCode::ERROR;
  }
  if (result < str.size()) {
    return ReturnCode::OVERFLOW;
  }
  assert(result == str.size() && "write failed");
  return ReturnCode::OK;
}
}  // namespace hydrolib::shell

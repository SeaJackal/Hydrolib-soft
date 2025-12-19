#pragma once

#include <streambuf>

#include "hydrolib_stream_concepts.hpp"

namespace hydrolib {
template <concepts::stream::ByteFullStreamConcept Stream>
class Streambuf : public std::streambuf {
 public:
  Streambuf(Stream &stream);

 public:
  int overflow(int c) override;
  int uflow() override;
  int underflow() override;

 private:
  Stream &stream_;

  int current_char_;
};

template <concepts::stream::ByteFullStreamConcept Stream>
Streambuf<Stream>::Streambuf(Stream &stream)
    : stream_(stream), current_char_(traits_type::eof()) {}

template <concepts::stream::ByteFullStreamConcept Stream>
int Streambuf<Stream>::overflow(int c) {
  if (c == traits_type::eof()) {
    return traits_type::eof();
  }

  char_type ch = static_cast<char_type>(c);
  int i = write(stream_, &ch, 1);
  return i == 1 ? c : traits_type::eof();
}

template <concepts::stream::ByteFullStreamConcept Stream>
int Streambuf<Stream>::uflow() {
  int c = underflow();
  current_char_ = traits_type::eof();
  return c;
}

template <concepts::stream::ByteFullStreamConcept Stream>
int Streambuf<Stream>::underflow() {
  if (current_char_ == traits_type::eof()) {
    char_type ch;
    int i = read(stream_, &ch, 1);
    current_char_ = i == 1 ? static_cast<int>(ch) : traits_type::eof();
  }
  return current_char_;
}
}  // namespace hydrolib
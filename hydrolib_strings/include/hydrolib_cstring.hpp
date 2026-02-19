#pragma once

#include <algorithm>
#include <cstring>

namespace hydrolib::strings {
template <unsigned CAPACITY>
class CString {
 public:
  constexpr CString(std::string_view str);
  constexpr CString(const char *str);
  constexpr CString(const char *str, int length);
  constexpr CString() = default;

  [[nodiscard]] constexpr int GetLength() const;

  int Write(const void *data, int length);

  constexpr operator const char *() const;
  constexpr operator char *();
  constexpr char &operator[](int index);

 private:
  std::array<char, CAPACITY> string_ = {};
  unsigned length_ = 0;
};

template <unsigned CAPACITY>
int write(CString<CAPACITY> &str, const void *data, unsigned length);

template <unsigned CAPACITY>
constexpr CString<CAPACITY>::CString(std::string_view str)
    : length_(str.size()) {
  std::copy(str.begin(), str.end(), string_.begin());
}

template <unsigned CAPACITY>
constexpr CString<CAPACITY>::CString(const char *str)
    : CString(str, std::strlen(str)) {}

template <unsigned CAPACITY>
constexpr CString<CAPACITY>::CString(const char *str, int length)
    : length_(length) {
  std::copy(str, str + length_, string_.begin());
}

template <unsigned CAPACITY>
constexpr int CString<CAPACITY>::GetLength() const {
  return length_;
}

template <unsigned CAPACITY>
int CString<CAPACITY>::Write(const void *data, int length) {
  int writing_length = length;
  if (CAPACITY - length_ < length) [[unlikely]] {
    writing_length = CAPACITY - length_;
  }
  memcpy(string_.data() + length_, data, writing_length);
  length_ += writing_length;
  return writing_length;
}

template <unsigned CAPACITY>
constexpr CString<CAPACITY>::operator const char *() const {
  return string_.data();
}

template <unsigned CAPACITY>
constexpr CString<CAPACITY>::operator char *() {
  return string_.data();
}

template <unsigned CAPACITY>
constexpr char &CString<CAPACITY>::operator[](int index) {
  return string_[index];
}

template <unsigned CAPACITY>
int write(CString<CAPACITY> &str, const void *data, unsigned length) {
  return str.Write(data, length);
}

}  // namespace hydrolib::strings

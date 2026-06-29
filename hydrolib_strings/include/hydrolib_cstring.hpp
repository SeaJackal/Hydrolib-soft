#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <string_view>

namespace hydrolib::strings {
template <int kCapacity>
class CString {
 public:
  constexpr CString(std::string_view str);  // NOLINT
  constexpr CString(const char* str);       // NOLINT
  constexpr CString(const char* str, int length);
  constexpr CString() = default;

  [[nodiscard]] constexpr int GetLength() const;

  int Push(const void* data, int length);
  void Pop(int length);

  constexpr operator std::string_view();              // NOLINT
  constexpr operator const std::string_view() const;  // NOLINT
  constexpr operator const char*() const;             // NOLINT
  constexpr operator char*();                         // NOLINT
  constexpr char& operator[](int index);

 private:
  std::array<char, kCapacity> string_ = {};
  int length_ = 0;
};

template <int kCapacity>
int write(CString<kCapacity>& str, const void* data, int length);  // NOLINT

template <int kCapacity>
constexpr CString<kCapacity>::CString(std::string_view str)
    : length_(static_cast<int>(str.size())) {
  std::copy(str.begin(), str.end(), string_.begin());
}

template <int kCapacity>
constexpr CString<kCapacity>::CString(const char* str)
    : CString(str, std::strlen(str)) {}

template <int kCapacity>
constexpr CString<kCapacity>::CString(const char* str, int length)
    : length_(length) {
  std::copy(str, str + length_, string_.begin());
}

template <int kCapacity>
constexpr int CString<kCapacity>::GetLength() const {
  return length_;
}

template <int kCapacity>
int CString<kCapacity>::Push(const void* data, int length) {
  int writing_length = length;
  if (kCapacity - length_ < length) [[unlikely]] {
    writing_length = kCapacity - length_;
  }
  memcpy(string_.data() + length_, data, writing_length);
  length_ += writing_length;
  return writing_length;
}

template <int kCapacity>
void CString<kCapacity>::Pop(int length) {
  length_ -= length;
  assert(length_ >= 0);
}

template <int kCapacity>
constexpr CString<kCapacity>::operator const char*() const {
  return string_.data();
}

template <int kCapacity>
constexpr CString<kCapacity>::operator char*() {
  return string_.data();
}

template <int kCapacity>
constexpr char& CString<kCapacity>::operator[](int index) {
  return string_[index];
}

template <int kCapacity>
constexpr CString<kCapacity>::operator std::string_view() {
  return std::string_view(string_.data(), length_);
}

template <int kCapacity>
constexpr CString<kCapacity>::operator const std::string_view() const {
  return std::string_view(string_.data(), length_);
}

template <int kCapacity>
int write(CString<kCapacity>& str, const void* data, int length) {
  return str.Push(data, length);
}

}  // namespace hydrolib::strings

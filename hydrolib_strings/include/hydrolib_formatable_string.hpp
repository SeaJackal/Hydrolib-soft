#pragma once

#include <array>
#include <cassert>
#include <charconv>
#include <concepts>
#include <cstring>
#include <string_view>

#include "hydrolib_return_codes.hpp"
#include "hydrolib_stream_concepts.hpp"

namespace hydrolib::strings {
template <typename T>
concept StringConsept = requires(T string) {
  { static_cast<const char *>(string) };
  { string.GetLength() } -> std::convertible_to<std::size_t>;
};

template <typename T, typename DestType>
concept ConvertibleToBytesConcept =
    concepts::stream::ByteWritableStreamConcept<DestType> &&
    requires(T value, DestType &buffer) {
      { value.ToBytes(buffer) } -> std::convertible_to<ReturnCode>;
    };

template <typename... ArgTypes>
class StaticFormatableString {
 public:
  static constexpr unsigned MAX_PARAMETERS_COUNT = 10;

  constexpr StaticFormatableString() = default;
  consteval StaticFormatableString(const char *string);

  [[nodiscard]] unsigned GetLength() const;

  template <concepts::stream::ByteWritableStreamConcept DestType>
  ReturnCode ToBytes(DestType &buffer, ArgTypes... params) const;

  constexpr operator const char *() const;

 private:
  template <concepts::stream::ByteWritableStreamConcept DestType,
            StringConsept String, typename... Ts>
  ReturnCode ToBytes_(DestType &buffer, int next_param_index,
                      int translated_length, String param, Ts... others) const;
  template <concepts::stream::ByteWritableStreamConcept DestType, typename T,
            typename... Ts>
  ReturnCode ToBytes_(DestType &buffer, int next_param_index,
                      int translated_length, T param, Ts... others) const
    requires ConvertibleToBytesConcept<T, DestType>;
  template <concepts::stream::ByteWritableStreamConcept DestType,
            typename... Ts>
  ReturnCode ToBytes_(DestType &buffer, int next_param_index,
                      int translated_length, int param, Ts... others) const;
  template <concepts::stream::ByteWritableStreamConcept DestType>
  ReturnCode ToBytes_(DestType &buffer, int next_param_index,
                      int translated_length) const;

  std::string_view string_ = nullptr;
  int param_count_ = 0;
  std::array<int, MAX_PARAMETERS_COUNT> param_pos_diffs_ = {};
};

inline void Error(const char *message);

inline void Error([[maybe_unused]] const char *message) {
  int param = 0;
  [[maybe_unused]] int error = 1 / param;
};

template <typename... ArgTypes>
consteval StaticFormatableString<ArgTypes...>::StaticFormatableString(
    const char *string)
    : string_(string) {
  int last_param = 0;
  bool open_flag = false;
  for (int i = 0; i < static_cast<int>(string_.size()); i++) {
    if (string_[i] == '{') {
      if (open_flag) {
        Error("Unpaired parameter in formatable string");
      }
      open_flag = true;
      param_pos_diffs_[param_count_] = i - last_param;
      param_count_++;
    } else if (string_[i] == '}') {
      last_param = i + 1;
      open_flag = false;
    }
  }
  if (sizeof...(ArgTypes) != param_count_) {
    Error("Not enough arguments for inserting to formatable string");
  }
  if (open_flag) {
    Error("Unclosed parameter in formatable string");
  }
}

template <typename... ArgTypes>
template <concepts::stream::ByteWritableStreamConcept DestType>
ReturnCode StaticFormatableString<ArgTypes...>::ToBytes(
    DestType &buffer, ArgTypes... params) const {
  return ToBytes_(buffer, 0, 0, params...);
}

template <typename... ArgTypes>
constexpr StaticFormatableString<ArgTypes...>::operator const char *() const {
  return string_.data();
}

template <typename... ArgTypes>
template <concepts::stream::ByteWritableStreamConcept DestType, typename T,
          typename... Ts>
ReturnCode StaticFormatableString<ArgTypes...>::ToBytes_(DestType &buffer,
                                                         int next_param_index,
                                                         int translated_length,
                                                         T param,
                                                         Ts... others) const
  requires ConvertibleToBytesConcept<T, DestType>
{
  if (translated_length >= static_cast<int>(string_.size())) {
    return ReturnCode::OK;
  }

  int write_res = write(buffer, string_.data() + translated_length,
                        param_pos_diffs_[next_param_index]);
  if (write_res == -1) {
    return ReturnCode::ERROR;
  }
  if (write_res != param_pos_diffs_[next_param_index]) {
    return ReturnCode::OVERFLOW;
  }
  translated_length += param_pos_diffs_[next_param_index] + (sizeof("{}") - 1);
  next_param_index++;

  auto write_integer_result = param.ToBytes(buffer);
  if (write_integer_result != ReturnCode::OK) {
    return write_integer_result;
  }

  return ToBytes_(buffer, next_param_index, translated_length, others...);
}

template <typename... ArgTypes>
template <concepts::stream::ByteWritableStreamConcept DestType, typename... Ts>
ReturnCode StaticFormatableString<ArgTypes...>::ToBytes_(DestType &buffer,
                                                         int next_param_index,
                                                         int translated_length,
                                                         int param,
                                                         Ts... others) const {
  if (translated_length >= static_cast<int>(string_.size())) {
    return ReturnCode::OK;
  }

  int write_res = write(buffer, string_.data() + translated_length,
                        param_pos_diffs_[next_param_index]);
  if (write_res == -1) {
    return ReturnCode::ERROR;
  }
  if (write_res != param_pos_diffs_[next_param_index]) {
    return ReturnCode::OVERFLOW;
  }
  translated_length += param_pos_diffs_[next_param_index] + (sizeof("{}") - 1);
  next_param_index++;

  std::array<char, 12> integer_bytes = {};

  auto convertion_result = std::to_chars(
      integer_bytes.data(), integer_bytes.data() + integer_bytes.size(), param);
  auto writing_result = write(buffer, integer_bytes.data(),
                              convertion_result.ptr - integer_bytes.data());
  if (writing_result == -1) {
    return ReturnCode::ERROR;
  }
  if (writing_result != convertion_result.ptr - integer_bytes.data()) {
    return ReturnCode::OVERFLOW;
  }

  return ToBytes_(buffer, next_param_index, translated_length, others...);
}

template <typename... ArgTypes>
template <concepts::stream::ByteWritableStreamConcept DestType,
          StringConsept String, typename... Ts>
ReturnCode StaticFormatableString<ArgTypes...>::ToBytes_(DestType &buffer,
                                                         int next_param_index,
                                                         int translated_length,
                                                         String param,
                                                         Ts... others) const {
  if (translated_length >= static_cast<int>(string_.size())) {
    return ReturnCode::OK;
  }

  int write_res = write(buffer, string_.data() + translated_length,
                        param_pos_diffs_[next_param_index]);
  if (write_res == -1) {
    return ReturnCode::ERROR;
  }
  if (write_res != param_pos_diffs_[next_param_index]) {
    return ReturnCode::OVERFLOW;
  }
  translated_length += param_pos_diffs_[next_param_index] + 2;
  next_param_index++;

  write_res =
      write(buffer, static_cast<const char *>(param), param.GetLength());
  if (write_res == -1) {
    return ReturnCode::ERROR;
  }
  if (write_res != param.GetLength()) {
    return ReturnCode::OVERFLOW;
  }

  return ToBytes_(buffer, next_param_index, translated_length, others...);
}

template <typename... ArgTypes>
template <concepts::stream::ByteWritableStreamConcept DestType>
ReturnCode StaticFormatableString<ArgTypes...>::ToBytes_(
    DestType &buffer, int next_param_index, int translated_length) const {
  (void)next_param_index;
  int write_res = write(buffer, string_.data() + translated_length,
                        string_.size() - translated_length);
  if (write_res == -1) {
    return ReturnCode::ERROR;
  }
  if (write_res != static_cast<int>(string_.size() - translated_length)) {
    return ReturnCode::OVERFLOW;
  }
  return ReturnCode::OK;
}

template <typename... ArgTypes>
unsigned StaticFormatableString<ArgTypes...>::GetLength() const {
  return string_.size();
}
}  // namespace hydrolib::strings

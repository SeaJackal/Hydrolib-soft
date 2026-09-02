#pragma once

#include <string_view>
namespace hydrolib {
enum class ReturnCode { kOk = 0, kFail, kNoData, kOverflow, kError, kTimeout };

template <typename T>
class Expected {
 public:
  Expected(const T& value);   // NOLINT
  Expected(ReturnCode code);  // NOLINT
  Expected(const T& value, ReturnCode code);

  operator T();           // NOLINT
  operator ReturnCode();  // NOLINT
  operator bool();        // NOLINT

 private:
  ReturnCode code_;
  T value_;
};

constexpr void CompileTimeAssert(bool assertion,
                                 [[maybe_unused]] std::string_view message) {
  [[maybe_unused]] int error = 1 / static_cast<int>(assertion);
}

template <typename T>
Expected<T>::Expected(const T& value) : code_(ReturnCode::kOk), value_(value) {}

template <typename T>
Expected<T>::Expected(ReturnCode code) : code_(code), value_() {}

template <typename T>
Expected<T>::Expected(const T& value, ReturnCode code)
    : code_(code), value_(value) {}

template <typename T>
Expected<T>::operator T() {
  return value_;
}

template <typename T>
Expected<T>::operator ReturnCode() {
  return code_;
}

template <typename T>
Expected<T>::operator bool() {
  return code_ == ReturnCode::kOk;
}
}  // namespace hydrolib

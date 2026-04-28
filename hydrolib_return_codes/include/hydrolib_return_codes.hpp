#pragma once

namespace hydrolib {
enum class ReturnCode { OK = 0, FAIL, NO_DATA, OVERFLOW, ERROR, TIMEOUT };

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

template <typename T>
Expected<T>::Expected(const T& value) : code_(ReturnCode::OK), value_(value) {}

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
  return code_ == ReturnCode::OK;
}
}  // namespace hydrolib

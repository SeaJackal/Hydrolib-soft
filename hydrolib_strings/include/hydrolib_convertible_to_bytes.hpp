#pragma once

#include <concepts>

#include "hydrolib_return_codes.hpp"
#include "hydrolib_stream_concepts.hpp"

namespace hydrolib::strings {
template <typename T, typename DestType>
concept ConvertibleToBytesConcept =
    concepts::stream::ByteWritableStreamConcept<DestType> &&
    requires(T value, DestType &buffer) {
      { value.ToBytes(buffer) } -> std::convertible_to<ReturnCode>;
    };

template <typename DestType>
ReturnCode IntToBytes(int param, DestType &buffer) {
  if (param == 0) {
    auto write_res = write(buffer, "0", 1);
    if (write_res == -1) {
      return ReturnCode::ERROR;
    }
    if (write_res != 1) {
      return ReturnCode::OVERFLOW;
    }
  } else {
    if (param < 0) {
      auto write_res = write(buffer, "-", 1);
      if (write_res == -1) {
        return ReturnCode::ERROR;
      }
      if (write_res != 1) {
        return ReturnCode::OVERFLOW;
      }
      param = -param;
    }

    constexpr int kBase = 10;

    int digit = 1;
    while (digit <= param) {
      digit *= kBase;
    }
    digit /= kBase;

    char symbol = 0;

    while (digit != 0) {
      symbol = static_cast<char>(param / digit) + '0';
      param %= digit;
      auto write_res = write(buffer, &symbol, 1);
      if (write_res == -1) {
        return ReturnCode::ERROR;
      }
      if (write_res != 1) {
        return ReturnCode::OVERFLOW;
      }
      digit /= kBase;
    }
  }
  return ReturnCode::OK;
}
}  // namespace hydrolib::strings

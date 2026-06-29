#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "hydrolib_return_codes.hpp"

namespace hydrolib::cobs {
template <std::byte kMagicByteValue>
[[nodiscard]] int Encode(std::span<std::byte> data);
template <std::byte kMagicByteValue>
ReturnCode Decode(int first_encoded_byte, std::span<std::byte> data);

template <std::byte kMagicByteValue>
int Encode(std::span<std::byte> data) {
  int result = UINT8_MAX;
  int last_appearance = 0;
  for (int i = 0; i < static_cast<int>(data.size()); i++) {
    if (data[i] == kMagicByteValue) {
      last_appearance = i;
      result = i;
      break;
    }
  }
  if(result == UINT8_MAX) {
    return UINT8_MAX;
  }
  for (int i = last_appearance + 1; i < static_cast<int>(data.size()); i++) {
    if (data[i] == kMagicByteValue) {
      data[last_appearance] = static_cast<std::byte>(i - last_appearance);
      last_appearance = i;
    }
  }
  data[last_appearance] = std::byte(0);
  return result;
}

template <std::byte kMagicByteValue>
ReturnCode Decode(int first_encoded_byte, std::span<std::byte> data) {
  if (first_encoded_byte == UINT8_MAX) {
    return ReturnCode::OK;
  }
  if (first_encoded_byte >= static_cast<int>(data.size())) {
    return ReturnCode::ERROR;
  }
  int current_appearance = first_encoded_byte;
  while (data[current_appearance] != std::byte(0)) {
    int next_appearance =
        current_appearance + static_cast<int>(data[current_appearance]);
    data[current_appearance] = kMagicByteValue;
    current_appearance = next_appearance;
    if (current_appearance >= static_cast<int>(data.size())) {
      return ReturnCode::ERROR;
    }
  }
  data[current_appearance] = kMagicByteValue;
  return ReturnCode::OK;
}
};  // namespace hydrolib::cobs

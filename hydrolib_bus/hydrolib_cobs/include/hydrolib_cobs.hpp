#pragma once

#include <cstddef>
#include <span>

#include "hydrolib_return_codes.hpp"

namespace hydrolib::cobs {
template <std::byte kMagicByteValue>
void Encode(std::span<std::byte> data);
template <std::byte kMagicByteValue>
ReturnCode Decode(std::span<std::byte> data);

template <std::byte kMagicByteValue>
void Encode(std::span<std::byte> data) {
  int last_appearance = 0;
  for (int i = 1; i < static_cast<int>(data.size()); i++) {
    if (data[i] == kMagicByteValue) {
      data[last_appearance] = static_cast<std::byte>(i - last_appearance);
      last_appearance = i;
    }
  }
  data[last_appearance] = std::byte(0);
}

template <std::byte kMagicByteValue>
ReturnCode Decode(std::span<std::byte> data) {
  int current_appearance = 0;
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
  data[0] = std::byte(0);
  return ReturnCode::OK;
}
};  // namespace hydrolib::cobs

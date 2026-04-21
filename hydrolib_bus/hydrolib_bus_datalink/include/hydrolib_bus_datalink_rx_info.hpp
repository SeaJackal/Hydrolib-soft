#pragma once

#include <array>
#include <span>

#include "hydrolib_bus_datalink_message.hpp"

namespace hydrolib::bus::datalink {

class MessageData {
 public:
  MessageData() = default;
  explicit MessageData(int length);

  operator std::span<std::byte>();
  operator std::span<const std::byte>() const;

 private:
  int length_ = 0;
  std::array<std::byte, kMaxDataLength>
      data_{};  // TODO(vscode): make allocation
};

struct MessageInfo {
  AddressType src_address = std::byte(0);
  MessageData data;
};

inline MessageData::MessageData(int length) : length_(length) {}

inline MessageData::operator std::span<std::byte>() {
  return std::span(data_).subspan(0, length_);
}

inline MessageData::operator std::span<const std::byte>() const {
  return std::span(data_).subspan(0, length_);
}

}  // namespace hydrolib::bus::datalink

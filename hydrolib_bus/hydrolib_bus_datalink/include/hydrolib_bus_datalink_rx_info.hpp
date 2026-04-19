#pragma once

#include <array>
#include <span>

#include "hydrolib_bus_datalink_message.hpp"

namespace hydrolib::bus::datalink {
class RxInfo {
 public:
  RxInfo() = default;
  RxInfo(AddressType src_address, int length);

  [[nodiscard]] std::span<std::byte> GetData();
  [[nodiscard]] AddressType GetSrcAddress() const;

 private:
  AddressType src_address_ = std::byte(0);
  int length_ = 0;
  std::array<std::byte, kMaxDataLength>
      data_{};  // TODO(vscode): make allocation
};

inline RxInfo::RxInfo(AddressType src_address, int length)
    : src_address_(src_address), length_(length) {}

inline std::span<std::byte> RxInfo::GetData() {
  return std::span(data_).subspan(0, length_);
}

inline AddressType RxInfo::GetSrcAddress() const { return src_address_; }
}  // namespace hydrolib::bus::datalink

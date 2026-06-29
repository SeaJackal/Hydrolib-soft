#pragma once

#include <climits>
#include <cstddef>
#include <cstdint>
#include <span>

namespace hydrolib::crc {
class CRC8 {
 public:
  static constexpr uint16_t kPolynomial = 0x0700;

  void Next(std::byte byte);
  void Next(std::span<const std::byte> data);
  [[nodiscard]] std::byte Get();

 private:
  uint16_t crc_ = 0;
};

inline void CRC8::Next(std::byte byte) {
  crc_ |= static_cast<uint8_t>(byte);
  for (int i = 0; i < CHAR_BIT; i++) {
    constexpr int kMaskForMostSignificantBit =
        1 << ((CHAR_BIT * sizeof(uint16_t)) - 1);
    if ((crc_ & kMaskForMostSignificantBit) != 0) {
      crc_ = (crc_ << 1) ^ kPolynomial;
    } else {
      crc_ = crc_ << 1;
    }
  }
}

inline void CRC8::Next(std::span<const std::byte> data) {
  for (auto byte : data) {
    Next(byte);
  }
}

inline std::byte CRC8::Get() {
  Next(std::byte(0));
  return static_cast<std::byte>(crc_ >> CHAR_BIT);
}
}  // namespace hydrolib::crc

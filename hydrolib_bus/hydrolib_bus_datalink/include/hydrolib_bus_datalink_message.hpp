#pragma once

#include <cstddef>
#include <cstdint>

namespace hydrolib::bus::datalink {

using AddressType = std::byte;

struct MessageHeader {
  AddressType dest_address;
  AddressType src_address;
  uint8_t length;
  uint8_t cobs_length;
} __attribute__((__packed__));

constexpr std::byte kMagicByte = std::byte(0xAA);
constexpr int kCRCLength = 1;
constexpr int kMaxMessageLength = UINT8_MAX;
constexpr int kMaxDataLength =
    kMaxMessageLength - sizeof(MessageHeader) - sizeof(kMagicByte) - kCRCLength;

struct MessageBuffer {
  std::byte magic_byte;
  MessageHeader header;
  std::byte data_and_crc[kMaxDataLength + kCRCLength];  // NOLINT
} __attribute__((__packed__));

}  // namespace hydrolib::bus::datalink
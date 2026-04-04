#pragma once

#include <cstddef>
#include <cstdint>

namespace hydrolib::bus::datalink {

using AddressType = std::byte;

struct MessageHeader {
  std::byte magic_byte;
  AddressType dest_address;
  AddressType src_address;
  uint8_t length;
  uint8_t cobs_length;
} __attribute__((__packed__));

constexpr std::byte kMagicByte = std::byte(0xAA);
constexpr int kCRCLength = 1;
constexpr int kMaxMessageLength = UINT8_MAX;
constexpr int kMaxDataLength =
    kMaxMessageLength - sizeof(MessageHeader) - kCRCLength;

struct MessageBuffer {
  MessageHeader header;
  std::byte data_and_crc[kMaxDataLength + kCRCLength];  // NOLINT
} __attribute__((__packed__));

}  // namespace hydrolib::bus::datalink
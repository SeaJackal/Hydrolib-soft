#pragma once

#include <cstddef>
#include <cstdint>

namespace hydrolib::bus::application {
enum class Command : uint8_t { kWrite, kRead, kResponse, kError };

struct MemoryAccessInfo {
  uint8_t address;
  uint8_t length;
} __attribute__((__packed__));

struct MemoryAccessHeader {
  Command command;
  MemoryAccessInfo info;
} __attribute__((__packed__));

constexpr unsigned kMaxDataLength = UINT8_MAX;
constexpr unsigned kMaxMessageLength =
    sizeof(MemoryAccessHeader) + kMaxDataLength;

struct MemoryAccessMessageBuffer {
  MemoryAccessHeader header;
  std::byte data[kMaxDataLength];  // NOLINT
} __attribute__((__packed__));

struct ResponseMessageBuffer {
  Command command;
  std::byte data[kMaxDataLength];  // NOLINT
} __attribute__((__packed__));

}  // namespace hydrolib::bus::application

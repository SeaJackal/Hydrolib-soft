#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <span>

namespace hydrolib::bus::datalink {

template <int kChunkSize, int kMaxChunks>
class DummyAllocator {
 public:
  static std::byte* allocate(int size);              // NOLINT
  static void deallocate(std::byte* ptr, int size);  // NOLINT

 private:
  static inline std::array<std::byte,
                           static_cast<size_t>(kChunkSize* kMaxChunks)>
      buffer{};
  static inline std::array<bool, kMaxChunks> occupied_chunks{};
};

template <int kChunkSize, int kMaxChunks>
std::byte* DummyAllocator<kChunkSize, kMaxChunks>::allocate(
    [[maybe_unused]] int size) {
  for (int i = 0; i < kMaxChunks; i++) {
    if (!occupied_chunks[i]) {
      occupied_chunks[i] = true;
      return std::span(buffer).subspan(i * kChunkSize, size).data();
    }
  }
  assert(false);
}

template <int kChunkSize, int kMaxChunks>
void DummyAllocator<kChunkSize, kMaxChunks>::deallocate(
    std::byte* ptr, [[maybe_unused]] int size) {
  if (ptr == nullptr) {
    return;
  }
  assert(ptr >= buffer.data() && ptr < buffer.data() + buffer.size()); // NOLINT
  assert(size <= kChunkSize);
  assert((ptr - buffer.data()) % kChunkSize == 0); // NOLINT
  int index = (ptr - buffer.data()) / kChunkSize;
  assert(occupied_chunks[index]);
  occupied_chunks[index] = false;
}

}  // namespace hydrolib::bus::datalink

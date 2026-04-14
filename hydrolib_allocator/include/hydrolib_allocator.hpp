#pragma once

#include <cstddef>
#include <span>

namespace hydrolib::allocator {
template <typename T, int kChunkSize, int kChunksNumber>
class Allocator {
  template <typename U, int kOtherChunkSize, int kOtherChunksNumber>
  friend class Allocator;

 public:
  template <class U>
  struct rebind {
    using other = Allocator<U, kChunkSize, kChunksNumber>;
  };

  using value_type = T;

  explicit Allocator(std::span<std::byte> buffer);

  template <typename U>
  Allocator(const Allocator<U, kChunkSize, kChunksNumber>& other);

  [[nodiscard]] T* allocate(int number);
  void deallocate(T* pointer, int number);

 private:
  std::span<std::byte> buffer_;
};

template <typename T, typename U, int kChunkSize, int kChunksNumber>
[[nodiscard]] bool operator==(
    const Allocator<T, kChunkSize, kChunksNumber>& left,
    const Allocator<U, kChunkSize, kChunksNumber>& right) noexcept {
  return left.buffer_ == right.buffer_;
}

template <typename T, typename U, int kChunkSize, int kChunksNumber>
[[nodiscard]] bool operator!=(
    const Allocator<T, kChunkSize, kChunksNumber>& left,
    const Allocator<U, kChunkSize, kChunksNumber>& right) noexcept {
  return !(left == right);
}

template <typename T, int kChunkSize, int kChunksNumber>
Allocator<T, kChunkSize, kChunksNumber>::Allocator(std::span<std::byte> buffer)
    : buffer_(buffer) {}

template <typename T, int kChunkSize, int kChunksNumber>
template <typename U>
Allocator<T, kChunkSize, kChunksNumber>::Allocator(
    const Allocator<U, kChunkSize, kChunksNumber>& other)
    : buffer_(other.buffer_) {}

template <typename T, int kChunkSize, int kChunksNumber>
T* Allocator<T, kChunkSize, kChunksNumber>::allocate(int number) {
  if (number * sizeof(T) > kChunkSize) [[unlikely]] {
    return nullptr;
  }
  for (int i = 0; i < kChunksNumber; ++i) {
    if (buffer_[i * kChunkSize] == std::byte(0)) {
      buffer_[i * kChunkSize] = std::byte(1);
      return reinterpret_cast<T*>(
          buffer_.subspan((i * kChunkSize) + 1, kChunkSize - 1).data());
    }
  }
  return nullptr;
}

template <typename T, int kChunkSize, int kChunksNumber>
void Allocator<T, kChunkSize, kChunksNumber>::deallocate(
    T* pointer, [[maybe_unused]] int number) {
  auto* raw_pointer = reinterpret_cast<std::byte*>(pointer);
  raw_pointer[-1] = std::byte(0);
}

}  // namespace hydrolib::allocator

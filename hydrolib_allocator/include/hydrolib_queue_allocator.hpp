#pragma once

#include <array>
#include <cstddef>

namespace hydrolib::allocator {
template <typename T, int kBufferSize>
class QueueAllocator {
 public:
  using value_type = T;

  QueueAllocator() = default;

  [[nodiscard]] T* allocate(int number);
  void deallocate(T* pointer, int number);
  [[nodiscard]] int GetFreeSize();

 private:
  std::array<std::byte, kBufferSize + 1> buffer_{};

  int head_ = 0;
  int tail_ = 0;
};

template <typename T, int kBufferSize>
T* QueueAllocator<T, kBufferSize>::allocate(int number) {
  if (number * sizeof(T) > GetFreeSize()) [[unlikely]] {
    return nullptr;
  }
  if (head_ > tail_) {
    auto result = &buffer_[tail_];
    tail_ += sizeof(T) * number;
    return result;
  }
  if (buffer_.size() - tail_ >= number) {
    auto result = &buffer_[tail_];
    tail_ += sizeof(T) * number;
    if (tail_ == buffer_.size()) {
      tail_ = 0;
    }
    return result;
  }
  tail_ = sizeof(T) * number;
  return buffer_.data();
}

template <typename T, int kBufferSize>
void QueueAllocator<T, kBufferSize>::deallocate(
    [[maybe_unused]] T* pointer, int number) {
  head_ = (head_ + number) % buffer_.size();
}

template <typename T, int kBufferSize>
int QueueAllocator<T, kBufferSize>::GetFreeSize() {
  unsigned head = head_;
  if (tail_ >= head) {
    return kBufferSize - tail_ + head;
  }
  return head - tail_ - 1;
}

}  // namespace hydrolib::allocator

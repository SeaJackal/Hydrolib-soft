#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <deque>
#include <vector>

#include "hydrolib_allocator.hpp"

TEST(AllocatorByte, AllocateCheck) {
  static constexpr int kBufferSize = 64;
  std::array<std::byte, kBufferSize * kBufferSize> buffer;
  hydrolib::allocator::Allocator<std::byte, kBufferSize, kBufferSize> alloc{
      buffer};
  int buffer_size = kBufferSize;

  auto length_former = [buffer_size]([[maybe_unused]] int itteration) -> int {
    constexpr int kNumberOfIterrations = 10;
    if (itteration >= kNumberOfIterrations) {
      return 0;
    }
    return buffer_size / kNumberOfIterrations;
  };

  auto* pointer = alloc.allocate(2);
  alloc.deallocate(pointer, 2);

  std::vector<std::tuple<int, std::byte*>> allocated_buffs;
  for (int i = 0; i != kBufferSize; ++i) {
    int length = length_former(i);
    if (length == 0) {
      break;
    }
    auto* new_buff_start = alloc.allocate(length);
    ASSERT_NE(new_buff_start, nullptr);
    auto* new_buff_end = new_buff_start + length - 1;
    for (auto [buff_length, buff_start] : allocated_buffs) {
      EXPECT_TRUE(buff_start > new_buff_start ||
                  new_buff_start >= buff_start + buff_length);
      EXPECT_TRUE(buff_start > new_buff_end ||
                  new_buff_end >= buff_start + buff_length);
    }
    allocated_buffs.emplace_back(length, new_buff_start);
  }
  for (auto [buff_length, buff_start] : allocated_buffs) {
    alloc.deallocate(buff_start, buff_length);
  }
  EXPECT_NE(alloc.allocate(kBufferSize), nullptr);
}

TEST(AllocatorByte, QueueCheck) {
  static constexpr int kBufferSize = 64;
  std::array<std::byte, sizeof(int64_t) * kBufferSize> buffer;
  hydrolib::allocator::Allocator<int64_t, sizeof(int64_t), kBufferSize> alloc{
      buffer};
  std::deque<int64_t, decltype(alloc)> queue{alloc};
}

#include <gtest/gtest.h>

#include <cstddef>
#include <vector>

#include "hydrolib_queue_allocator.hpp"

TEST(QueueAllocatorByte, FreeSizeStartsAtBufferSize) {
  static constexpr int kBufferSize = 64;
  hydrolib::allocator::QueueAllocator<std::byte, kBufferSize> alloc;
  EXPECT_EQ(alloc.GetFreeSize(), kBufferSize);
}

TEST(QueueAllocatorByte, FreeSizeChanges) {
  static constexpr int kBufferSize = 64;
  int allocated_size = 16;
  hydrolib::allocator::QueueAllocator<std::byte, kBufferSize> alloc;
  std::byte* pointer = alloc.allocate(allocated_size);
  EXPECT_EQ(alloc.GetFreeSize(), kBufferSize - allocated_size);
  alloc.deallocate(pointer, allocated_size);
  EXPECT_EQ(alloc.GetFreeSize(), kBufferSize);
}

TEST(QueueAllocatorByte, CantAllocateMoreThenBuffer) {
  static constexpr int kBufferSize = 64;
  hydrolib::allocator::QueueAllocator<std::byte, kBufferSize> alloc;
  std::byte* pointer = alloc.allocate(kBufferSize + 1);
  EXPECT_EQ(pointer, nullptr);
}

TEST(QueueAllocatorByte, AllocateCheck) {
  static constexpr int kBufferSize = 64;
  hydrolib::allocator::QueueAllocator<std::byte, kBufferSize> alloc;
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

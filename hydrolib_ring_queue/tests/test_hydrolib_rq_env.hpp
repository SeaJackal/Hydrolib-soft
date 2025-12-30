#pragma once

#include <gtest/gtest.h>

#include <array>

#include "hydrolib_ring_queue.hpp"

class TestHydrolibRingQueue : public ::testing::Test {
 public:
  static constexpr int kDefaultCapacity = 16;

 protected:
  TestHydrolibRingQueue();

  hydrolib::ring_queue::RingQueue<kDefaultCapacity> test_queue = {};  // NOLINT
  std::array<uint8_t, kDefaultCapacity> data = {};                  // NOLINT
};

inline TestHydrolibRingQueue::TestHydrolibRingQueue() {
  for (int i = 0; i < kDefaultCapacity; i++) {
    data[i] = i;
  }
}

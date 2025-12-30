#pragma once

#include <gtest/gtest.h>

#include "hydrolib_ring_queue.hpp"

class TestHydrolibRingQueue : public ::testing::Test {
 public:
  static constexpr int kDefaultCapacity = 16;

 protected:
  TestHydrolibRingQueue() = default;

  hydrolib::ring_queue::RingQueue<kDefaultCapacity> test_queue = {};
};

class TestHydrolibRingQueueCommon
    : public TestHydrolibRingQueue,
      public ::testing::WithParamInterface<uint16_t> {};

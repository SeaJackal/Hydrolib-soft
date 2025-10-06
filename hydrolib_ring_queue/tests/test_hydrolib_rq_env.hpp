#ifndef TEST_HYDROLIB_RQ_ENV_H_
#define TEST_HYDROLIB_RQ_ENV_H_

#include "hydrolib_ring_queue.hpp"

#include <gtest/gtest.h>

#define DEFAULT_CAPACITY 16

class TestHydrolibRingQueue : public ::testing::Test
{
protected:
    TestHydrolibRingQueue()
        : buffer_capacity(DEFAULT_CAPACITY), test_queue(buffer, buffer_capacity)
    {
    }

    uint8_t buffer_capacity;
    uint8_t buffer[DEFAULT_CAPACITY] = {};
    hydrolib::ring_queue::RingQueue test_queue;

    ~TestHydrolibRingQueue() {}
};

class TestHydrolibRingQueueCommon
    : public TestHydrolibRingQueue,
      public ::testing::WithParamInterface<uint16_t>
{
};

#endif

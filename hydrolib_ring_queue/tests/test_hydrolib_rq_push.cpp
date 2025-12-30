#include <gtest/gtest.h>

#include <cstddef>

#include "test_hydrolib_rq_env.hpp"

class TestHydrolibRingQueuePush : public TestHydrolibRingQueue,
                                  public ::testing::WithParamInterface<int> {};

INSTANTIATE_TEST_CASE_P(
    Test, TestHydrolibRingQueuePush,
    ::testing::Values(1, sizeof(TestHydrolibRingQueue::kDefaultCapacity), 2,
                      7));

TEST_P(TestHydrolibRingQueuePush, Push) {
  int length = GetParam();
  ASSERT_LE(length, TestHydrolibRingQueue::kDefaultCapacity);

  hydrolib::ReturnCode push_status = test_queue.Push(data.data(), length);
  EXPECT_EQ(length, test_queue.GetLength());
  EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);

  for (int i = 0; i < length; i++) {
    EXPECT_EQ(i, test_queue[i]);
  }
}

TEST_P(TestHydrolibRingQueuePush, PushShifted) {
  int length = GetParam();

  ASSERT_LE(length, TestHydrolibRingQueue::kDefaultCapacity);

  for (int i = 0; i < kDefaultCapacity / 2; i++) {
    hydrolib::ReturnCode filling_status = test_queue.PushByte(0);
    EXPECT_EQ(filling_status, hydrolib::ReturnCode::OK);
  }

  test_queue.Drop(kDefaultCapacity / 2);

  hydrolib::ReturnCode push_status = test_queue.Push(data.data(), length);
  EXPECT_EQ(length, test_queue.GetLength());
  EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);

  for (int i = 0; i < length; i++) {
    EXPECT_EQ(i, test_queue[i]);
  }
}

TEST_F(TestHydrolibRingQueue, PushOverSimple) {
  hydrolib::ReturnCode push_status =
      test_queue.Push(data.data(), kDefaultCapacity + 1);
  EXPECT_EQ(push_status, hydrolib::ReturnCode::FAIL);
}

TEST_F(TestHydrolibRingQueuePush, PushOverComplex) {
  constexpr int kStartLength = kDefaultCapacity / 2;
  for (int i = 0; i < kStartLength; i++) {
    hydrolib::ReturnCode filling_status = test_queue.PushByte(data[i]);
    EXPECT_EQ(filling_status, hydrolib::ReturnCode::OK);
  }

  hydrolib::ReturnCode push_status =
      test_queue.Push(data.data(), kDefaultCapacity - kStartLength + 1);
  EXPECT_EQ(push_status, hydrolib::ReturnCode::FAIL);

  for (int i = 0; i < kStartLength; i++) {
    EXPECT_EQ(i, test_queue[i]);
  }
}

TEST_P(TestHydrolibRingQueuePush, PushToLimit) {
  uint16_t length = GetParam();

  for (int i = length; i <= kDefaultCapacity; i += length) {
    hydrolib::ReturnCode push_status =
        test_queue.Push(data.data() + i - length, length);
    EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);
  }

  for (int i = 0; i < kDefaultCapacity / length * length; i++) {
    EXPECT_EQ(i, test_queue[i]);
  }
}

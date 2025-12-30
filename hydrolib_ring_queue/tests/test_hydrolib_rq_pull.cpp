#include <gtest/gtest.h>

#include <array>
#include <cstddef>

#include "test_hydrolib_rq_env.hpp"

class TestHydrolibRingQueuePull : public TestHydrolibRingQueue,
                                  public ::testing::WithParamInterface<int> {};

INSTANTIATE_TEST_CASE_P(
    Test, TestHydrolibRingQueuePull,
    ::testing::Values(1, TestHydrolibRingQueue::kDefaultCapacity,
                      TestHydrolibRingQueue::kDefaultCapacity - 1, 2, 7));

TEST_P(TestHydrolibRingQueuePull, Pull) {
  int length = GetParam();
  ASSERT_LE(length, kDefaultCapacity);

  hydrolib::ReturnCode push_status =
      test_queue.Push(data.data(), kDefaultCapacity);
  EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);

  std::array<uint8_t, kDefaultCapacity> out{};
  hydrolib::ReturnCode pull_status = test_queue.Pull(out.data(), length);
  EXPECT_EQ(pull_status, hydrolib::ReturnCode::OK);
  EXPECT_EQ(kDefaultCapacity - length, test_queue.GetLength());

  for (int i = 0; i < length; i++) {
    EXPECT_EQ(i, out[i]);
  }
}

TEST_P(TestHydrolibRingQueuePull, PullShifted) {
  int length = GetParam();
  ASSERT_LE(length, TestHydrolibRingQueue::kDefaultCapacity);

  for (int i = 0; i < kDefaultCapacity / 2; i++) {
    hydrolib::ReturnCode filling_status = test_queue.PushByte(0);
    EXPECT_EQ(filling_status, hydrolib::ReturnCode::OK);
  }

  test_queue.Drop(kDefaultCapacity / 2);

  hydrolib::ReturnCode push_status =
      test_queue.Push(data.data(), kDefaultCapacity);
  EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);

  std::array<uint8_t, kDefaultCapacity> out{};
  hydrolib::ReturnCode pull_status = test_queue.Pull(out.data(), length);
  EXPECT_EQ(pull_status, hydrolib::ReturnCode::OK);
  EXPECT_EQ(kDefaultCapacity - length, test_queue.GetLength());

  for (int i = 0; i < length; i++) {
    EXPECT_EQ(data[i], out[i]);
  }
}

TEST_P(TestHydrolibRingQueuePull, PullOverSimple) {
  int length = GetParam();
  ASSERT_LE(length, TestHydrolibRingQueue::kDefaultCapacity);

  hydrolib::ReturnCode push_status = test_queue.Push(data.data(), length);
  EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);

  std::array<uint8_t, kDefaultCapacity> out{};
  hydrolib::ReturnCode pull_status = test_queue.Pull(out.data(), length + 1);
  EXPECT_EQ(pull_status, hydrolib::ReturnCode::FAIL);
  EXPECT_EQ(length, test_queue.GetLength());
}

TEST_F(TestHydrolibRingQueue, PullEmpty) {
  std::array<uint8_t, kDefaultCapacity> out{};
  hydrolib::ReturnCode pull_status = test_queue.Pull(out.data(), 1);
  EXPECT_EQ(pull_status, hydrolib::ReturnCode::FAIL);
  EXPECT_EQ(0, test_queue.GetLength());
}

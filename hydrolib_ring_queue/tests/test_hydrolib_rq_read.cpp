#include "hydrolib_return_codes.hpp"
#include "test_hydrolib_rq_env.hpp"

struct TestCase {
  int shift;
  int length;
};

class TestHydrolibRingQueueRead
    : public TestHydrolibRingQueue,
      public ::testing::WithParamInterface<TestCase> {
 protected:
  TestHydrolibRingQueueRead();
};

TestHydrolibRingQueueRead::TestHydrolibRingQueueRead() {
  for (int i = 0; i < kDefaultCapacity; i++) {
    hydrolib::ReturnCode filling_status = test_queue.PushByte(i);
    EXPECT_EQ(filling_status, hydrolib::ReturnCode::OK);
  }
}

INSTANTIATE_TEST_CASE_P(
    Test, TestHydrolibRingQueueRead,
    ::testing::Values(TestCase{0, 1},
                      TestCase{0, TestHydrolibRingQueue::kDefaultCapacity},
                      TestCase{1, TestHydrolibRingQueue::kDefaultCapacity - 1},
                      TestCase{TestHydrolibRingQueue::kDefaultCapacity - 1, 1},
                      TestCase{TestHydrolibRingQueue::kDefaultCapacity, 1},
                      TestCase{0, TestHydrolibRingQueue::kDefaultCapacity + 1},
                      TestCase{2, 3}, TestCase{1, 7}, TestCase{3, 8}));

TEST_P(TestHydrolibRingQueueRead, Read) {
  auto param = GetParam();
  int shift = param.shift;
  int length = param.length;

  std::array<uint8_t, kDefaultCapacity> data{};
  hydrolib::ReturnCode read_status =
      test_queue.Read(data.data(), length, shift);
  if (length + shift > kDefaultCapacity) {
    EXPECT_EQ(read_status, hydrolib::ReturnCode::FAIL);
  } else {
    EXPECT_EQ(read_status, hydrolib::ReturnCode::OK);

    for (int i = 0; i < length; i++) {
      EXPECT_EQ(shift + i, data[i]);
    }
  }
}

TEST_P(TestHydrolibRingQueueRead, ReadShifted) {
  auto param = GetParam();
  int shift = param.shift;
  int length = param.length;

  test_queue.Drop(kDefaultCapacity / 2);

  for (int i = 0; i < kDefaultCapacity / 2; i++) {
    hydrolib::ReturnCode filling_status =
        test_queue.PushByte(kDefaultCapacity + i);
    EXPECT_EQ(filling_status, hydrolib::ReturnCode::OK);
  }

  std::array<uint8_t, kDefaultCapacity> data{};
  hydrolib::ReturnCode read_status =
      test_queue.Read(data.data(), length, shift);
  if (length + shift > kDefaultCapacity) {
    EXPECT_EQ(read_status, hydrolib::ReturnCode::FAIL);
  } else {
    EXPECT_EQ(read_status, hydrolib::ReturnCode::OK);

    for (int i = 0; i < length; i++) {
      EXPECT_EQ(shift + i + (kDefaultCapacity / 2), data[i]);
    }
  }
}

TEST_F(TestHydrolibRingQueueRead, ReadByte) {
  for (int i = 0; i < kDefaultCapacity; i++) {
    EXPECT_EQ(i, test_queue[i]);
  }
}

TEST_F(TestHydrolibRingQueueRead, ReadByteShifted) {
  test_queue.Drop(kDefaultCapacity / 2);

  for (int i = 0; i < kDefaultCapacity / 2; i++) {
    hydrolib::ReturnCode filling_status =
        test_queue.PushByte(kDefaultCapacity + i);
    EXPECT_EQ(filling_status, hydrolib::ReturnCode::OK);
  }

  for (int i = 0; i < kDefaultCapacity; i++) {
    EXPECT_EQ(i + (kDefaultCapacity / 2), test_queue[i]);
  }
}

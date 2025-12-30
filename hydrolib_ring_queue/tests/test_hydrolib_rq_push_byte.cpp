#include "test_hydrolib_rq_env.hpp"

TEST_F(TestHydrolibRingQueue, PushByte) {
  uint8_t write_byte = 1;
  hydrolib::ReturnCode push_status = test_queue.PushByte(write_byte);
  EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);
  EXPECT_EQ(test_queue.GetLength(), 1);

  EXPECT_EQ(write_byte, test_queue[0]);
}

TEST_F(TestHydrolibRingQueue, PushSomeBytes) {
  for (int i = 0; i < kDefaultCapacity; i++) {
    hydrolib::ReturnCode push_status = test_queue.PushByte(i);
    EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);
    EXPECT_EQ(test_queue.GetLength(), i + 1);
    EXPECT_EQ(i, test_queue[i]);
  }
}

TEST_F(TestHydrolibRingQueue, PushBytesToLimit) {
  for (int i = 0; i < kDefaultCapacity; i++) {
    hydrolib::ReturnCode push_status = test_queue.PushByte(i);
    EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);
  }

  uint8_t write_byte = 1;
  hydrolib::ReturnCode full_push_status = test_queue.PushByte(write_byte);
  EXPECT_EQ(full_push_status, hydrolib::ReturnCode::FAIL);

  EXPECT_EQ(test_queue.GetLength(), kDefaultCapacity);
  EXPECT_EQ(test_queue[kDefaultCapacity - 1], kDefaultCapacity - 1);
}

#include "hydrolib_return_codes.hpp"
#include "test_hydrolib_rq_env.hpp"

TEST_F(TestHydrolibRingQueue, IsEmpty) {
  bool is_empty_initial = test_queue.IsEmpty();
  EXPECT_TRUE(is_empty_initial);

  uint8_t write_byte = 1;
  hydrolib::ReturnCode push_status = test_queue.PushByte(write_byte);
  EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);

  bool is_empty_push = test_queue.IsEmpty();
  EXPECT_FALSE(is_empty_push);

  uint8_t read_byte = 0;
  hydrolib::ReturnCode pull_status = test_queue.PullByte(&read_byte);
  EXPECT_EQ(pull_status, hydrolib::ReturnCode::OK);

  bool is_empty_pull = test_queue.IsEmpty();
  EXPECT_TRUE(is_empty_pull);
}

TEST_F(TestHydrolibRingQueue, IsFull) {
  bool is_full_initial = test_queue.IsFull();
  EXPECT_FALSE(is_full_initial);

  for (int i = 0; i < kDefaultCapacity; i++) {
    hydrolib::ReturnCode push_status = test_queue.PushByte(i);
    EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);
  }

  bool is_full_after_filling = test_queue.IsFull();
  EXPECT_TRUE(is_full_after_filling);

  uint8_t read_byte = 0;
  hydrolib::ReturnCode pull_status = test_queue.PullByte(&read_byte);
  EXPECT_EQ(pull_status, hydrolib::ReturnCode::OK);

  bool is_full_after_pull = test_queue.IsFull();
  EXPECT_FALSE(is_full_after_pull);

  uint8_t write_byte = 1;
  hydrolib::ReturnCode finall_push_status = test_queue.PushByte(write_byte);
  EXPECT_EQ(finall_push_status, hydrolib::ReturnCode::OK);

  bool is_full_after_push = test_queue.IsFull();
  EXPECT_TRUE(is_full_after_push);
}

TEST_F(TestHydrolibRingQueue, Length) {
  for (int i = 0; i < kDefaultCapacity; i++) {
    hydrolib::ReturnCode push_status = test_queue.PushByte(i);
    EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);

    int length = test_queue.GetLength();
    EXPECT_EQ(i + 1, length);
  }
  for (int i = 0; i < kDefaultCapacity; i++) {
    uint8_t read_byte = 0;
    hydrolib::ReturnCode pull_status = test_queue.PullByte(&read_byte);
    EXPECT_EQ(pull_status, hydrolib::ReturnCode::OK);

    int length = test_queue.GetLength();
    EXPECT_EQ(kDefaultCapacity - i - 1, length);
  }
}

TEST_F(TestHydrolibRingQueue, Clear) {
  for (int i = 0; i < kDefaultCapacity; i++) {
    hydrolib::ReturnCode push_status = test_queue.PushByte(i);
    EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);
  }
  test_queue.Clear();

  bool is_empty = test_queue.IsEmpty();
  EXPECT_TRUE(is_empty);
}

TEST_F(TestHydrolibRingQueue, Drop) {
  for (int i = 0; i < kDefaultCapacity; i++) {
    hydrolib::ReturnCode filling_status = test_queue.PushByte(i);
    EXPECT_EQ(filling_status, hydrolib::ReturnCode::OK);
  }
  hydrolib::ReturnCode drop_status = test_queue.Drop(kDefaultCapacity / 2);
  EXPECT_EQ(drop_status, hydrolib::ReturnCode::OK);

  int length = test_queue.GetLength();
  EXPECT_EQ(kDefaultCapacity / 2, length);

  EXPECT_EQ(kDefaultCapacity / 2, test_queue[0]);

  hydrolib::ReturnCode wrong_drop_status =
      test_queue.Drop(kDefaultCapacity + 1);
  EXPECT_EQ(wrong_drop_status, hydrolib::ReturnCode::FAIL);
}

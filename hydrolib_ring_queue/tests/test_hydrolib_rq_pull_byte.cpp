#include "hydrolib_return_codes.hpp"
#include "test_hydrolib_rq_env.hpp"

TEST_F(TestHydrolibRingQueue, PullByte)
{
    uint8_t write_byte = 1;
    hydrolib::ReturnCode push_status =
        test_queue.PushByte(  write_byte);
    EXPECT_EQ(push_status, hydrolib::ReturnCode::kOk);

    uint8_t read_byte = -1;
    hydrolib::ReturnCode pull_status =
        test_queue.PullByte(  &read_byte);
    EXPECT_EQ(pull_status, hydrolib::ReturnCode::kOk);
    EXPECT_EQ(write_byte, read_byte);
}

TEST_F(TestHydrolibRingQueue, PullSomeBytes)
{
    for (uint8_t i = 0; i < kDefaultCapacity; i++)
    {
        uint8_t write_byte = 1;
        hydrolib::ReturnCode push_status =
            test_queue.PushByte(  write_byte);
        EXPECT_EQ(push_status, hydrolib::ReturnCode::kOk);
    }

    for (uint8_t i = 0; i < kDefaultCapacity; i++)
    {
        uint8_t write_byte = 1;
        uint8_t read_byte = -1;
        hydrolib::ReturnCode pull_status =
            test_queue.PullByte(  &read_byte);
        EXPECT_EQ(pull_status, hydrolib::ReturnCode::kOk);
        EXPECT_EQ(write_byte, read_byte);
    }
}

TEST_F(TestHydrolibRingQueue, PullByteFromEmpty)
{
    uint8_t read_byte = -1;
    hydrolib::ReturnCode initial_pull_status =
        test_queue.PullByte(  &read_byte);
    EXPECT_EQ(initial_pull_status, hydrolib::ReturnCode::kFail);

    uint8_t write_byte = 1;
    hydrolib::ReturnCode push_status =
        test_queue.PushByte(  write_byte);
    EXPECT_EQ(push_status, hydrolib::ReturnCode::kOk);

    read_byte = -1;
    hydrolib::ReturnCode correct_pull_status =
        test_queue.PullByte(  &read_byte);
    EXPECT_EQ(correct_pull_status, hydrolib::ReturnCode::kOk);
    EXPECT_EQ(write_byte, read_byte);

    hydrolib::ReturnCode final_pull_status =
        test_queue.PullByte(  &read_byte);
    EXPECT_EQ(final_pull_status, hydrolib::ReturnCode::kFail);
}

TEST_F(TestHydrolibRingQueue, PullByteFromEmptyAfterFilling)
{
    for (uint8_t i = 0; i < kDefaultCapacity; i++)
    {
        hydrolib::ReturnCode push_status =
            test_queue.PushByte(i);
        EXPECT_EQ(push_status, hydrolib::ReturnCode::kOk);
    }

    uint8_t read_byte;
    for (uint8_t i = 0; i < kDefaultCapacity; i++)
    {
        hydrolib::ReturnCode pull_status =
            test_queue.PullByte(  &read_byte);
        EXPECT_EQ(pull_status, hydrolib::ReturnCode::kOk);
    }

    hydrolib::ReturnCode pull_empty_status =
        test_queue.PullByte(  &read_byte);
    EXPECT_EQ(pull_empty_status, hydrolib::ReturnCode::kFail);
}

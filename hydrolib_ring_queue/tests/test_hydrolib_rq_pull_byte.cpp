#include "hydrolib_return_codes.hpp"
#include "test_hydrolib_rq_env.hpp"

TEST_F(TestHydrolibRingQueue, PullByte)
{
    uint8_t write_byte = 1;
    hydrolib::ReturnCode push_status =
        test_queue.PushByte(  write_byte);
    EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);

    uint8_t read_byte = -1;
    hydrolib::ReturnCode pull_status =
        test_queue.PullByte(  &read_byte);
    EXPECT_EQ(pull_status, hydrolib::ReturnCode::OK);
    EXPECT_EQ(write_byte, read_byte);
}

TEST_F(TestHydrolibRingQueue, PullSomeBytes)
{
    for (uint8_t i = 0; i < buffer_capacity; i++)
    {
        uint8_t write_byte = 1;
        hydrolib::ReturnCode push_status =
            test_queue.PushByte(  write_byte);
        EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);
    }

    for (uint8_t i = 0; i < buffer_capacity; i++)
    {
        uint8_t write_byte = 1;
        uint8_t read_byte = -1;
        hydrolib::ReturnCode pull_status =
            test_queue.PullByte(  &read_byte);
        EXPECT_EQ(pull_status, hydrolib::ReturnCode::OK);
        EXPECT_EQ(write_byte, read_byte);
    }
}

TEST_F(TestHydrolibRingQueue, PullByteFromEmpty)
{
    uint8_t read_byte = -1;
    hydrolib::ReturnCode initial_pull_status =
        test_queue.PullByte(  &read_byte);
    EXPECT_EQ(initial_pull_status, hydrolib::ReturnCode::FAIL);

    uint8_t write_byte = 1;
    hydrolib::ReturnCode push_status =
        test_queue.PushByte(  write_byte);
    EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);

    read_byte = -1;
    hydrolib::ReturnCode correct_pull_status =
        test_queue.PullByte(  &read_byte);
    EXPECT_EQ(correct_pull_status, hydrolib::ReturnCode::OK);
    EXPECT_EQ(write_byte, read_byte);

    hydrolib::ReturnCode final_pull_status =
        test_queue.PullByte(  &read_byte);
    EXPECT_EQ(final_pull_status, hydrolib::ReturnCode::FAIL);
}

TEST_F(TestHydrolibRingQueue, PullByteFromEmptyAfterFilling)
{
    for (uint8_t i = 0; i < buffer_capacity; i++)
    {
        hydrolib::ReturnCode push_status =
            test_queue.PushByte(i);
        EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);
    }

    uint8_t read_byte;
    for (uint8_t i = 0; i < buffer_capacity; i++)
    {
        hydrolib::ReturnCode pull_status =
            test_queue.PullByte(  &read_byte);
        EXPECT_EQ(pull_status, hydrolib::ReturnCode::OK);
    }

    hydrolib::ReturnCode pull_empty_status =
        test_queue.PullByte(  &read_byte);
    EXPECT_EQ(pull_empty_status, hydrolib::ReturnCode::FAIL);
}

#include "hydrolib_return_codes.hpp"
#include "test_hydrolib_rq_env.hpp"

TEST_F(TestHydrolibRingQueue, IsEmpty)
{
    bool is_empty_initial =
        test_queue.IsEmpty( );
    EXPECT_TRUE(is_empty_initial);

    uint8_t write_byte = 1;
    hydrolib::ReturnCode push_status =
        test_queue.PushByte(  write_byte);
    EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);

    bool is_empty_push = test_queue.IsEmpty( );
    EXPECT_FALSE(is_empty_push);

    uint8_t read_byte;
    hydrolib::ReturnCode pull_status =
        test_queue.PullByte(  &read_byte);
    EXPECT_EQ(pull_status, hydrolib::ReturnCode::OK);

    bool is_empty_pull = test_queue.IsEmpty( );
    EXPECT_TRUE(is_empty_pull);
}

TEST_F(TestHydrolibRingQueue, IsFull)
{
    bool is_full_initial =
        test_queue.IsFull( );
    EXPECT_FALSE(is_full_initial);

    for (uint16_t i = 0; i < buffer_capacity; i++)
    {
        hydrolib::ReturnCode push_status =
            test_queue.PushByte(  i);
        EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);
    }

    bool is_full_after_filling =
        test_queue.IsFull( );
    EXPECT_TRUE(is_full_after_filling);

    uint8_t read_byte;
    hydrolib::ReturnCode pull_status =
        test_queue.PullByte(  &read_byte);
    EXPECT_EQ(pull_status, hydrolib::ReturnCode::OK);

    bool is_full_after_pull =
        test_queue.IsFull( );
    EXPECT_FALSE(is_full_after_pull);

    uint8_t write_byte = 1;
    hydrolib::ReturnCode finall_push_status =
        test_queue.PushByte(  write_byte);
    EXPECT_EQ(finall_push_status, hydrolib::ReturnCode::OK);

    bool is_full_after_push =
        test_queue.IsFull( );
    EXPECT_TRUE(is_full_after_push);
}

TEST_P(TestHydrolibRingQueueCommon, Length)
{
    uint16_t push_number = GetParam();
    for (uint16_t i = 0; i < push_number; i++)
    {
        hydrolib::ReturnCode push_status =
            test_queue.PushByte(  i);
        EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);

        uint16_t length =
            test_queue.GetLength( );
        EXPECT_EQ(i + 1, length);
    }
    for (uint16_t i = 0; i < push_number; i++)
    {
        uint8_t read_byte;
        hydrolib::ReturnCode pull_status =
            test_queue.PullByte(  &read_byte);
        EXPECT_EQ(pull_status, hydrolib::ReturnCode::OK);

        uint16_t length =
            test_queue.GetLength( );
        EXPECT_EQ(push_number - i - 1, length);
    }
}

TEST_P(TestHydrolibRingQueueCommon, Clear)
{
    uint16_t push_number = GetParam();
    for (uint16_t i = 0; i < push_number; i++)
    {
        hydrolib::ReturnCode push_status =
            test_queue.PushByte(  i);
        EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);
    }
    test_queue.Clear( );

    bool is_empty = test_queue.IsEmpty( );
    EXPECT_TRUE(is_empty);
}

TEST_P(TestHydrolibRingQueueCommon, Drop)
{
    uint16_t drop_number = GetParam();
    for (uint16_t i = 0; i < buffer_capacity; i++)
    {
        hydrolib::ReturnCode filling_status =
            test_queue.PushByte(  i);
        EXPECT_EQ(filling_status, hydrolib::ReturnCode::OK);
    }
    hydrolib::ReturnCode drop_status =
        test_queue.Drop(  drop_number);
    EXPECT_EQ(drop_status, hydrolib::ReturnCode::OK);

    uint16_t length = test_queue.GetLength( );
    EXPECT_EQ(buffer_capacity - drop_number, length);

    uint8_t read_byte = -1;
    hydrolib::ReturnCode read_status =
        test_queue.ReadByte(  &read_byte, 0);
    EXPECT_EQ(read_status, hydrolib::ReturnCode::OK);
    EXPECT_EQ(drop_number, read_byte);

    hydrolib::ReturnCode wrong_drop_status =
        test_queue.Drop( 
                                              buffer_capacity + 1);
    EXPECT_EQ(wrong_drop_status, hydrolib::ReturnCode::FAIL);
}

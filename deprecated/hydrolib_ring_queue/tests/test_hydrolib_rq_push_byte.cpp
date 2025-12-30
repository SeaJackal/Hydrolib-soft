#include "test_hydrolib_rq_env.hpp"

TEST_F(TestHydrolibRingQueue, PushByte)
{
    uint8_t write_byte = 1;
    hydrolib::ReturnCode push_status =
        test_queue.PushByte(  write_byte);
    EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);

    uint8_t read_byte;
    hydrolib::ReturnCode read_status =
        test_queue.ReadByte(  &read_byte, 0);
    EXPECT_EQ(read_status, hydrolib::ReturnCode::OK);
    EXPECT_EQ(write_byte, read_byte);
}

TEST_F(TestHydrolibRingQueue, PushSomeBytes)
{
    for (uint8_t i = 0; i < buffer_capacity; i++)
    {
        hydrolib::ReturnCode push_status =
            test_queue.PushByte(  i);
        EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);

        uint8_t read_byte;
        hydrolib::ReturnCode read_status =
            test_queue.ReadByte(  &read_byte,
                                                      i);
        EXPECT_EQ(read_status, hydrolib::ReturnCode::OK);
        EXPECT_EQ(i, read_byte);
    }
}

TEST_F(TestHydrolibRingQueue, PushBytesToLimit)
{
    for (uint8_t i = 0; i < buffer_capacity; i++)
    {
        hydrolib::ReturnCode push_status =
            test_queue.PushByte(  i);
        EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);
    }

    uint8_t write_byte = 1;
    hydrolib::ReturnCode full_push_status =
        test_queue.PushByte(  write_byte);
    EXPECT_EQ(full_push_status, hydrolib::ReturnCode::FAIL);

    uint8_t read_byte;
    hydrolib::ReturnCode read_status =
        test_queue.ReadByte(  &read_byte,
                                                  buffer_capacity - 1);
    EXPECT_EQ(read_status, hydrolib::ReturnCode::OK);
    EXPECT_EQ(buffer_capacity - 1, read_byte);
}

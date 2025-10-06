#include "test_hydrolib_rq_env.hpp"

TEST_P(TestHydrolibRingQueueCommon, FindByte)
{
    uint16_t shift = GetParam();

    for (uint16_t i = 0; i < buffer_capacity / 2; i++)
    {
        hydrolib::ReturnCode push_status =
            test_queue.PushByte(  i);
        EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);
    }

    for (uint16_t i = 0; i < buffer_capacity / 2; i++)
    {
        uint8_t read_byte;
        hydrolib::ReturnCode emptying_status =
            test_queue.PullByte(  &read_byte);
        EXPECT_EQ(emptying_status, hydrolib::ReturnCode::OK);
    }

    for (uint16_t i = 0; i < buffer_capacity; i++)
    {
        hydrolib::ReturnCode filling_status =
            test_queue.PushByte(  i);
        EXPECT_EQ(filling_status, hydrolib::ReturnCode::OK);
    }

    for (uint16_t i = 0; i < buffer_capacity; i++)
    {
        uint16_t found_index =
            test_queue.FindByte(i, shift);
        if (i >= shift)
        {
            EXPECT_EQ(i, found_index);
        }
        else
        {
            EXPECT_EQ((uint16_t)(-1), found_index);
        }
    }

    uint16_t found_index = test_queue.FindByte(
          buffer_capacity, shift);
    EXPECT_EQ((uint16_t)(-1), found_index);
}

TEST_P(TestHydrolibRingQueueCommon, Find2BytesLE)
{
    uint16_t shift = GetParam();

    for (uint16_t i = 0; i < buffer_capacity / 2; i++)
    {
        hydrolib::ReturnCode push_status =
            test_queue.PushByte(  i);
        EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);
    }

    for (uint16_t i = 0; i < buffer_capacity / 2; i++)
    {
        uint8_t read_byte;
        hydrolib::ReturnCode emptying_status =
            test_queue.PullByte(  &read_byte);
        EXPECT_EQ(emptying_status, hydrolib::ReturnCode::OK);
    }

    for (uint16_t i = 0; i < buffer_capacity; i++)
    {
        hydrolib::ReturnCode filling_status =
            test_queue.PushByte(  i);
        EXPECT_EQ(filling_status, hydrolib::ReturnCode::OK);
    }

    for (uint16_t i = 0; i < buffer_capacity - 1; i++)
    {
        uint16_t test_bytes = i | (i + 1) << 8;
        uint16_t found_index = test_queue.Find2BytesLE(
              test_bytes, shift);
        if (i >= shift)
        {
            EXPECT_EQ(i, found_index);
        }
        else
        {
            EXPECT_EQ((uint16_t)(-1), found_index);
        }
    }

    uint16_t found_index =
        test_queue.Find2BytesLE(  buffer_capacity, shift);
    EXPECT_EQ((uint16_t)(-1), found_index);
}

TEST_P(TestHydrolibRingQueueCommon, Find4BytesLE)
{
    uint16_t shift = GetParam();

    for (uint16_t i = 0; i < buffer_capacity / 2; i++)
    {
        hydrolib::ReturnCode push_status =
            test_queue.PushByte(  i);
        EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);
    }

    for (uint16_t i = 0; i < buffer_capacity / 2; i++)
    {
        uint8_t read_byte;
        hydrolib::ReturnCode emptying_status =
            test_queue.PullByte(  &read_byte);
        EXPECT_EQ(emptying_status, hydrolib::ReturnCode::OK);
    }

    for (uint16_t i = 0; i < buffer_capacity; i++)
    {
        hydrolib::ReturnCode filling_status =
            test_queue.PushByte(  i);
        EXPECT_EQ(filling_status, hydrolib::ReturnCode::OK);
    }

    for (uint16_t i = 0; i < buffer_capacity - 3; i++)
    {
        uint32_t test_bytes = i | (i + 1) << 8 | (i + 2) << 16 | (i + 3) << 24;
        uint16_t found_index = test_queue.Find4BytesLE(
              test_bytes, shift);
        if (i >= shift)
        {
            EXPECT_EQ(i, found_index);
        }
        else
        {
            EXPECT_EQ((uint16_t)(-1), found_index);
        }
    }

    uint16_t found_index = test_queue.Find4BytesLE(
          buffer_capacity, shift);
    EXPECT_EQ((uint16_t)(-1), found_index);
}

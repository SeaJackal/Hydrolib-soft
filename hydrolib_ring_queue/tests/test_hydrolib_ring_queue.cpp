#include <hydrolib_ring_queue.h>

#include <gtest/gtest.h>

TEST(TestHydrolibRingQueue, PushByte)
{
    hydrolib_RingQueue ring_buffer;
    const uint8_t buffer_capacity = 16;
    uint8_t buffer[buffer_capacity];

    hydrolib_RingQueue_Init(&ring_buffer, buffer, buffer_capacity);

    uint8_t test_byte = 1;
    hydrolib_ReturnCode status = hydrolib_RingQueue_PushByte(&ring_buffer, test_byte);
    EXPECT_EQ(status, hydrolib_ReturnCode::HYDROLIB_RETURN_OK);

    uint8_t read_byte;
    status = hydrolib_RingQueue_ReadByte(&ring_buffer, &read_byte, 0);
    EXPECT_EQ(status, hydrolib_ReturnCode::HYDROLIB_RETURN_OK);
    EXPECT_EQ(test_byte, read_byte);
}

TEST(TestHydrolibRingQueue, PushSomeBytes)
{
    hydrolib_RingQueue ring_buffer;
    const uint8_t buffer_capacity = 16;
    uint8_t buffer[buffer_capacity];

    hydrolib_RingQueue_Init(&ring_buffer, buffer, buffer_capacity);

    for (uint8_t i = 0; i < buffer_capacity; i++)
    {
        hydrolib_ReturnCode status = hydrolib_RingQueue_PushByte(&ring_buffer, i);
        EXPECT_EQ(status, hydrolib_ReturnCode::HYDROLIB_RETURN_OK);

        uint8_t read_byte;
        status = hydrolib_RingQueue_ReadByte(&ring_buffer, &read_byte, i);
        EXPECT_EQ(status, hydrolib_ReturnCode::HYDROLIB_RETURN_OK);
        EXPECT_EQ(i, read_byte);
    }
}

TEST(TestHydrolibRingQueue, PullByte)
{
    hydrolib_RingQueue ring_buffer;
    const uint8_t buffer_capacity = 16;
    uint8_t buffer[buffer_capacity];

    hydrolib_RingQueue_Init(&ring_buffer, buffer, buffer_capacity);

    uint8_t test_byte = 1;
    hydrolib_ReturnCode status = hydrolib_RingQueue_PushByte(&ring_buffer, test_byte);
    EXPECT_EQ(status, hydrolib_ReturnCode::HYDROLIB_RETURN_OK);

    uint8_t read_byte;
    status = hydrolib_RingQueue_PullByte(&ring_buffer, &read_byte);
    EXPECT_EQ(status, hydrolib_ReturnCode::HYDROLIB_RETURN_OK);
    EXPECT_EQ(test_byte, read_byte);

    status = hydrolib_RingQueue_PullByte(&ring_buffer, &read_byte);
    EXPECT_EQ(status, hydrolib_ReturnCode::HYDROLIB_RETURN_FAIL);
}

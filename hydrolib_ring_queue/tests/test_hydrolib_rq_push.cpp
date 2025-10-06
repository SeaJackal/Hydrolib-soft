#include "test_hydrolib_rq_env.hpp"

class TestHydrolibRingQueuePush : public TestHydrolibRingQueue,
                                  public ::testing::WithParamInterface<uint16_t>
{
};

INSTANTIATE_TEST_CASE_P(Test, TestHydrolibRingQueuePush,
                        ::testing::Range<uint16_t>(1, DEFAULT_CAPACITY));

TEST_P(TestHydrolibRingQueuePush, Push)
{
    uint16_t length = GetParam();
    uint8_t data[DEFAULT_CAPACITY];
    for (uint16_t i = 0; i < DEFAULT_CAPACITY; i++)
    {
        data[i] = i;
    }

    hydrolib::ReturnCode push_status =
        test_queue.Push(  data, length);
    EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);

    for (uint16_t i = 0; i < length; i++)
    {
        uint8_t read_byte;
        hydrolib::ReturnCode read_status =
            test_queue.ReadByte(  &read_byte,
                                                      i);
        EXPECT_EQ(read_status, hydrolib::ReturnCode::OK);
        EXPECT_EQ(i, read_byte);
    }
}

TEST_P(TestHydrolibRingQueuePush, PushShifted)
{
    uint16_t length = GetParam();
    uint8_t data[DEFAULT_CAPACITY];
    for (uint16_t i = 0; i < DEFAULT_CAPACITY; i++)
    {
        data[i] = i;
    }

    for (uint16_t i = 0; i < DEFAULT_CAPACITY / 2; i++)
    {
        hydrolib::ReturnCode filling_status =
            test_queue.PushByte(  0);
        EXPECT_EQ(filling_status, hydrolib::ReturnCode::OK);
    }

    test_queue.Drop(  DEFAULT_CAPACITY / 2);

    hydrolib::ReturnCode push_status =
        test_queue.Push(  data, length);
    EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);

    for (uint16_t i = 0; i < length; i++)
    {
        uint8_t read_byte;
        hydrolib::ReturnCode read_status =
            test_queue.ReadByte(  &read_byte,
                                                      i);
        EXPECT_EQ(read_status, hydrolib::ReturnCode::OK);
        EXPECT_EQ(i, read_byte);
    }
}

TEST_F(TestHydrolibRingQueue, PushOverSimple)
{
    uint8_t data[DEFAULT_CAPACITY + 1];
    for (uint16_t i = 0; i < DEFAULT_CAPACITY + 1; i++)
    {
        data[i] = i;
    }

    hydrolib::ReturnCode push_status = test_queue.Push(
          data, DEFAULT_CAPACITY + 1);
    EXPECT_EQ(push_status, hydrolib::ReturnCode::FAIL);
}

TEST_P(TestHydrolibRingQueuePush, PushOverComplex)
{
    uint16_t length = GetParam();
    uint8_t data[DEFAULT_CAPACITY];
    for (uint16_t i = 0; i < DEFAULT_CAPACITY; i++)
    {
        data[i] = i;
    }

    for (uint16_t i = 0; i < length; i++)
    {
        hydrolib::ReturnCode filling_status =
            test_queue.PushByte(  data[i]);
        EXPECT_EQ(filling_status, hydrolib::ReturnCode::OK);
    }

    hydrolib::ReturnCode push_status = test_queue.Push(
          data, DEFAULT_CAPACITY - length + 1);
    EXPECT_EQ(push_status, hydrolib::ReturnCode::FAIL);

    for (uint16_t i = 0; i < length; i++)
    {
        uint8_t read_byte;
        hydrolib::ReturnCode read_status =
            test_queue.ReadByte(  &read_byte,
                                                      i);
        EXPECT_EQ(read_status, hydrolib::ReturnCode::OK);
        EXPECT_EQ(i, read_byte);
    }
}

TEST_P(TestHydrolibRingQueuePush, PushToLimit)
{
    uint16_t length = GetParam();
    uint8_t data[DEFAULT_CAPACITY];
    for (uint16_t i = 0; i < DEFAULT_CAPACITY; i++)
    {
        data[i] = i;
    }

    for (uint16_t i = length; i <= DEFAULT_CAPACITY; i += length)
    {
        hydrolib::ReturnCode push_status =
            test_queue.Push( 
                                                  data + i - length, length);
        EXPECT_EQ(push_status, hydrolib::ReturnCode::OK);
    }

    for (uint16_t i = 0; i < DEFAULT_CAPACITY / length * length; i++)
    {
        uint8_t read_byte;
        hydrolib::ReturnCode read_status =
            test_queue.ReadByte(  &read_byte,
                                                      i);
        EXPECT_EQ(read_status, hydrolib::ReturnCode::OK);
        EXPECT_EQ(i, read_byte);
    }
}

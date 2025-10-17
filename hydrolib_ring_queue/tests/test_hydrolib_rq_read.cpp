#include "hydrolib_return_codes.hpp"
#include "test_hydrolib_rq_env.hpp"

class TestHydrolibRingQueueRead
    : public TestHydrolibRingQueue,
      public ::testing::WithParamInterface<std::tuple<uint16_t, uint16_t>>
{
};

INSTANTIATE_TEST_CASE_P(
    Test, TestHydrolibRingQueueRead,
    ::testing::Combine(::testing::Range<uint16_t>(0, DEFAULT_CAPACITY),
                       ::testing::Range<uint16_t>(1, DEFAULT_CAPACITY + 1)));

TEST_P(TestHydrolibRingQueueRead, Read)
{
    auto param = GetParam();
    uint16_t shift = std::get<0>(param);
    uint16_t length = std::get<1>(param);

    for (uint16_t i = 0; i < DEFAULT_CAPACITY; i++)
    {
        hydrolib::ReturnCode filling_status = test_queue.PushByte(i);
        EXPECT_EQ(filling_status, hydrolib::ReturnCode::OK);
    }

    uint8_t data[DEFAULT_CAPACITY];
    hydrolib::ReturnCode read_status = test_queue.Read(data, length, shift);
    if (length + shift > DEFAULT_CAPACITY)
    {
        EXPECT_EQ(read_status, hydrolib::ReturnCode::FAIL);
    }
    else
    {
        EXPECT_EQ(read_status, hydrolib::ReturnCode::OK);

        for (uint8_t i = 0; i < length; i++)
        {
            EXPECT_EQ(shift + i, data[i]);
        }
    }
}

TEST_P(TestHydrolibRingQueueRead, ReadShifted)
{
    auto param = GetParam();
    uint16_t shift = std::get<0>(param);
    uint16_t length = std::get<1>(param);

    for (uint16_t i = 0; i < DEFAULT_CAPACITY; i++)
    {
        hydrolib::ReturnCode filling_status = test_queue.PushByte(i);
        EXPECT_EQ(filling_status, hydrolib::ReturnCode::OK);
    }

    test_queue.Drop(DEFAULT_CAPACITY / 2);

    for (uint16_t i = 0; i < DEFAULT_CAPACITY / 2; i++)
    {
        hydrolib::ReturnCode filling_status =
            test_queue.PushByte(DEFAULT_CAPACITY + i);
        EXPECT_EQ(filling_status, hydrolib::ReturnCode::OK);
    }

    uint8_t data[DEFAULT_CAPACITY];
    hydrolib::ReturnCode read_status = test_queue.Read(data, length, shift);
    if (length + shift > DEFAULT_CAPACITY)
    {
        EXPECT_EQ(read_status, hydrolib::ReturnCode::FAIL);
    }
    else
    {
        EXPECT_EQ(read_status, hydrolib::ReturnCode::OK);

        for (uint8_t i = 0; i < length; i++)
        {
            EXPECT_EQ(shift + i + DEFAULT_CAPACITY / 2, data[i]);
        }
    }
}

#include "hydrolib_serial_protocol_pack.hpp"

#include <deque>

#include <gtest/gtest.h>

using namespace hydrolib::serialProtocol;

namespace test_pack
{

#define PUBLIC_MEMORY_LENGTH 20

#define DEVICE_ADDRESS_RECEIVER 0
#define DEVICE_ADDRESS_TRANSMITTER 1

    SerialProtocolHandler *receiver_link;

    void Transmit(uint8_t *data, uint32_t length)
    {
        receiver_link->Receive(data, length);
    }

    void DummyTransmit(uint8_t *data, uint32_t length)
    {
        (void)data;
        (void)length;
    }

    class TestHydrolibSerialProtocolPack : public ::testing::Test
    {
    protected:
        TestHydrolibSerialProtocolPack() : transmitter(DEVICE_ADDRESS_TRANSMITTER, Transmit, nullptr, 0),
                                           receiver(DEVICE_ADDRESS_RECEIVER, DummyTransmit, public_memory, PUBLIC_MEMORY_LENGTH)
        {
            receiver_link = &receiver;
        }

        SerialProtocolHandler transmitter;
        SerialProtocolHandler receiver;

        uint8_t public_memory[PUBLIC_MEMORY_LENGTH];

        uint8_t test_data[0xFF];
    };

    class TestHydrolibSPpackMemoryAccess : public TestHydrolibSerialProtocolPack,
                                           public ::testing::WithParamInterface<std::tuple<uint16_t, uint16_t>>
    {
    };

    INSTANTIATE_TEST_CASE_P(
        Test,
        TestHydrolibSPpackMemoryAccess,
        ::testing::Combine(
            ::testing::Range<uint16_t>(0, PUBLIC_MEMORY_LENGTH),
            ::testing::Range<uint16_t>(1, PUBLIC_MEMORY_LENGTH + 1)));

    TEST_P(TestHydrolibSPpackMemoryAccess, MemWritingTest)
    {
        auto param = GetParam();
        uint8_t mem_address = std::get<0>(param);
        uint8_t writing_length = std::get<1>(param);
        for (uint8_t j = 0; j < 10; j++)
        {
            if (mem_address + writing_length > PUBLIC_MEMORY_LENGTH)
            {
                continue;
            }
            hydrolib_ReturnCode transmit_status = transmitter.TransmitWrite(DEVICE_ADDRESS_RECEIVER,
                                                                            mem_address, writing_length, test_data);
            EXPECT_EQ(HYDROLIB_RETURN_OK, transmit_status);

            receiver.ProcessRx();
            for (uint8_t i = 0; i < writing_length; i++)
            {
                EXPECT_EQ(test_data[i], public_memory[mem_address + i]);
            }
        }
    }

    TEST_P(TestHydrolibSPpackMemoryAccess, MemWritingWithNoizeTest)
    {
        auto param = GetParam();
        uint8_t mem_address = std::get<0>(param);
        uint8_t writing_length = std::get<1>(param);
        for (uint8_t j = 0; j < 10; j++)
        {
            if (mem_address + writing_length > PUBLIC_MEMORY_LENGTH)
            {
                continue;
            }
            Transmit(&j, 1);
            hydrolib_ReturnCode transmit_status =
                transmitter.TransmitWrite(DEVICE_ADDRESS_RECEIVER,
                                          mem_address, writing_length, test_data);

            EXPECT_EQ(HYDROLIB_RETURN_OK, transmit_status);

            Transmit(&j, 1);

            receiver.ProcessRx();
            for (uint8_t i = 0; i < writing_length; i++)
            {
                EXPECT_EQ(test_data[i], public_memory[mem_address + i]);
            }
        }
    }
}
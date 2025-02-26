#include "hydrolib_serial_protocol_core.hpp"

#include <deque>

#include <gtest/gtest.h>

using namespace hydrolib::serialProtocol;

namespace test_core
{

#define PUBLIC_MEMORY_LENGTH 20

#define DEVICE_ADDRESS_RECEIVER 0
#define DEVICE_ADDRESS_TRANSMITTER 1

    class TestRxQueue : public MessageProcessor::RxQueueInterface,
                        public MessageProcessor::TxQueueInterface
    {
    private:
        std::deque<uint8_t> queue;

    public:
        hydrolib_ReturnCode Read(void *buffer, uint32_t length, uint32_t shift) const override
        {
            uint8_t *byte_buffer = reinterpret_cast<uint8_t *>(buffer);
            if (shift + length > queue.size())
            {
                return HYDROLIB_RETURN_FAIL;
            }
            for (uint32_t i = 0; i < length; i++)
            {
                byte_buffer[i] = queue[shift + i];
            }
            return HYDROLIB_RETURN_OK;
        }

        void Drop(uint32_t number) override
        {
            if (number > queue.size())
            {
                queue.clear();
            }
            else
            {
                for (uint32_t i = 0; i < number; i++)
                {
                    queue.pop_front();
                }
            }
        }

        void Clear() override
        {
            queue.clear();
        }

        hydrolib_ReturnCode Push(void *data, uint32_t length) override
        {
            for (uint32_t i = 0; i < length; i++)
            {
                queue.push_back(reinterpret_cast<uint8_t *>(data)[i]);
            }
            return HYDROLIB_RETURN_OK;
        }

        void WriteByte(uint8_t data)
        {
            queue.push_back(data);
        }
    };

    class TestHydrolibSerialProtocolCore : public ::testing::Test
    {
    protected:
        TestHydrolibSerialProtocolCore() : transmitter(DEVICE_ADDRESS_TRANSMITTER, txrx_queue, rxtx_queue, nullptr, 0),
                                           receiver(DEVICE_ADDRESS_RECEIVER, rxtx_queue, txrx_queue, public_memory, PUBLIC_MEMORY_LENGTH)
        {
            for (int i = 0; i < 0xFF; i++)
            {
                test_data[i] = i;
            }
        }

        TestRxQueue txrx_queue;
        TestRxQueue rxtx_queue;

        MessageProcessor transmitter;
        MessageProcessor receiver;

        uint8_t public_memory[PUBLIC_MEMORY_LENGTH];

        uint8_t test_data[0xFF];
    };

    class TestHydrolibSPcoreMemoryAccess : public TestHydrolibSerialProtocolCore,
                                           public ::testing::WithParamInterface<std::tuple<uint16_t, uint16_t>>
    {
    };

    INSTANTIATE_TEST_CASE_P(
        Test,
        TestHydrolibSPcoreMemoryAccess,
        ::testing::Combine(
            ::testing::Range<uint16_t>(0, PUBLIC_MEMORY_LENGTH),
            ::testing::Range<uint16_t>(1, PUBLIC_MEMORY_LENGTH + 1)));

    TEST_P(TestHydrolibSPcoreMemoryAccess, MemWritingTest)
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

    TEST_P(TestHydrolibSPcoreMemoryAccess, MemWritingWithNoizeTest)
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
            txrx_queue.WriteByte(j);
            hydrolib_ReturnCode transmit_status =
                transmitter.TransmitWrite(DEVICE_ADDRESS_RECEIVER,
                                          mem_address, writing_length, test_data);

            EXPECT_EQ(HYDROLIB_RETURN_OK, transmit_status);

            txrx_queue.WriteByte(j);

            receiver.ProcessRx();
            for (uint8_t i = 0; i < writing_length; i++)
            {
                EXPECT_EQ(test_data[i], public_memory[mem_address + i]);
            }
        }
    }

}

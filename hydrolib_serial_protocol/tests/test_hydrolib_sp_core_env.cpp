#include "test_hydrolib_sp_core_env.hpp"

using namespace hydrolib::serialProtocol;

namespace test_core
{

    hydrolib_ReturnCode TestRxQueue::Read(void *buffer, uint32_t length, uint32_t shift) const
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

    void TestRxQueue::Drop(uint32_t number)
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

    void TestRxQueue::Clear()
    {
        queue.clear();
    }

    hydrolib_ReturnCode TestPublicMemory::Read(void *buffer, uint32_t address,
                                               uint32_t length)
    {
        memcpy(buffer, this->buffer + address, length);
        return HYDROLIB_RETURN_OK;
    }
    hydrolib_ReturnCode TestPublicMemory::Write(const void *buffer, uint32_t address,
                                                uint32_t length)
    {
        memcpy(this->buffer + address, buffer, length);
        return HYDROLIB_RETURN_OK;
    }
    uint32_t TestPublicMemory::Size()
    {
        return PUBLIC_MEMORY_LENGTH;
    }

    hydrolib_ReturnCode TestRxQueue::Push(void *data, uint32_t length)
    {
        for (uint32_t i = 0; i < length; i++)
        {
            queue.push_back(reinterpret_cast<uint8_t *>(data)[i]);
        }
        return HYDROLIB_RETURN_OK;
    }

    void TestRxQueue::WriteByte(uint8_t data)
    {
        queue.push_back(data);
    }

    TestHydrolibSerialProtocolCore::TestHydrolibSerialProtocolCore()
        : transmitter(DEVICE_ADDRESS_TRANSMITTER, txrx_queue, rxtx_queue, public_memory),
          receiver(DEVICE_ADDRESS_RECEIVER, rxtx_queue, txrx_queue,
                   public_memory)
    {
        for (int i = 0; i < 0xFF; i++)
        {
            test_data[i] = i;
        }
    }

    INSTANTIATE_TEST_CASE_P(
        Test,
        TestHydrolibSPcoreMemoryAccess,
        ::testing::Combine(
            ::testing::Range<uint16_t>(0, PUBLIC_MEMORY_LENGTH),
            ::testing::Range<uint16_t>(1, PUBLIC_MEMORY_LENGTH + 1)));

}

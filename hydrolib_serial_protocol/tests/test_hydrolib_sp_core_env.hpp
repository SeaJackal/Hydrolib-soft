#include "hydrolib_serial_protocol_core.hpp"

#include <deque>

#include <gtest/gtest.h>

using namespace hydrolib::serial_protocol;

namespace test_core
{

#define PUBLIC_MEMORY_LENGTH 20

#define DEVICE_ADDRESS_RECEIVER 3
#define DEVICE_ADDRESS_TRANSMITTER 4

    class TestRxQueue
    {
    private:
        std::deque<uint8_t> queue;

    public:
        hydrolib_ReturnCode Read(void *buffer, uint32_t length, uint32_t shift) const;

        void Drop(uint32_t number);

        void Clear();

        hydrolib_ReturnCode Push(const void *data, uint32_t length);

        void WriteByte(uint8_t data);
    };

    class TestPublicMemory
    {
    public:
        hydrolib_ReturnCode Read(void *buffer, uint32_t address,
                                 uint32_t length);
        hydrolib_ReturnCode Write(const void *buffer, uint32_t address,
                                  uint32_t length);
        uint32_t Size();

    public:
        uint8_t buffer[PUBLIC_MEMORY_LENGTH];
    };

    class TestHydrolibSerialProtocolCore : public ::testing::Test
    {
    protected:
        TestHydrolibSerialProtocolCore();
        TestRxQueue txrx_queue;
        TestRxQueue rxtx_queue;

        MessageProcessor<TestRxQueue, TestRxQueue, TestPublicMemory> transmitter;
        MessageProcessor<TestRxQueue, TestRxQueue, TestPublicMemory> receiver;

        TestPublicMemory public_memory;

        uint8_t test_data[0xFF];
    };

    class TestHydrolibSPcoreMemoryAccess : public TestHydrolibSerialProtocolCore,
                                           public ::testing::WithParamInterface<std::tuple<uint16_t, uint16_t>>
    {
    };

}

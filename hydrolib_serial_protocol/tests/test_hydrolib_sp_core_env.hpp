#include "hydrolib_serial_protocol_core.hpp"

#include <deque>

#include <gtest/gtest.h>

using namespace hydrolib::serialProtocol;

namespace test_core
{

#define PUBLIC_MEMORY_LENGTH 20

#define DEVICE_ADDRESS_RECEIVER 3
#define DEVICE_ADDRESS_TRANSMITTER 4

    class TestRxQueue : public MessageProcessor::RxQueueInterface,
                        public MessageProcessor::TxQueueInterface
    {
    private:
        std::deque<uint8_t> queue;

    public:
        hydrolib_ReturnCode Read(void *buffer, uint32_t length, uint32_t shift) const override;

        void Drop(uint32_t number) override;

        void Clear() override;

        hydrolib_ReturnCode Push(void *data, uint32_t length) override;

        void WriteByte(uint8_t data);
    };

    class TestPublicMemory : public MessageProcessor::PublicMemoryInterface
    {
    public:
        hydrolib_ReturnCode Read(void *buffer, uint32_t address,
                                 uint32_t length) override;
        hydrolib_ReturnCode Write(const void *buffer, uint32_t address,
                                  uint32_t length) override;
        uint32_t Size() override;

    public:
        uint8_t buffer[PUBLIC_MEMORY_LENGTH];
    };

    class TestHydrolibSerialProtocolCore : public ::testing::Test
    {
    protected:
        TestHydrolibSerialProtocolCore();
        TestRxQueue txrx_queue;
        TestRxQueue rxtx_queue;

        MessageProcessor transmitter;
        MessageProcessor receiver;

        TestPublicMemory public_memory;

        uint8_t test_data[0xFF];
    };

    class TestHydrolibSPcoreMemoryAccess : public TestHydrolibSerialProtocolCore,
                                           public ::testing::WithParamInterface<std::tuple<uint16_t, uint16_t>>
    {
    };

}

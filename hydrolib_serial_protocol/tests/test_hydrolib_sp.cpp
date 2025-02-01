#include "hydrolib_serial_protocol.h"

#include "hydrolib_ring_queue.h"

#include <string.h>

#include <gtest/gtest.h>

#define PUBLIC_MEMORY_LENGTH 200

#define DEVICE_ADDRESS_RECEIVER 0
#define DEVICE_ADDRESS_TRANSMITTER 1

hydrolib_RingQueue txrx_ring_queue;

bool ReceiveFunc(uint8_t *byte);
bool TransmitFunc(uint8_t *byte);
bool DummyReceiveFunc(uint8_t *byte);
bool DummyTransmitFunc(uint8_t *byte);
uint8_t CRCfunc(const uint8_t *buffer, uint16_t length);

class TestHydrolibSerialProtocol : public ::testing::Test,
                                   public ::testing::WithParamInterface<uint16_t>
{
protected:
    TestHydrolibSerialProtocol()
    {
        hydrolib_SerialProtocol_Init(&transmitter, DEVICE_ADDRESS_TRANSMITTER,
                                     DummyReceiveFunc, TransmitFunc, CRCfunc,
                                     tx_public_memory, PUBLIC_MEMORY_LENGTH);
        hydrolib_SerialProtocol_Init(&receiver, DEVICE_ADDRESS_RECEIVER,
                                     ReceiveFunc, DummyTransmitFunc, CRCfunc,
                                     rx_public_memory, PUBLIC_MEMORY_LENGTH);
        _hydrolib_SP_MessageHeaderMemAccess *header_mem_access =
            (_hydrolib_SP_MessageHeaderMemAccess *)raw_tx_buffer;
        header_mem_access->device_address = DEVICE_ADDRESS_RECEIVER << 3 | _HYDROLIB_SP_COMMAND_READ;
        header_mem_access->memory_address = 0;
        header_mem_access->memory_access_length = 1;
        raw_tx_buffer[sizeof(_hydrolib_SP_MessageHeaderMemAccess)] =
            CRCfunc(raw_tx_buffer, sizeof(_hydrolib_SP_MessageHeaderMemAccess));

        hydrolib_RingQueue_Init(&txrx_ring_queue, rx_buffer, HYDROLIB_SP_MAX_MESSAGE_LENGTH);

        for (uint8_t i = 0; i < sizeof(test_data); i++)
        {
            test_data[i] = i;
        }
    }

    hydrolib_SerialProtocolHandler transmitter;
    hydrolib_SerialProtocolHandler receiver;

    uint8_t raw_tx_buffer[HYDROLIB_SP_MAX_MESSAGE_LENGTH];
    uint8_t test_data[PUBLIC_MEMORY_LENGTH];
    uint8_t tx_public_memory[PUBLIC_MEMORY_LENGTH];
    uint8_t rx_public_memory[PUBLIC_MEMORY_LENGTH];

    uint8_t rx_buffer[HYDROLIB_SP_MAX_MESSAGE_LENGTH];
    uint8_t tx_buffer[HYDROLIB_SP_MAX_MESSAGE_LENGTH];
};

TEST_F(TestHydrolibSerialProtocol, RawReceivingTest)
{
    for (uint8_t i = 0; i < sizeof(_hydrolib_SP_MessageHeaderMemAccess) + 1; i++)
    {
        hydrolib_RingQueue_PushByte(&txrx_ring_queue, raw_tx_buffer[i]);
    }

    for (uint8_t i = 0; i < HYDROLIB_SP_MAX_MESSAGE_LENGTH; i++)
    {
        hydrolib_SerialProtocol_DoWork(&receiver);
    }
    uint8_t data_length = receiver.current_rx_message_length;
    EXPECT_EQ(sizeof(_hydrolib_SP_MessageHeaderMemAccess) + 1, data_length);
    for (uint8_t i = 0; i < sizeof(_hydrolib_SP_MessageHeaderMemAccess) + 1; i++)
    {
        EXPECT_EQ(raw_tx_buffer[i], receiver.current_rx_message[i]);
    }
}

TEST_F(TestHydrolibSerialProtocol, MemWritingTest)
{
    uint8_t mem_address = 0;
    uint8_t writing_length = 10;
    hydrolib_SerialProtocol_TransmitWrite(&transmitter, DEVICE_ADDRESS_RECEIVER,
                                          mem_address, writing_length, test_data);
    for (uint8_t i = 0; i < HYDROLIB_SP_MAX_MESSAGE_LENGTH; i++)
    {
        hydrolib_SerialProtocol_DoWork(&transmitter);
        hydrolib_SerialProtocol_DoWork(&receiver);
    }
    for (uint8_t i = 0; i < writing_length; i++)
    {
        EXPECT_EQ(test_data[i], rx_public_memory[mem_address + i]);
    }
}

bool ReceiveFunc(uint8_t *byte)
{
    hydrolib_ReturnCode pull_status = hydrolib_RingQueue_PullByte(&txrx_ring_queue, byte);
    return pull_status == HYDROLIB_RETURN_OK;
}

bool TransmitFunc(uint8_t *byte)
{
    hydrolib_ReturnCode push_status = hydrolib_RingQueue_PushByte(&txrx_ring_queue, *byte);
    return push_status == HYDROLIB_RETURN_OK;
}

bool DummyReceiveFunc(uint8_t *byte)
{
    (void)byte;
    return false;
}

bool DummyTransmitFunc(uint8_t *byte)
{
    (void)byte;
    return false;
}

uint8_t CRCfunc(const uint8_t *buffer, uint16_t length)
{
    (void)buffer;
    (void)length;
    return 0xAA;
}

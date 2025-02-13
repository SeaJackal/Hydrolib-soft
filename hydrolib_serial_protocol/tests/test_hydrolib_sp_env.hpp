#ifndef TEST_HYDROLIB_SP_ENV_H_
#define TEST_HYDROLIB_SP_ENV_H_

#include "hydrolib_serial_protocol.h"

#include "hydrolib_ring_queue.h"

#include <string.h>

#include <gtest/gtest.h>

#define PUBLIC_MEMORY_LENGTH 20

#define DEVICE_ADDRESS_RECEIVER 0
#define DEVICE_ADDRESS_TRANSMITTER 1

extern uint8_t txrx_buffer[2 * HYDROLIB_SP_MAX_MESSAGE_LENGTH];
extern uint16_t txrx_length;

bool ReceiveFunc(uint8_t *byte);
bool TransmitFunc(uint8_t *byte);
bool DummyReceiveFunc(uint8_t *byte);
bool DummyTransmitFunc(uint8_t *byte);
uint8_t CRCfunc(const uint8_t *buffer, uint16_t length);

class TestHydrolibSerialProtocol : public ::testing::Test
{
protected:
    TestHydrolibSerialProtocol();

    hydrolib_SerialProtocolHandler transmitter;
    hydrolib_SerialProtocolHandler receiver;

    uint8_t test_data[PUBLIC_MEMORY_LENGTH];
    uint8_t tx_public_memory[PUBLIC_MEMORY_LENGTH];
    uint8_t rx_public_memory[PUBLIC_MEMORY_LENGTH];

    uint8_t rx_buffer[HYDROLIB_SP_MAX_MESSAGE_LENGTH];
    uint8_t tx_buffer[HYDROLIB_SP_MAX_MESSAGE_LENGTH];
};

class TestHydrolibSPmemoryAccess : public TestHydrolibSerialProtocol,
                                   public ::testing::WithParamInterface<std::tuple<uint16_t, uint16_t>>
{
};

#endif

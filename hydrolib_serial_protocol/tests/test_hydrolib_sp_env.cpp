#include "test_hydrolib_sp_env.hpp"

uint8_t txrx_buffer[2 * HYDROLIB_SP_MAX_MESSAGE_LENGTH];
uint16_t txrx_length;

INSTANTIATE_TEST_CASE_P(
    Test,
    TestHydrolibSPmemoryAccess,
    ::testing::Combine(
        ::testing::Range<uint16_t>(0, PUBLIC_MEMORY_LENGTH),
        ::testing::Range<uint16_t>(1, PUBLIC_MEMORY_LENGTH + 1)));

TestHydrolibSerialProtocol::TestHydrolibSerialProtocol()
{
    hydrolib_SerialProtocol_Init(&transmitter, DEVICE_ADDRESS_TRANSMITTER,
                                 TransmitFunc, tx_public_memory, PUBLIC_MEMORY_LENGTH);
    hydrolib_SerialProtocol_Init(&receiver, DEVICE_ADDRESS_RECEIVER,
                                 DummyTransmitFunc, rx_public_memory, PUBLIC_MEMORY_LENGTH);

    txrx_length = 0;

    for (uint8_t i = 0; i < sizeof(test_data); i++)
    {
        test_data[i] = i;
    }
}

bool TransmitFunc(uint8_t *byte)
{
    txrx_buffer[txrx_length] = *byte;
    txrx_length++;
    return txrx_length < sizeof(txrx_buffer);
}

bool DummyTransmitFunc(uint8_t *byte)
{
    (void)byte;
    return false;
}

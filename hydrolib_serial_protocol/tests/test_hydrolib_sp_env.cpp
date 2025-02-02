#include "test_hydrolib_sp_env.hpp"

hydrolib_RingQueue txrx_ring_queue;

INSTANTIATE_TEST_CASE_P(
    Test,
    TestHydrolibSPmemoryAccess,
    ::testing::Combine(
        ::testing::Range<uint16_t>(0, PUBLIC_MEMORY_LENGTH),
        ::testing::Range<uint16_t>(1, PUBLIC_MEMORY_LENGTH + 1)));

TestHydrolibSerialProtocol::TestHydrolibSerialProtocol()
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

    hydrolib_RingQueue_Init(&txrx_ring_queue, rx_buffer, HYDROLIB_SP_MAX_MESSAGE_LENGTH * 2);

    for (uint8_t i = 0; i < sizeof(test_data); i++)
    {
        test_data[i] = i;
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
    uint16_t pol = 0x0700;
    uint16_t crc = buffer[0] << 8;
    for (uint8_t i = 1; i < length; i++)
    {
        crc |= buffer[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1 ^ pol);
            }
            else
            {
                crc = crc << 1;
            }
        }
    }
    return crc >> 8;
}

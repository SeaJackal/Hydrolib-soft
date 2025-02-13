#include "test_hydrolib_sp_env.hpp"

TEST_P(TestHydrolibSPmemoryAccess, MemWritingTest)
{
    auto param = GetParam();
    uint8_t mem_address = std::get<0>(param);
    uint8_t writing_length = std::get<1>(param);
    for (uint8_t j = 0; j < 10; j++)
    {
        hydrolib_ReturnCode transmit_status =
            hydrolib_SerialProtocol_TransmitWrite(&transmitter, DEVICE_ADDRESS_RECEIVER,
                                                  mem_address, writing_length, test_data);
        if (mem_address + writing_length > PUBLIC_MEMORY_LENGTH)
        {
            EXPECT_EQ(HYDROLIB_RETURN_FAIL, transmit_status);
            continue;
        }
        else
        {
            EXPECT_EQ(HYDROLIB_RETURN_OK, transmit_status);
        }

        for (uint8_t i = 0; i < HYDROLIB_SP_MAX_MESSAGE_LENGTH; i++)
        {
            hydrolib_SerialProtocol_DoWork(&transmitter);
        }
        hydrolib_ReturnCode receive_status =
            hydrolib_SerialProtocol_Receive(&receiver, txrx_buffer, txrx_length);
        EXPECT_EQ(HYDROLIB_RETURN_OK, receive_status);
        txrx_length = 0;
        hydrolib_SerialProtocol_DoWork(&receiver);
        for (uint8_t i = 0; i < writing_length; i++)
        {
            EXPECT_EQ(test_data[i], rx_public_memory[mem_address + i]);
        }
    }
}

TEST_P(TestHydrolibSPmemoryAccess, MemWritingWithNoizeTest)
{
    auto param = GetParam();
    uint8_t mem_address = std::get<0>(param);
    uint8_t writing_length = std::get<1>(param);
    for (uint8_t j = 0; j < 10; j++)
    {
        TransmitFunc(&j);
        hydrolib_ReturnCode transmit_status =
            hydrolib_SerialProtocol_TransmitWrite(&transmitter, DEVICE_ADDRESS_RECEIVER,
                                                  mem_address, writing_length, test_data);
        if (mem_address + writing_length > PUBLIC_MEMORY_LENGTH)
        {
            EXPECT_EQ(HYDROLIB_RETURN_FAIL, transmit_status);
            continue;
        }
        else
        {
            EXPECT_EQ(HYDROLIB_RETURN_OK, transmit_status);
        }

        for (uint8_t i = 0; i < HYDROLIB_SP_MAX_MESSAGE_LENGTH; i++)
        {
            hydrolib_SerialProtocol_DoWork(&transmitter);
        }
        TransmitFunc(&j);

        hydrolib_ReturnCode receive_status =
            hydrolib_SerialProtocol_Receive(&receiver, txrx_buffer, txrx_length);
        EXPECT_EQ(HYDROLIB_RETURN_OK, receive_status);
        txrx_length = 0;
        hydrolib_SerialProtocol_DoWork(&receiver);

        for (uint8_t i = 0; i < writing_length; i++)
        {
            EXPECT_EQ(test_data[i], rx_public_memory[mem_address + i]);
        }
    }
}

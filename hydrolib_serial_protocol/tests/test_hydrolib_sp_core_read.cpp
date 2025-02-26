#include "test_hydrolib_sp_core_env.hpp"

using namespace hydrolib::serialProtocol;

namespace test_core
{
    TEST_P(TestHydrolibSPcoreMemoryAccess, MemReadingTest)
    {
        auto param = GetParam();
        uint8_t mem_address = std::get<0>(param);
        uint8_t reading_length = std::get<1>(param);
        uint8_t reading_buffer[PUBLIC_MEMORY_LENGTH];
        for (int i = 0; i < PUBLIC_MEMORY_LENGTH; i++)
        {
            public_memory[i] = i;
        }
        for (uint8_t j = 0; j < 10; j++)
        {
            if (mem_address + reading_length > PUBLIC_MEMORY_LENGTH)
            {
                continue;
            }
            hydrolib_ReturnCode transmit_status = transmitter.TransmitRead(DEVICE_ADDRESS_RECEIVER,
                                                                           mem_address, reading_length, reading_buffer);
            EXPECT_EQ(HYDROLIB_RETURN_OK, transmit_status);

            receiver.ProcessRx();
            transmitter.ProcessRx();
            for (uint8_t i = 0; i < reading_length; i++)
            {
                EXPECT_EQ(public_memory[mem_address + i], reading_buffer[i]);
            }
        }
    }

    // TEST_P(TestHydrolibSPcoreMemoryAccess, MemWritingWithNoizeTest)
    // {
    //     auto param = GetParam();
    //     uint8_t mem_address = std::get<0>(param);
    //     uint8_t reading_length = std::get<1>(param);
    //     for (uint8_t j = 0; j < 10; j++)
    //     {
    //         if (mem_address + reading_length > PUBLIC_MEMORY_LENGTH)
    //         {
    //             continue;
    //         }
    //         txrx_queue.WriteByte(j);
    //         hydrolib_ReturnCode transmit_status =
    //             transmitter.TransmitWrite(DEVICE_ADDRESS_RECEIVER,
    //                                       mem_address, reading_length, test_data);

    //         EXPECT_EQ(HYDROLIB_RETURN_OK, transmit_status);

    //         txrx_queue.WriteByte(j);

    //         receiver.ProcessRx();
    //         for (uint8_t i = 0; i < reading_length; i++)
    //         {
    //             EXPECT_EQ(test_data[i], public_memory[mem_address + i]);
    //         }
    //     }
    // }
}

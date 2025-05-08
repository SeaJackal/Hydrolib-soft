#include "hydrolib_common.h"
#include "hydrolib_serial_protocol_commands.hpp"
#include "hydrolib_serial_protocol_message.hpp"
#include "test_hydrolib_sp_serialize_env.hpp"

using namespace hydrolib::serial_protocol;

namespace test_core
{
TEST_P(TestHydrolibSerialProtocolSerializeParametrized, MemReadingTest)
{
    auto param = GetParam();
    uint8_t mem_address = std::get<0>(param);
    uint8_t reading_length = std::get<1>(param);
    for (uint8_t j = 0; j < 10; j++)
    {
        if (mem_address + reading_length > PUBLIC_MEMORY_LENGTH)
        {
            continue;
        }
        CommandInfo transmitted_info;
        transmitted_info.read = {.source_address = SERIALIZER_ADDRESS,
                                 .dest_address = DESERIALIZER_ADDRESS,
                                 .memory_address = mem_address,
                                 .memory_access_length = reading_length};
        hydrolib_ReturnCode transmit_status =
            serializer.Process(Command::READ, transmitted_info);
        EXPECT_EQ(HYDROLIB_RETURN_OK, transmit_status);

        hydrolib_ReturnCode receive_status = deserializer.Process();
        EXPECT_EQ(HYDROLIB_RETURN_OK, receive_status);

        Command command = deserializer.GetCommand();
        EXPECT_EQ(Command::READ, command);

        CommandInfo received_info = deserializer.GetInfo();
        EXPECT_EQ(SERIALIZER_ADDRESS, received_info.read.source_address);
        EXPECT_EQ(DESERIALIZER_ADDRESS, received_info.read.dest_address);
        EXPECT_EQ(mem_address, received_info.read.memory_address);
        EXPECT_EQ(reading_length, received_info.read.memory_access_length);
    }
}

TEST_P(TestHydrolibSerialProtocolSerializeParametrized, MemReadingWithNoizeTest)
{
    auto param = GetParam();
    uint8_t mem_address = std::get<0>(param);
    uint8_t reading_length = std::get<1>(param);
    for (uint8_t j = 0; j < 10; j++)
    {
        if (mem_address + reading_length > PUBLIC_MEMORY_LENGTH)
        {
            continue;
        }

        stream.WriteByte(j);

        CommandInfo transmitted_info;
        transmitted_info.read = {.source_address = SERIALIZER_ADDRESS,
                                 .dest_address = DESERIALIZER_ADDRESS,
                                 .memory_address = mem_address,
                                 .memory_access_length = reading_length};
        hydrolib_ReturnCode transmit_status =
            serializer.Process(Command::READ, transmitted_info);
        EXPECT_EQ(HYDROLIB_RETURN_OK, transmit_status);

        hydrolib_ReturnCode receive_status;
        for (unsigned i = 0; i < MessageHeader::MAX_MESSAGE_LENGTH; i++)
        {
            receive_status = deserializer.Process();
            if (receive_status != HYDROLIB_RETURN_NO_DATA)
            {
                break;
            }
            stream.WriteByte(j);
        }

        EXPECT_EQ(HYDROLIB_RETURN_OK, receive_status);

        Command command = deserializer.GetCommand();
        EXPECT_EQ(Command::READ, command);

        CommandInfo received_info = deserializer.GetInfo();
        EXPECT_EQ(SERIALIZER_ADDRESS, received_info.read.source_address);
        EXPECT_EQ(DESERIALIZER_ADDRESS, received_info.read.dest_address);
        EXPECT_EQ(mem_address, received_info.read.memory_address);
        EXPECT_EQ(reading_length, received_info.read.memory_access_length);
    }
}

// TEST_P(TestHydrolibSPcoreMemoryAccess, MemWritingWithNoizeTest)
// {
//     auto param = GetParam();
//     uint8_t mem_address = std::get<0>(param);
//     uint8_t writing_length = std::get<1>(param);
//     for (uint8_t j = 0; j < 10; j++)
//     {
//         if (mem_address + writing_length > PUBLIC_MEMORY_LENGTH)
//         {
//             continue;
//         }
//         txrx_queue.WriteByte(j);
//         hydrolib_ReturnCode transmit_status = transmitter.TransmitWrite(
//             DEVICE_ADDRESS_RECEIVER, mem_address, writing_length, test_data);

//         EXPECT_EQ(HYDROLIB_RETURN_OK, transmit_status);

//         for (uint8_t i = 0;
//              i < MessageProcessor<TestRxQueue, TestRxQueue,
//                                   TestPublicMemory>::MAX_MESSAGE_LENGTH;
//              i++)
//         {
//             if (receiver.ProcessRx())
//             {
//                 break;
//             }
//             txrx_queue.WriteByte(j);
//         }
//         for (uint8_t i = 0; i < writing_length; i++)
//         {
//             EXPECT_EQ(test_data[i], public_memory.buffer[mem_address + i]);
//         }
//     }
// }
} // namespace test_core

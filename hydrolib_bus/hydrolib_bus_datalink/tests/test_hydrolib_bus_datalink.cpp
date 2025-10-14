#include "test_hydrolib_bus_datalink.hpp"
#include "hydrolib_bus_datalink_message.hpp"
#include "hydrolib_logger.hpp"
#include <cstdint>
#include <iostream>

TestLogStream log_stream;
hydrolib::logger::LogDistributor distributor("[%s] [%l] %m\n", log_stream);
hydrolib::logger::Logger logger("Serializer", 0, distributor);

int write([[maybe_unused]] TestLogStream &stream, const void *dest,
          unsigned length)
{
    for (unsigned i = 0; i < length; i++)
    {
        std::cout << (reinterpret_cast<const char *>(dest))[i];
    }
    return length;
}

int read(TestStream &stream, void *dest, unsigned length)
{
    uint8_t *byte_buffer = reinterpret_cast<uint8_t *>(dest);
    length = length > stream.queue_.size() ? stream.queue_.size() : length;
    for (uint32_t i = 0; i < length; i++)
    {
        byte_buffer[i] = stream.queue_.front();
        stream.queue_.pop_front();
    }
    return length;
}

int write(TestStream &stream, const void *dest, unsigned length)
{
    for (uint32_t i = 0; i < length; i++)
    {
        stream.queue_.push_back(reinterpret_cast<const uint8_t *>(dest)[i]);
    }
    return length;
}

void TestStream::WriteByte(unsigned byte_index, unsigned byte)
{
    queue_[byte_index] = byte;
    return;
}

unsigned TestStream::ReadByte(unsigned byte_index)
{
    return queue_[byte_index];
}

int TestStream::GetQueueSize() { return queue_.size(); }

TestHydrolibBusDatalink::TestHydrolibBusDatalink()
    : serializer(SERIALIZER_ADDRESS, stream, logger),
      deserializer(DESERIALIZER_ADDRESS, stream, logger),
      tx_stream(serializer, deserializer, DESERIALIZER_ADDRESS),
      rx_stream(serializer, deserializer, SERIALIZER_ADDRESS)
{
    distributor.SetAllFilters(0, hydrolib::logger::LogLevel::DEBUG);
    for (int i = 0; i < PUBLIC_MEMORY_LENGTH; i++)
    {
        // test_data[i] = (hydrolib::bus::datalink::kMagicByte + 14*i) % 0xFF;
        //test_data[i] = i;
        // test_data[i] = 0xFA + i - 15;
        if (i % 3 == 0)
        {
            test_data[i] = hydrolib::bus::datalink::kMagicByte;
        }
        else
        {
            //test_data[i] = (i * i + 10 - i) % 255;
            test_data[i] = i;
        }
    }
}

TEST_F(TestHydrolibBusDatalink, ExchangeTest)
{
    for (unsigned j = 0; j < 20; j++)
    {
        write(tx_stream, test_data, PUBLIC_MEMORY_LENGTH);
        // serializer.Process(DESERIALIZER_ADDRESS, test_data,
        //                    PUBLIC_MEMORY_LENGTH);

        uint8_t buffer[PUBLIC_MEMORY_LENGTH];

        unsigned length = read(rx_stream, buffer, PUBLIC_MEMORY_LENGTH);

        // hydrolib::ReturnCode receive_status = deserializer.Process();
        // EXPECT_EQ(hydrolib::ReturnCode::OK, receive_status);

        // hydrolib::bus::datalink::AddressType address =
        //     deserializer.GetSourceAddress();
        // EXPECT_EQ(address, SERIALIZER_ADDRESS);

        // unsigned length = deserializer.GetDataLength();
        EXPECT_EQ(length, PUBLIC_MEMORY_LENGTH);

        // const uint8_t *data = deserializer.GetData();
        for (unsigned i = 0; i < length; i++)
        {
            EXPECT_EQ(buffer[i], test_data[i]);
        }
    }
}

TEST_F(TestHydrolibBusDatalink, ChangeOneByteTest)
{
    int lost_bytes = 0;

    for (int j = 0; j < 20; j++)
    {
        write(tx_stream, test_data, PUBLIC_MEMORY_LENGTH);

        if (j % 10 == 1)
        {
            stream.WriteByte(sizeof(hydrolib::bus::datalink::MessageHeader) +
                                 j % PUBLIC_MEMORY_LENGTH +
                                 hydrolib::bus::datalink::kCRCLength,
                             (hydrolib::bus::datalink::kMagicByte + j) % 0xFF);
            lost_bytes += stream.GetQueueSize();
        }

        uint8_t buffer[PUBLIC_MEMORY_LENGTH];

        unsigned length = read(rx_stream, buffer, PUBLIC_MEMORY_LENGTH);

        if (j % 10 == 1)
        {
            EXPECT_EQ(length, 0);
        }
        else
        {
            EXPECT_EQ(length, PUBLIC_MEMORY_LENGTH);

            for (unsigned i = 0; i < length; i++)
            {
                EXPECT_EQ(buffer[i], test_data[i]);
            }
        }
    }

    EXPECT_EQ(deserializer.GetLostBytes(), lost_bytes);

    uint8_t buffer[PUBLIC_MEMORY_LENGTH];

    unsigned length = read(rx_stream, buffer, PUBLIC_MEMORY_LENGTH);

    EXPECT_EQ(length, 0);
}

// INSTANTIATE_TEST_CASE_P(
//     Test, TestHydrolibSerialProtocolSerializeParametrized,
//     ::testing::Combine(::testing::Range<uint16_t>(0, PUBLIC_MEMORY_LENGTH),
//                        ::testing::Range<uint16_t>(1,
//                                                   PUBLIC_MEMORY_LENGTH +
//                                                   1)));

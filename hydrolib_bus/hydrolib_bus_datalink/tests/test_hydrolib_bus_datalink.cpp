#include "test_hydrolib_bus_datalink.hpp"

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
    : sender_manager(SERIALIZER_ADDRESS, stream, hydrolib::logger::mock_logger),
      receiver_manager(DESERIALIZER_ADDRESS, stream,
                       hydrolib::logger::mock_logger)
{
    hydrolib::logger::mock_distributor.SetAllFilters(
        0, hydrolib::logger::LogLevel::DEBUG);
    for (int i = 0; i < PUBLIC_MEMORY_LENGTH; i++)
    {
        // test_data[i] = (hydrolib::bus::datalink::kMagicByte + 14*i) % 0xFF;
        // test_data[i] = i;
        // test_data[i] = 0xFA + i - 15;
        if (i % 3 == 0)
        {
            test_data[i] = hydrolib::bus::datalink::kMagicByte;
        }
        else
        {
            // test_data[i] = (i * i + 10 - i) % 255;
            test_data[i] = i;
        }
    }
}

TEST_F(TestHydrolibBusDatalink, ExchangeTest)
{
    hydrolib::bus::datalink::Stream tx_stream(sender_manager,
                                              DESERIALIZER_ADDRESS);
    hydrolib::bus::datalink::Stream rx_stream(receiver_manager,
                                              SERIALIZER_ADDRESS);

    for (unsigned j = 0; j < 500; j++)
    {
        int written_bytes = tx_stream.Write(test_data, PUBLIC_MEMORY_LENGTH);
        EXPECT_EQ(written_bytes, PUBLIC_MEMORY_LENGTH);

        receiver_manager.Process();

        uint8_t buffer[PUBLIC_MEMORY_LENGTH];
        unsigned length = rx_stream.Read(buffer, PUBLIC_MEMORY_LENGTH);

        EXPECT_EQ(length, PUBLIC_MEMORY_LENGTH);

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

TEST_F(TestHydrolibBusDatalink, ChangeMagicBytesTest)
{

    for (int j = 0; j < 20; j++)
    {
        write(tx_stream, test_data, PUBLIC_MEMORY_LENGTH);

        if (j % 10 == 1)
        {
            stream.WriteByte(0, j % (hydrolib::bus::datalink::kMagicByte - 1));
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

    EXPECT_EQ(deserializer.GetLostBytes(), 0);

    uint8_t buffer[PUBLIC_MEMORY_LENGTH];

    unsigned length = read(rx_stream, buffer, PUBLIC_MEMORY_LENGTH);

    EXPECT_EQ(length, 0);
}

TEST_F(TestHydrolibBusDatalink, ChangeDestAddressByteTest)
{

    for (int j = 0; j < 20; j++)
    {
        write(tx_stream, test_data, PUBLIC_MEMORY_LENGTH);

        if (j % 10 == 1)
        {
            uint8_t change_dest_adress = stream.ReadByte(1);
            stream.WriteByte(1,change_dest_adress + j);
            //lost_bytes += stream.GetQueueSize();
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

    EXPECT_EQ(deserializer.GetLostBytes(), 0);

    uint8_t buffer[PUBLIC_MEMORY_LENGTH];

    unsigned length = read(rx_stream, buffer, PUBLIC_MEMORY_LENGTH);

    EXPECT_EQ(length, 0);
}

TEST_F(TestHydrolibBusDatalink, ChangeSrcAddressByteTest)
{
    int lost_bytes = 0;

    for (int j = 0; j < 20; j++)
    {
        write(tx_stream, test_data, PUBLIC_MEMORY_LENGTH);

        if (j % 10 == 1)
        {
            uint8_t change_src_adress = stream.ReadByte(2);
            stream.WriteByte(2,change_src_adress + j);
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
TEST_F(TestHydrolibBusDatalink, ChangeLenghtTest)
{

    for (int j = 0; j < 20; j++)
    {
        write(tx_stream, test_data, PUBLIC_MEMORY_LENGTH);

        if (j % 10 == 1)
        {
            uint8_t change_lenght_adress = stream.ReadByte(3);
            stream.WriteByte(3,change_lenght_adress + j);
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

    EXPECT_EQ(deserializer.GetLostBytes(), 0);

    uint8_t buffer[PUBLIC_MEMORY_LENGTH];

    unsigned length = read(rx_stream, buffer, PUBLIC_MEMORY_LENGTH);

    EXPECT_EQ(length, 0);
}

TEST_F(TestHydrolibBusDatalink, ChangeCobsLenghtByteTest)
{
    int lost_bytes = 0;

    for (int j = 0; j < 20; j++)
    {
        write(tx_stream, test_data, PUBLIC_MEMORY_LENGTH);

        if (j % 10 == 1)
        {
            uint8_t change_cobs_lenght = stream.ReadByte(4);
            stream.WriteByte(4,change_cobs_lenght + j);
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

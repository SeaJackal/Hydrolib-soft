#include "test_hydrolib_bus_datalink.hpp"
#include "hydrolib_logger.hpp"

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

TestHydrolibBusDatalink::TestHydrolibBusDatalink()
    : serializer(SERIALIZER_ADDRESS, stream, logger),
      deserializer(DESERIALIZER_ADDRESS, stream, logger),
      tx_stream(serializer, deserializer, DESERIALIZER_ADDRESS),
      rx_stream(serializer, deserializer, SERIALIZER_ADDRESS)
{
    distributor.SetAllFilters(0, hydrolib::logger::LogLevel::DEBUG);
    for (int i = 0; i < PUBLIC_MEMORY_LENGTH; i++)
    {
        test_data[i] = i;
    }
}

TEST_F(TestHydrolibBusDatalink, ExchangeTest)
{
    for (unsigned j = 0; j < 500; j++)
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

// INSTANTIATE_TEST_CASE_P(
//     Test, TestHydrolibSerialProtocolSerializeParametrized,
//     ::testing::Combine(::testing::Range<uint16_t>(0, PUBLIC_MEMORY_LENGTH),
//                        ::testing::Range<uint16_t>(1,
//                                                   PUBLIC_MEMORY_LENGTH +
//                                                   1)));

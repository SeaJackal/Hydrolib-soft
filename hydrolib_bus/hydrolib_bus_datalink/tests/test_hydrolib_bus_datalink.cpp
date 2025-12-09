#include "test_hydrolib_bus_datalink.hpp"

TestHydrolibBusDatalink::TestHydrolibBusDatalink()
    : sender_manager(SERIALIZER_ADDRESS, stream, hydrolib::logger::mock_logger),
      receiver_manager(DESERIALIZER_ADDRESS, stream,
                       hydrolib::logger::mock_logger),
      tx_stream(sender_manager, DESERIALIZER_ADDRESS),
      rx_stream(receiver_manager, SERIALIZER_ADDRESS)
{
    hydrolib::logger::mock_distributor.SetAllFilters(
        0, hydrolib::logger::LogLevel::DEBUG);
    for (int i = 0; i < kTestDataLength; i++)
    {
        if (i % 3 == 0)
        {
            test_data[i] = hydrolib::bus::datalink::kMagicByte;
        }
        else
        {
            test_data[i] = i;
        }
    }
}

INSTANTIATE_TEST_CASE_P(
    Test, TestHydrolibBusDatalinkParametrized,
    ::testing::Values(0, sizeof(hydrolib::bus::datalink::MessageHeader),
                      sizeof(hydrolib::bus::datalink::MessageHeader) + 2,
                      sizeof(hydrolib::bus::datalink::MessageHeader) + 5,
                      sizeof(hydrolib::bus::datalink::MessageHeader) +
                          TestHydrolibBusDatalink::kTestDataLength - 1),
    [](const testing::TestParamInfo<
        TestHydrolibBusDatalinkParametrized::ParamType> &info)
    {
        if (info.param == 0)
        {
            return std::string("MagicByte");
        }
        else if (static_cast<unsigned>(info.param) >
                 sizeof(hydrolib::bus::datalink::MessageHeader))
        {
            return "Data_" +
                   std::to_string(
                       info.param -
                       sizeof(hydrolib::bus::datalink::MessageHeader));
        }
        else
        {
            return std::string("MessageHeader");
        }
    });

TEST_F(TestHydrolibBusDatalink, ExchangeTest)
{
    for (unsigned j = 0; j < 500; j++)
    {
        int written_bytes = write(tx_stream, test_data, kTestDataLength);
        EXPECT_EQ(written_bytes, kTestDataLength);

        receiver_manager.Process();

        uint8_t buffer[kTestDataLength];
        unsigned length = rx_stream.Read(buffer, kTestDataLength);

        EXPECT_EQ(length, kTestDataLength);

        for (unsigned i = 0; i < length; i++)
        {
            EXPECT_EQ(buffer[i], test_data[i]);
        }
    }
}

TEST_P(TestHydrolibBusDatalinkParametrized, ChangeOneByteTest)
{
    int corrupted_byte_index = GetParam();

    write(tx_stream, test_data, kTestDataLength);

    stream[corrupted_byte_index]++;
    int lost_bytes = static_cast<int>(stream.GetSize());

    receiver_manager.Process();

    uint8_t buffer[kTestDataLength];

    unsigned corrupted_length = read(rx_stream, buffer, kTestDataLength);
    unsigned lost_bytes_after_corrupted_message =
        receiver_manager.GetLostBytes();

    EXPECT_EQ(corrupted_length, 0);
    EXPECT_EQ(lost_bytes_after_corrupted_message, lost_bytes);

    write(tx_stream, test_data, kTestDataLength);
    receiver_manager.Process();

    unsigned valid_length = read(rx_stream, buffer, kTestDataLength);
    unsigned lost_bytes_after_valid_message =
        receiver_manager.GetLostBytes() - lost_bytes_after_corrupted_message;

    EXPECT_EQ(valid_length, kTestDataLength);
    for (unsigned i = 0; i < valid_length; i++)
    {
        EXPECT_EQ(buffer[i], test_data[i]);
    }
    EXPECT_EQ(lost_bytes_after_valid_message, 0);
}

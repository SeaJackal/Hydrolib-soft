#include "test_hydrolib_bus_datalink.hpp"

#include <gtest/gtest.h>

#include <cstddef>

#include "hydrolib_bus_datalink_message.hpp"

TestHydrolibBusDatalink::TestHydrolibBusDatalink()
    : sender_manager(SERIALIZER_ADDRESS, stream, hydrolib::logger::mock_logger),
      receiver_manager(DESERIALIZER_ADDRESS, stream,
                       hydrolib::logger::mock_logger),
      tx_stream(sender_manager, DESERIALIZER_ADDRESS),
      rx_stream(receiver_manager, SERIALIZER_ADDRESS) {
  hydrolib::logger::mock_distributor.SetAllFilters(
      0, hydrolib::logger::LogLevel::DEBUG);
  for (uint8_t i = 0; i < kTestDataLength; i++) {
    if (i % 3 == 0) {
      test_data[i] = hydrolib::bus::datalink::kMagicByte;
    } else {
      test_data[i] = i;
    }
  }
}

TestHydrolibBusDatalinkStreamInterface::
    TestHydrolibBusDatalinkStreamInterface() {
  write(tx_stream, test_data, kTestMessageLength);
  stream.MakeAllbytesAvailable();
  int lost_bytes = receiver_manager.GetLostBytes();
  receiver_manager.Process();
  EXPECT_EQ(lost_bytes, 0);
}

INSTANTIATE_TEST_CASE_P(
    Test, TestHydrolibBusDatalinkParametrized,
    ::testing::Values(0, sizeof(hydrolib::bus::datalink::MessageHeader),
                      sizeof(hydrolib::bus::datalink::MessageHeader) + 2,
                      sizeof(hydrolib::bus::datalink::MessageHeader) + 5,
                      sizeof(hydrolib::bus::datalink::MessageHeader) +
                          TestHydrolibBusDatalink::kTestDataLength - 1),
    [](const testing::TestParamInfo<
        TestHydrolibBusDatalinkParametrized::ParamType> &info) {
      if (info.param == 0) {
        return std::string("MagicByte");
      } else if (static_cast<unsigned>(info.param) >
                 sizeof(hydrolib::bus::datalink::MessageHeader)) {
        return "Data_" +
               std::to_string(info.param -
                              sizeof(hydrolib::bus::datalink::MessageHeader));
      } else {
        return std::string("MessageHeader");
      }
    });

TEST_F(TestHydrolibBusDatalink, MessageLengthTest) {
  int written_bytes = write(tx_stream, test_data, kTestMessageLength);

  EXPECT_EQ(written_bytes, kTestMessageLength);
  EXPECT_EQ(stream.GetSize(),
            kTestDataLength + sizeof(hydrolib::bus::datalink::MessageHeader) +
                hydrolib::bus::datalink::kCRCLength);
}

TEST_F(TestHydrolibBusDatalink, ExchangeTest) {
  struct TestCase {
    int length;
    int offset;
  };
  TestCase test_cases[] = {{5, 0}, {10, 1}, {2, 7},  {20, 0}, {3, 2},  {4, 3},
                           {5, 4}, {17, 1}, {15, 2}, {14, 1}, {13, 0}, {1, 3}};
  for (unsigned j = 0; j < sizeof(test_cases) / sizeof(test_cases[0]); j++) {
    ASSERT_LE(test_cases[j].length + test_cases[j].offset, kTestDataLength);

    int written_bytes = write(tx_stream, test_data + test_cases[j].offset,
                              test_cases[j].length);
    stream.MakeAllbytesAvailable();
    EXPECT_EQ(written_bytes, test_cases[j].length);

    receiver_manager.Process();

    uint8_t buffer[kTestDataLength];
    unsigned length = read(rx_stream, buffer, test_cases[j].length);

    EXPECT_EQ(length, test_cases[j].length);

    for (unsigned i = 0; i < length; i++) {
      EXPECT_EQ(buffer[i], test_data[i + test_cases[j].offset]);
    }
  }
}

TEST_P(TestHydrolibBusDatalinkParametrized, ChangeOneByteTest) {
  int corrupted_byte_index = GetParam();

  write(tx_stream, test_data, kTestMessageLength);
  stream.MakeAllbytesAvailable();

  stream[corrupted_byte_index]++;
  int lost_bytes = static_cast<int>(stream.GetSize());

  receiver_manager.Process();

  uint8_t buffer[kTestDataLength];

  unsigned corrupted_length = read(rx_stream, buffer, kTestMessageLength);
  unsigned lost_bytes_after_corrupted_message = receiver_manager.GetLostBytes();

  EXPECT_EQ(corrupted_length, 0);
  EXPECT_EQ(lost_bytes_after_corrupted_message, lost_bytes);

  write(tx_stream, test_data, kTestMessageLength);
  stream.MakeAllbytesAvailable();
  receiver_manager.Process();

  unsigned valid_length = read(rx_stream, buffer, kTestMessageLength);
  unsigned lost_bytes_after_valid_message =
      receiver_manager.GetLostBytes() - lost_bytes_after_corrupted_message;

  EXPECT_EQ(valid_length, kTestMessageLength);
  for (unsigned i = 0; i < valid_length; i++) {
    EXPECT_EQ(buffer[i], test_data[i]);
  }
  EXPECT_EQ(lost_bytes_after_valid_message, 0);
}

TEST_F(TestHydrolibBusDatalink, ChangeLengthTest) {
  constexpr uint8_t kTestByte = 0xAA;

  write(tx_stream, &kTestByte, sizeof(kTestByte));
  stream.MakeAllbytesAvailable();

  stream[offsetof(hydrolib::bus::datalink::MessageHeader, length)] =
      hydrolib::bus::datalink::kMaxMessageLength;
  int small_message_length = static_cast<int>(stream.GetSize());
  int lost_bytes = small_message_length;

  receiver_manager.Process();

  uint8_t buffer[kTestDataLength];

  unsigned corrupted_length = read(rx_stream, buffer, kTestMessageLength);

  EXPECT_EQ(corrupted_length, 0);

  int remaining_bytes = hydrolib::bus::datalink::kMaxMessageLength -
                        sizeof(kTestByte) - hydrolib::bus::datalink::kCRCLength;

  while (remaining_bytes - small_message_length > 0) {
    write(tx_stream, &kTestByte, sizeof(kTestByte));
    stream.MakeAllbytesAvailable();

    lost_bytes += small_message_length;
    remaining_bytes -= small_message_length;

    receiver_manager.Process();
    corrupted_length = read(rx_stream, buffer, kTestMessageLength);
    EXPECT_EQ(corrupted_length, 0);
  }

  write(tx_stream, test_data, kTestMessageLength);
  stream.MakeAllbytesAvailable();
  receiver_manager.Process();

  unsigned valid_length = read(rx_stream, buffer, kTestMessageLength);
  unsigned real_lost_bytes = receiver_manager.GetLostBytes();

  EXPECT_EQ(valid_length, kTestMessageLength);
  for (unsigned i = 0; i < valid_length; i++) {
    EXPECT_EQ(buffer[i], test_data[i]);
  }
  EXPECT_EQ(real_lost_bytes, lost_bytes);
}

TEST_F(TestHydrolibBusDatalink, ProgressiveTransmissionTest) {
  int written_bytes = write(tx_stream, test_data, kTestMessageLength);
  EXPECT_EQ(written_bytes, kTestMessageLength);

  uint8_t buffer[kTestMessageLength];
  int bytes_to_read = static_cast<int>(stream.GetSize());
  for (int i = 0; i < bytes_to_read; i++) {
    receiver_manager.Process();
    unsigned length = read(rx_stream, buffer, kTestMessageLength);
    EXPECT_EQ(length, 0);
    stream.AddAvailableBytes(1);
  }

  receiver_manager.Process();
  unsigned length = read(rx_stream, buffer, kTestMessageLength);
  EXPECT_EQ(length, kTestMessageLength);

  for (unsigned i = 0; i < length; i++) {
    EXPECT_EQ(buffer[i], test_data[i]);
  }
}

TEST_F(TestHydrolibBusDatalinkStreamInterface, ReadSome) {
  uint8_t buffer[kTestMessageLength] = {};
  int read_numbers[] = {2, 1, 5, 15, 7, 12};
  int sum = 0;
  for (int i = 0; i < sizeof(read_numbers) / sizeof(read_numbers[0]); i++) {
    sum += read_numbers[i];
  }
  EXPECT_LE(sum, kTestMessageLength);
  int read_count = 0;
  for (int i = 0; i < sizeof(read_numbers) / sizeof(read_numbers[0]); i++) {
    int length = read(rx_stream, buffer, read_numbers[i]);
    EXPECT_EQ(length, read_numbers[i]);
    for (int j = 0; j < read_numbers[i]; j++) {
      EXPECT_EQ(buffer[j], test_data[read_count + j]);
    }
    read_count += length;
  }
}

TEST_F(TestHydrolibBusDatalinkStreamInterface, ReadMoreThanAvailable) {
  uint8_t buffer[kTestMessageLength] = {};
  int length = read(rx_stream, &buffer, kTestMessageLength + 1);
  EXPECT_EQ(length, kTestMessageLength);
  for (int i = 0; i < kTestMessageLength; i++) {
    EXPECT_EQ(buffer[i], test_data[i]);
  }
}

TEST_F(TestHydrolibBusDatalinkStreamInterface, ReadFull) {
  uint8_t buffer[kTestMessageLength] = {};
  int length = read(rx_stream, buffer, kTestMessageLength);
  EXPECT_EQ(length, kTestMessageLength);
  for (int i = 0; i < kTestMessageLength; i++) {
    EXPECT_EQ(buffer[i], test_data[i]);
  }
}

#include <gtest/gtest.h>

#include <functional>
#include <random>

#include "hydrolib_bus_datalink_deserializer.hpp"
#include "hydrolib_bus_datalink_serializer.hpp"
#include "hydrolib_logger_mock.hpp"
#include "mock_stream.hpp"

namespace {
class SerializeDeserialize : public ::testing::Test {
 public:
  static constexpr hydrolib::bus::datalink::AddressType kSerializerAddress =
      std::byte(3);
  static constexpr hydrolib::bus::datalink::AddressType kDeserializerAddress =
      std::byte(4);

 protected:
  hydrolib::streams::mock::MockByteStream stream;

  hydrolib::bus::datalink::Serializer<hydrolib::streams::mock::MockByteStream,
                                      decltype(hydrolib::logger::mock_logger)>
      serializer{kSerializerAddress, stream, hydrolib::logger::mock_logger};
  hydrolib::bus::datalink::Deserializer<hydrolib::streams::mock::MockByteStream,
                                        decltype(hydrolib::logger::mock_logger)>
      deserializer{kDeserializerAddress, stream, hydrolib::logger::mock_logger};

  void SimpleExchange(const std::vector<std::byte>& data);
};

class SerializeDeserializeOneMessage
    : public SerializeDeserialize,
      public ::testing::WithParamInterface<std::vector<std::byte>> {};

class SerializeDeserializeTwoMessages
    : public SerializeDeserialize,
      public ::testing::WithParamInterface<
          std::tuple<std::vector<std::byte>, std::vector<std::byte>>> {};

class SerializeDeserializeThreeMessages
    : public SerializeDeserialize,
      public ::testing::WithParamInterface<
          std::tuple<std::vector<std::byte>, std::vector<std::byte>,
                     std::vector<std::byte>>> {};

class SerializeDeserializeProgressive
    : public SerializeDeserialize,
      public ::testing::WithParamInterface<
          std::tuple<std::vector<std::byte>, std::function<int()>>> {};

std::vector<std::byte> GenerateRandomData(int length, int seed) {
  std::mt19937 gen(seed);
  std::uniform_int_distribution<int> dist(0, UINT8_MAX);
  std::vector<std::byte> data(length);
  std::ranges::generate(data, [&gen, &dist]() { return std::byte(dist(gen)); });
  return data;
}

std::vector<std::byte> GenerateRandomDataWithMagicBytes(int length, int seed,
                                                        int magic_byte_seed) {
  std::mt19937 gen(seed);
  std::uniform_int_distribution<int> dist(0, length);
  auto data = GenerateRandomData(length, magic_byte_seed);
  for (int i = 0; i != length / 2; i++) {
    data[dist(gen)] = hydrolib::bus::datalink::kMagicByte;
  }
  return data;
}

std::vector<std::byte> GenerateOnlyMagicBytes(int length) {
  std::vector<std::byte> data(length);
  std::ranges::fill(data, hydrolib::bus::datalink::kMagicByte);
  return data;
}

const std::array test_cases = {
    GenerateRandomData(10, 0),
    GenerateRandomData(7, 15),
    GenerateRandomData(1, 0),
    GenerateRandomData(hydrolib::bus::datalink::kMaxDataLength, 3),
    GenerateRandomData(hydrolib::bus::datalink::kMaxDataLength, 124),
    GenerateRandomData(157, 10),
    GenerateRandomDataWithMagicBytes(10, 0, 0),
    GenerateRandomDataWithMagicBytes(hydrolib::bus::datalink::kMaxDataLength, 3,
                                     4),
    GenerateRandomDataWithMagicBytes(157, 10, 5),
    GenerateOnlyMagicBytes(1),
    GenerateOnlyMagicBytes(hydrolib::bus::datalink::kMaxDataLength),
    GenerateOnlyMagicBytes(123)};

void SerializeDeserialize::SimpleExchange(const std::vector<std::byte>& data) {
  serializer.Process(kDeserializerAddress, data);
  stream.MakeAllbytesAvailable();
  auto result = deserializer.Process();

  ASSERT_EQ(static_cast<hydrolib::ReturnCode>(result),
            hydrolib::ReturnCode::OK);

  auto message = static_cast<decltype(deserializer)::CurrentMessageInfo>(std::move(result));
  auto message_data = static_cast<std::span<std::byte>>(message.data);
  EXPECT_EQ(message.src_address, kSerializerAddress);
  ASSERT_EQ(message_data.size(), data.size());
  for (int i = 0; i != static_cast<int>(message_data.size()); ++i) {
    EXPECT_EQ(message_data[i], data[i]);
  }
}
}  // namespace

INSTANTIATE_TEST_CASE_P(Test, SerializeDeserializeOneMessage,
                        ::testing::ValuesIn(test_cases));

INSTANTIATE_TEST_CASE_P(Test, SerializeDeserializeTwoMessages,
                        ::testing::Combine(::testing::ValuesIn(test_cases),
                                           ::testing::ValuesIn(test_cases)));

INSTANTIATE_TEST_CASE_P(
    Test, SerializeDeserializeThreeMessages,
    ::testing::Combine(::testing::Values(test_cases[0], test_cases[1],
                                         test_cases[2]),
                       ::testing::Values(test_cases[0], test_cases[11]),
                       ::testing::Values(test_cases[11], test_cases[7])));

INSTANTIATE_TEST_CASE_P(
    Test, SerializeDeserializeProgressive,
    ::testing::Combine(
        ::testing::ValuesIn(test_cases),
        ::testing::Values(
            []() { return 1; }, []() { return 5; },
            [gen = std::mt19937(0),
             dist = std::uniform_int_distribution<int>(1, 5)]() mutable {
              return dist(gen);
            },
            [gen = std::mt19937(1),
             dist = std::uniform_int_distribution<int>(1, 3)]() mutable {
              return dist(gen);
            })));

TEST_P(SerializeDeserializeOneMessage, LengthTest) {
  const auto& data = GetParam();
  serializer.Process(kDeserializerAddress, data);
  EXPECT_EQ(stream.GetSize(),
            data.size() + sizeof(hydrolib::bus::datalink::kMagicByte) +
                sizeof(hydrolib::bus::datalink::MessageHeader) +
                hydrolib::bus::datalink::kCRCLength);
}

TEST_P(SerializeDeserializeOneMessage, SimpleExchange) {
  const auto& data = GetParam();
  SimpleExchange(data);
}

TEST_P(SerializeDeserializeTwoMessages, SimpleExchange) {
  const auto& param = GetParam();

  std::array datas = {std::get<0>(param), std::get<1>(param)};

  for (const auto& data : datas) {
    SimpleExchange(data);
  }
}

TEST_P(SerializeDeserializeThreeMessages, SimpleExchange) {
  const auto& param = GetParam();

  std::array datas = {std::get<0>(param), std::get<1>(param),
                      std::get<2>(param)};

  for (const auto& data : datas) {
    SimpleExchange(data);
  }
}

TEST_P(SerializeDeserializeProgressive, ProgressiveExchange) {
  auto param = GetParam();
  const auto& data = std::get<0>(param);
  const auto& func = std::get<1>(param);
  serializer.Process(kDeserializerAddress, data);

  auto available_bytes = stream.GetSize();

  int sum = 0;
  while (sum < static_cast<int>(available_bytes)) {
    auto result = deserializer.Process();
    ASSERT_EQ(static_cast<hydrolib::ReturnCode>(result),
              hydrolib::ReturnCode::NO_DATA);
    auto available_bytes = func();
    stream.AddAvailableBytes(available_bytes);
    sum += available_bytes;
  }

  auto result = deserializer.Process();
  ASSERT_EQ(static_cast<hydrolib::ReturnCode>(result),
            hydrolib::ReturnCode::OK);

  auto message = static_cast<decltype(deserializer)::CurrentMessageInfo>(std::move(result));
  auto message_data = static_cast<std::span<std::byte>>(message.data);
  EXPECT_EQ(message.src_address, kSerializerAddress);
  ASSERT_EQ(message_data.size(), data.size());
  for (int i = 0; i != static_cast<int>(message_data.size()); ++i) {
    EXPECT_EQ(message_data[i], data[i]);
  }
}

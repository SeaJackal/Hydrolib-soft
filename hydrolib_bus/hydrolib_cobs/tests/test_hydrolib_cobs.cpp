#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "hydrolib_cobs.hpp"

namespace {
class TestCOBS : public ::testing::Test,
                 public ::testing::WithParamInterface<std::vector<uint8_t>> {
 public:
  static constexpr uint8_t kMagicByte = 0xAA;

  static std::vector<uint8_t> GenerateOnlyMagicBytes(int number) {
    std::vector<uint8_t> result(number);
    for (auto& elem : result) {
      elem = kMagicByte;
    }
    return result;
  }

  static std::vector<uint8_t> GenerateWithoutMagicBytes(int number, int base) {
    std::vector<uint8_t> result(number);
    for (int i = 0; i != number; ++i) {
      result[i] = (i + base) % UINT8_MAX;
    }
    if (number > kMagicByte) {
      result[kMagicByte] = 0;
    }
    return result;
  }

  static std::vector<uint8_t> GenerateWithMagicBytes(int number,
                                                     int magic_byte_period) {
    std::vector<uint8_t> result(number);
    for (int i = 0; i != number; ++i) {
      result[i] = i;
    }
    for (int i = 1; i < number; i += magic_byte_period) {
      result[i] = kMagicByte;
    }
    return result;
  }

  static inline std::vector<std::vector<uint8_t>> cases{
      {1, 2, 3, kMagicByte, 4, 5, kMagicByte},
      {0, kMagicByte, 31, kMagicByte, 90, kMagicByte},
      {0, 42, kMagicByte, kMagicByte, 0},
      GenerateOnlyMagicBytes(13),
      GenerateOnlyMagicBytes(1),
      GenerateOnlyMagicBytes(UINT8_MAX),
      GenerateWithoutMagicBytes(17, 0),
      GenerateWithoutMagicBytes(14, 5),
      GenerateWithoutMagicBytes(1, 7),
      GenerateWithoutMagicBytes(UINT8_MAX, 0),
      GenerateWithMagicBytes(UINT8_MAX, 50),
      GenerateWithMagicBytes(50, 7),
      GenerateWithMagicBytes(12, 2),
      GenerateWithMagicBytes(33, 5)};
};
}  // namespace

INSTANTIATE_TEST_CASE_P(Test, TestCOBS, ::testing::ValuesIn(TestCOBS::cases));

TEST_P(TestCOBS, EncodeDecode) {
  const auto& data = GetParam();

  auto serialized_data = data;
  int encoded_length = hydrolib::cobs::Encode<std::byte(kMagicByte)>(
      std::as_writable_bytes(std::span(serialized_data)));
  EXPECT_TRUE(std::ranges::find(serialized_data, kMagicByte) ==
              serialized_data.end());

  auto result = hydrolib::cobs::Decode<std::byte(kMagicByte)>(
      encoded_length, std::as_writable_bytes(std::span(serialized_data)));
  EXPECT_EQ(result, hydrolib::ReturnCode::OK);
  EXPECT_EQ(serialized_data, data);
}

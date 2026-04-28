#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

#include "hydrolib_crc.hpp"

namespace {
using CRC8TestCase = std::tuple<std::vector<uint8_t>, std::byte>;

class TestHydrolibCRC8 : public ::testing::TestWithParam<CRC8TestCase> {};
}  // namespace

TEST_P(TestHydrolibCRC8, CountCRC8MatchesExpected) {
  const auto& param = GetParam();
  const auto& data = std::get<0>(param);
  const auto& expected_crc8 = std::get<1>(param);
  ASSERT_FALSE(data.empty());
  hydrolib::crc::CRC8 crc8;
  for (auto byte : data) {
    crc8.Next(static_cast<std::byte>(byte));
  }
  const auto actual = crc8.Get();
  EXPECT_EQ(actual, expected_crc8);
}

INSTANTIATE_TEST_SUITE_P(
    Test, TestHydrolibCRC8,
    ::testing::Values(CRC8TestCase{{0x00}, std::byte(0x00)},
                      CRC8TestCase{{0x01}, std::byte(0x07)},
                      CRC8TestCase{{0xFF}, std::byte(0xF3)},
                      CRC8TestCase{{0x00, 0x00}, std::byte(0x00)},
                      CRC8TestCase{{0x00, 0x01}, std::byte(0x07)},
                      CRC8TestCase{{0x00, 0x80}, std::byte(0x89)},
                      CRC8TestCase{{0x01, 0x00}, std::byte(0x15)},
                      CRC8TestCase{{0x01, 0x01}, std::byte(0x12)},
                      CRC8TestCase{{0x02, 0x23}, std::byte(0xC3)},
                      CRC8TestCase{{0x13, 0x24, 0x00, 0xF3, 0x0F, 0x41, 0xFF,
                                    0x01, 0x56, 0x99, 0x00, 0x14},
                                   std::byte(0x55)}));

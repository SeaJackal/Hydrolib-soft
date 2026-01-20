#include "test_hydrolib_bus_application.hpp"

#include <gtest/gtest.h>

#include <cstring>
#include <thread>

using namespace std::literals::chrono_literals;

constexpr std::array<TestCase, 10> kTestCases = {
    TestCase{.address = 0, .length = 1},
    TestCase{.address = 1, .length = 5},
    TestCase{.address = TestPublicMemory::kPublicMemoryLength - 1, .length = 1},
    TestCase{.address = TestPublicMemory::kPublicMemoryLength - 3, .length = 3},
    TestCase{.address = 0, .length = TestPublicMemory::kPublicMemoryLength},
    TestCase{.address = 2, .length = TestPublicMemory::kPublicMemoryLength - 2},
    TestCase{.address = 6, .length = 3},
    TestCase{.address = 5, .length = 5},
    TestCase{.address = 1, .length = 9},
    TestCase{.address = 9, .length = 5}};

INSTANTIATE_TEST_CASE_P(Test, TestHydrolibBusApplication,
                        ::testing::ValuesIn(kTestCases));

TestHydrolibBusApplication::TestHydrolibBusApplication()
    : master(stream, hydrolib::logger::mock_logger),
      slave(stream, memory, hydrolib::logger::mock_logger) {
  hydrolib::logger::mock_distributor.SetAllFilters(
      0, hydrolib::logger::LogLevel::DEBUG);
  for (int i = 0; i < TestPublicMemory::kPublicMemoryLength; i++) {
    test_data[i] = i;
  }
}

hydrolib::ReturnCode TestPublicMemory::Read(void *read_buffer, unsigned address,
                                            unsigned length) {
  if (address + length > kPublicMemoryLength) {
    return hydrolib::ReturnCode::FAIL;
  }
  memcpy(read_buffer, &memory[address], length);
  return hydrolib::ReturnCode::OK;
}

hydrolib::ReturnCode TestPublicMemory::Write(const void *write_buffer,
                                             unsigned address,
                                             unsigned length) {
  if (address + length > kPublicMemoryLength) {
    return hydrolib::ReturnCode::FAIL;
  }
  memcpy(&memory[address], write_buffer, length);
  return hydrolib::ReturnCode::OK;
}

TEST_P(TestHydrolibBusApplication, WriteTest) {
  auto test_case = GetParam();
  std::array<uint8_t, TestPublicMemory::kPublicMemoryLength> reference_data =
      memory.memory;
  ASSERT_LE(test_case.address + test_case.length,
            TestPublicMemory::kPublicMemoryLength);
  master.Write(test_data.data(), test_case.address, test_case.length);
  memcpy(reference_data.data() + test_case.address, test_data.data(),
         test_case.length);
  stream.MakeAllbytesAvailable();
  slave.Process();
  for (int i = 0; i < TestPublicMemory::kPublicMemoryLength; i++) {
    EXPECT_EQ(reference_data[i], memory.memory[i]);
  }
}

TEST_F(TestHydrolibBusApplication, WriteSeriaTest) {
  std::array<uint8_t, TestPublicMemory::kPublicMemoryLength> reference_data =
      memory.memory;
  for (const auto &test_case : kTestCases) {
    ASSERT_LE(test_case.address + test_case.length,
              TestPublicMemory::kPublicMemoryLength);
    master.Write(test_data.data(), test_case.address, test_case.length);
    memcpy(reference_data.data() + test_case.address, test_data.data(),
           test_case.length);
    stream.MakeAllbytesAvailable();
    slave.Process();
    for (int i = 0; i < TestPublicMemory::kPublicMemoryLength; i++) {
      EXPECT_EQ(reference_data[i], memory.memory[i]);
    }
  }
}

TEST_P(TestHydrolibBusApplication, ReadTest) {
  auto test_case = GetParam();
  std::array<uint8_t, TestPublicMemory::kPublicMemoryLength> buffer{};
  ASSERT_LE(test_case.address + test_case.length,
            TestPublicMemory::kPublicMemoryLength);
  memcpy(memory.memory.data(), test_data.data(),
         TestPublicMemory::kPublicMemoryLength);
  master.Read(buffer.data(), test_case.address, test_case.length);
  stream.MakeAllbytesAvailable();
  slave.Process();
  stream.MakeAllbytesAvailable();
  master.Process();
  for (int i = 0; i < test_case.length; i++) {
    EXPECT_EQ(buffer[i], memory.memory[test_case.address + i]);
  }
  for (int i = 0; i < TestPublicMemory::kPublicMemoryLength; i++) {
    EXPECT_EQ(test_data[i], memory.memory[i]);
  }
}

TEST_F(TestHydrolibBusApplication, ReadSeriaTest) {
  memcpy(memory.memory.data(), test_data.data(),
         TestPublicMemory::kPublicMemoryLength);
  for (const auto &test_case : kTestCases) {
    std::array<uint8_t, TestPublicMemory::kPublicMemoryLength> buffer{};
    ASSERT_LE(test_case.address + test_case.length,
              TestPublicMemory::kPublicMemoryLength);
    master.Read(buffer.data(), test_case.address, test_case.length);
    stream.MakeAllbytesAvailable();
    slave.Process();
    stream.MakeAllbytesAvailable();
    master.Process();
    for (int i = 0; i < test_case.length; i++) {
      EXPECT_EQ(buffer[i], memory.memory[test_case.address + i]);
    }
    for (int i = 0; i < TestPublicMemory::kPublicMemoryLength; i++) {
      EXPECT_EQ(test_data[i], memory.memory[i]);
    }
  }
}

TEST_F(TestHydrolibBusApplication, ReadTimeoutTest) {
  auto test_case = kTestCases[0];
  std::array<uint8_t, TestPublicMemory::kPublicMemoryLength> buffer{};
  ASSERT_LE(test_case.address + test_case.length,
            TestPublicMemory::kPublicMemoryLength);
  memcpy(memory.memory.data(), test_data.data(),
         TestPublicMemory::kPublicMemoryLength);
  auto start_time = std::chrono::steady_clock::now();
  master.Read(buffer.data(), test_case.address, test_case.length);
  while (std::chrono::steady_clock::now() - start_time <
         decltype(master)::kRequestTimeout) {
  }
  stream.Clear();
  EXPECT_EQ(hydrolib::ReturnCode::TIMEOUT, master.Process());
  stream.MakeAllbytesAvailable();
  slave.Process();
  stream.MakeAllbytesAvailable();
  EXPECT_EQ(hydrolib::ReturnCode::OK, master.Process());
  for (int i = 0; i < test_case.length; i++) {
    EXPECT_EQ(buffer[i], memory.memory[test_case.address + i]);
  }
}

TEST_F(TestHydrolibBusApplication, ReadAlmostTimeoutTest) {
  auto test_case = kTestCases[0];
  std::array<uint8_t, TestPublicMemory::kPublicMemoryLength> buffer{};
  ASSERT_LE(test_case.address + test_case.length,
            TestPublicMemory::kPublicMemoryLength);
  memcpy(memory.memory.data(), test_data.data(),
         TestPublicMemory::kPublicMemoryLength);
  auto start_time = std::chrono::steady_clock::now();
  master.Read(buffer.data(), test_case.address, test_case.length);
  while (std::chrono::steady_clock::now() - start_time <
         decltype(master)::kRequestTimeout - 10ms) {
  }
  stream.MakeAllbytesAvailable();
  slave.Process();
  stream.MakeAllbytesAvailable();
  EXPECT_EQ(hydrolib::ReturnCode::OK, master.Process());
  for (int i = 0; i < test_case.length; i++) {
    EXPECT_EQ(buffer[i], memory.memory[test_case.address + i]);
  }
}

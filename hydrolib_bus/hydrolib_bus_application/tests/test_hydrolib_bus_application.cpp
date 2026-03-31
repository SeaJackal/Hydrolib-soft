#include "test_hydrolib_bus_application.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstring>
#include <span>

#include "hydrolib_bus_application_commands.hpp"
#include "hydrolib_bus_application_master.hpp"

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
    test_data[i] = static_cast<std::byte>(i);
  }
}

hydrolib::ReturnCode TestPublicMemory::Read(std::span<std::byte> read_buffer,
                                            int address) {
  if (address + read_buffer.size() > kPublicMemoryLength) {
    return hydrolib::ReturnCode::FAIL;
  }
  std::ranges::copy(
      std::span<std::byte>(memory).subspan(address, read_buffer.size()),
      read_buffer.begin());
  return hydrolib::ReturnCode::OK;
}

hydrolib::ReturnCode TestPublicMemory::Write(
    std::span<const std::byte> write_buffer, int address) {
  if (address + write_buffer.size() > kPublicMemoryLength) {
    return hydrolib::ReturnCode::FAIL;
  }
  std::ranges::copy(write_buffer,
                    std::span<std::byte>(memory).subspan(address).begin());
  return hydrolib::ReturnCode::OK;
}

TEST_P(TestHydrolibBusApplication, WriteTest) {
  auto test_case = GetParam();
  std::array<std::byte, TestPublicMemory::kPublicMemoryLength> reference_data =
      memory.memory;
  ASSERT_LE(test_case.address + test_case.length,
            TestPublicMemory::kPublicMemoryLength);
  master.RequestWrite(std::span<std::byte>(test_data.begin(), test_case.length),
                      test_case.address);
  std::ranges::copy(
      std::span<std::byte>(test_data.begin(), test_case.length),
      std::span<std::byte>(reference_data).subspan(test_case.address).begin());
  stream.MakeAllbytesAvailable();
  slave.Process();
  for (int i = 0; i < TestPublicMemory::kPublicMemoryLength; i++) {
    EXPECT_EQ(reference_data[i], memory.memory[i]);
  }
}

TEST_F(TestHydrolibBusApplication, WriteSeriaTest) {
  std::array<std::byte, TestPublicMemory::kPublicMemoryLength> reference_data =
      memory.memory;
  for (const auto& test_case : kTestCases) {
    ASSERT_LE(test_case.address + test_case.length,
              TestPublicMemory::kPublicMemoryLength);
    master.RequestWrite(
        std::span<std::byte>(test_data.begin(), test_case.length),
        test_case.address);
    std::ranges::copy(std::span<std::byte>(test_data.begin(), test_case.length),
                      std::span<std::byte>(reference_data)
                          .subspan(test_case.address)
                          .begin());
    stream.MakeAllbytesAvailable();
    slave.Process();
    for (int i = 0; i < TestPublicMemory::kPublicMemoryLength; i++) {
      EXPECT_EQ(reference_data[i], memory.memory[i]);
    }
  }
}

TEST_P(TestHydrolibBusApplication, ReadTest) {
  auto test_case = GetParam();
  std::array<std::byte, TestPublicMemory::kPublicMemoryLength> buffer{};
  ASSERT_LE(test_case.address + test_case.length,
            TestPublicMemory::kPublicMemoryLength);
  std::ranges::copy(test_data, memory.memory.begin());
  master.RequestRead(std::span<std::byte>(buffer.begin(), test_case.length),
                     test_case.address);
  EXPECT_EQ(master.Process(), hydrolib::ReturnCode::NO_DATA);
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
  for (const auto& test_case : kTestCases) {
    std::array<std::byte, TestPublicMemory::kPublicMemoryLength> buffer{};
    ASSERT_LE(test_case.address + test_case.length,
              TestPublicMemory::kPublicMemoryLength);
    master.RequestRead(std::span<std::byte>(buffer.begin(), test_case.length),
                       test_case.address);
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
  std::array<std::byte, TestPublicMemory::kPublicMemoryLength> buffer{};
  ASSERT_LE(test_case.address + test_case.length,
            TestPublicMemory::kPublicMemoryLength);
  std::ranges::copy(test_data, memory.memory.begin());
  auto start_time = std::chrono::steady_clock::now();
  master.RequestRead(buffer, test_case.address);
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
  std::array<std::byte, TestPublicMemory::kPublicMemoryLength> buffer{};
  ASSERT_LE(test_case.address + test_case.length,
            TestPublicMemory::kPublicMemoryLength);
  std::ranges::copy(test_data, memory.memory.begin());
  auto start_time = std::chrono::steady_clock::now();
  master.RequestRead(buffer, test_case.address);
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

TEST_F(TestHydrolibBusApplication,
       MemoryAccessReadError) {  // TODO: vscode - modernize mock
                                 // https://app.weeek.net/ws/701833/task/1065
  int address = TestPublicMemory::kPublicMemoryLength;
  int length = 1;

  std::array<std::byte, TestPublicMemory::kPublicMemoryLength> buffer{};

  master.RequestRead(std::span<std::byte>(buffer.begin(), length), address);
  stream.MakeAllbytesAvailable();
  slave.Process();

  stream.MakeAllbytesAvailable();
  EXPECT_EQ(master.Process(), hydrolib::ReturnCode::ERROR);
}

TEST_F(TestHydrolibBusApplication, MemoryAccessWriteError) {
  int address = TestPublicMemory::kPublicMemoryLength;
  int length = 1;

  std::array<std::byte, TestPublicMemory::kPublicMemoryLength> buffer{};

  master.RequestWrite(std::span<std::byte>(buffer.begin(), length), address);
  stream.MakeAllbytesAvailable();
  slave.Process();

  stream.MakeAllbytesAvailable();
  EXPECT_EQ(master.Process(), hydrolib::ReturnCode::FAIL);
}

TEST_F(TestHydrolibBusApplication,
       WrongCommandsTest) {  // TODO: vscode - think about it
  int address = 0;
  int length = 1;

  std::array<std::byte, TestPublicMemory::kPublicMemoryLength> buffer{};

  master.RequestRead(std::span<std::byte>(buffer.begin(), length), address);
  stream.Clear();

  hydrolib::bus::application::MemoryAccessMessageBuffer wrong_command{};

  wrong_command.header.command = hydrolib::bus::application::Command::kError;
  wrong_command.header.info.address = 0;
  wrong_command.header.info.length = 1;

  write(stream, &wrong_command,
        sizeof(hydrolib::bus::application::MemoryAccessHeader) + 1);
  stream.MakeAllbytesAvailable();

  slave.Process();
  stream.MakeAllbytesAvailable();

  EXPECT_EQ(master.Process(), hydrolib::ReturnCode::ERROR);

  master.RequestRead(std::span<std::byte>(buffer.begin(), length), address);
  stream.MakeAllbytesAvailable();

  slave.Process();
  stream.MakeAllbytesAvailable();

  EXPECT_EQ(master.Process(), hydrolib::ReturnCode::OK);
}

TEST_F(TestHydrolibBusApplication, UnexpectedLenghtDataTest) {
  int address = 0;
  int requested_lenght = 1;

  std::array<std::byte, TestPublicMemory::kPublicMemoryLength> buffer{};

  master.RequestRead(std::span<std::byte>(buffer.begin(), requested_lenght),
                     address);
  stream.MakeAllbytesAvailable();
  slave.Process();
  stream.MakeAllbytesAvailable();
  stream.Clear();

  hydrolib::bus::application::MemoryAccessMessageBuffer wrong_data{};

  wrong_data.header.command = hydrolib::bus::application::Command::kResponse;
  wrong_data.header.info.address = 0;
  wrong_data.header.info.length = requested_lenght + 1;

  write(stream, &wrong_data,
        sizeof(hydrolib::bus::application::MemoryAccessHeader) + 1);

  stream.MakeAllbytesAvailable();
  EXPECT_EQ(master.Process(), hydrolib::ReturnCode::ERROR);
}
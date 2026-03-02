#pragma once

#include <gtest/gtest.h>

#include "hydrolib_bus_application_commands.hpp"
#include "hydrolib_bus_application_master.hpp"
#include "hydrolib_bus_application_slave.hpp"
#include "hydrolib_logger_mock.hpp"
#include "hydrolib_return_codes.hpp"
#include "mock_stream.hpp"

class TestPublicMemory {
 public:
  static constexpr int kPublicMemoryLength = 30;

  hydrolib::ReturnCode Read(void *read_buffer, unsigned address,
                            unsigned length);

  hydrolib::ReturnCode Write(const void *write_buffer, unsigned address,
                             unsigned length);

  std::array<uint8_t, kPublicMemoryLength> memory{};
};

class ThrusterMemory {
 public:
  static constexpr int kMemoryLength = 34;
  static constexpr int kConnectStatusAddress = 0;
  static constexpr int kThrusterStartAddress = 4;
  static constexpr uint32_t kOkeyStatusCode = 0xABABABAB;

  hydrolib::ReturnCode Read(void *read_buffer, unsigned address,
                            unsigned length) {
    if (!IsConnectionOkey()) {
      return hydrolib::ReturnCode::FAIL;
    }
    if (kThrusterStartAddress + address + length > kMemoryLength) {
      return hydrolib::ReturnCode::FAIL;
    }
    std::memcpy(read_buffer, &memory[kThrusterStartAddress + address], length);
    return hydrolib::ReturnCode::OK;
  }

  hydrolib::ReturnCode Write(const void *write_buffer, unsigned address,
                             unsigned length) {
    if (!IsConnectionOkey()) {
      return hydrolib::ReturnCode::FAIL;
    }
    if (kThrusterStartAddress + address + length > kMemoryLength) {
      return hydrolib::ReturnCode::FAIL;
    }
    std::memcpy(&memory[kThrusterStartAddress + address], write_buffer, length);
    return hydrolib::ReturnCode::OK;
  }

  bool IsConnectionOkey() {
    uint32_t connection_status;
    std::memcpy(&connection_status, &memory[kConnectStatusAddress],
                sizeof(connection_status));
    return connection_status == kOkeyStatusCode;
  }

  std::array<uint8_t, kMemoryLength> memory{};
};

struct TestCase {
  int address;
  int length;
};

class TestHydrolibBusApplication
    : public ::testing::Test,
      public ::testing::WithParamInterface<TestCase> {
 protected:
  TestHydrolibBusApplication();

  hydrolib::streams::mock::MockByteStream stream{};
  TestPublicMemory memory{};

  hydrolib::bus::application::Master<hydrolib::streams::mock::MockByteStream,
                                     decltype(hydrolib::logger::mock_logger)>
      master;
  hydrolib::bus::application::Slave<TestPublicMemory,
                                    decltype(hydrolib::logger::mock_logger),
                                    hydrolib::streams::mock::MockByteStream>
      slave;
  std::array<uint8_t, TestPublicMemory::kPublicMemoryLength> test_data{};
};

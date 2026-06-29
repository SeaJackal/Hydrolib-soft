#pragma once

#include <gtest/gtest.h>

#include "hydrolib_bus_application_master.hpp"
#include "hydrolib_bus_application_slave.hpp"
#include "hydrolib_logger_mock.hpp"
#include "hydrolib_return_codes.hpp"
#include "mock_stream.hpp"

class TestPublicMemory {
 public:
  static constexpr int kPublicMemoryLength = 30;

  hydrolib::ReturnCode Read(std::span<std::byte> read_buffer, int address);

  hydrolib::ReturnCode Write(std::span<const std::byte> write_buffer,
                             int address);

  std::array<std::byte, kPublicMemoryLength> memory{};
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

  hydrolib::streams::mock::MockByteStream stream;
  TestPublicMemory memory{};

  hydrolib::bus::application::Master<hydrolib::streams::mock::MockByteStream,
                                     decltype(hydrolib::logger::mock_logger)>
      master;
  hydrolib::bus::application::Slave<TestPublicMemory,
                                    decltype(hydrolib::logger::mock_logger),
                                    hydrolib::streams::mock::MockByteStream>
      slave;
  std::array<std::byte, TestPublicMemory::kPublicMemoryLength> test_data{};
};

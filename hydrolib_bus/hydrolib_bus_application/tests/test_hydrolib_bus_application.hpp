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

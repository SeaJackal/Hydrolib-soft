#pragma once

#include <gtest/gtest.h>

#include "hydrolib_bus_datalink_message.hpp"
#include "hydrolib_bus_datalink_stream.hpp"
#include "hydrolib_logger_mock.hpp"
#include "mock_stream.hpp"

class TestHydrolibBusDatalink : public ::testing::Test {
 public:
  static constexpr int kTestMessageLength =
      hydrolib::bus::datalink::kMaxDataLength;
  static constexpr int kTestDataLength =
      hydrolib::bus::datalink::kMaxDataLength;

  static constexpr hydrolib::bus::datalink::AddressType kSerializerAddress =
      std::byte(3);
  static constexpr hydrolib::bus::datalink::AddressType kDeserializerAddress =
      std::byte(4);

 protected:
  TestHydrolibBusDatalink();

  hydrolib::streams::mock::MockByteStream stream;

  hydrolib::bus::datalink::StreamManager<
      hydrolib::streams::mock::MockByteStream,
      decltype(hydrolib::logger::mock_logger)>
      sender_manager{kSerializerAddress, stream, hydrolib::logger::mock_logger};
  hydrolib::bus::datalink::StreamManager<
      hydrolib::streams::mock::MockByteStream,
      decltype(hydrolib::logger::mock_logger)>
      receiver_manager{kDeserializerAddress, stream,
                       hydrolib::logger::mock_logger};

  hydrolib::bus::datalink::Stream<hydrolib::streams::mock::MockByteStream,
                                  decltype(hydrolib::logger::mock_logger)>
      tx_stream{sender_manager, kDeserializerAddress};
  hydrolib::bus::datalink::Stream<hydrolib::streams::mock::MockByteStream,
                                  decltype(hydrolib::logger::mock_logger)>
      rx_stream{receiver_manager, kSerializerAddress};

  std::array<std::byte, kTestDataLength> test_data{};
};

class TestHydrolibBusDatalinkStreamInterface : public TestHydrolibBusDatalink {
 public:
  TestHydrolibBusDatalinkStreamInterface();

  void Send();
};

class TestHydrolibBusDatalinkParametrized
    : public TestHydrolibBusDatalink,
      public ::testing::WithParamInterface<int> {};
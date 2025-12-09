#pragma once

#include "hydrolib_bus_datalink_stream.hpp"
#include "hydrolib_logger_mock.hpp"
#include "mock_stream.hpp"

#include <deque>
#include <gtest/gtest.h>

#define SERIALIZER_ADDRESS 3
#define DESERIALIZER_ADDRESS 4

class TestHydrolibBusDatalink : public ::testing::Test
{
public:
    static constexpr int kTestDataLength = 10;

protected:
    TestHydrolibBusDatalink();

protected:
    hydrolib::streams::mock::MockByteStream stream;

    hydrolib::bus::datalink::StreamManager<
        hydrolib::streams::mock::MockByteStream,
        decltype(hydrolib::logger::mock_logger)>
        sender_manager;
    hydrolib::bus::datalink::StreamManager<
        hydrolib::streams::mock::MockByteStream,
        decltype(hydrolib::logger::mock_logger)>
        receiver_manager;

    hydrolib::bus::datalink::Stream<hydrolib::streams::mock::MockByteStream,
                                    decltype(hydrolib::logger::mock_logger)>
        tx_stream;
    hydrolib::bus::datalink::Stream<hydrolib::streams::mock::MockByteStream,
                                    decltype(hydrolib::logger::mock_logger)>
        rx_stream;

    uint8_t test_data[kTestDataLength];
};

class TestHydrolibBusDatalinkParametrized
    : public TestHydrolibBusDatalink,
      public ::testing::WithParamInterface<int>
{
};
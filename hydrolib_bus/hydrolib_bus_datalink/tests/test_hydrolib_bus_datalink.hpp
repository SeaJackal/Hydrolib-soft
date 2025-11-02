#pragma once

#include "hydrolib_bus_datalink_stream.hpp"
#include "hydrolib_logger_mock.hpp"

#include <deque>
#include <gtest/gtest.h>

#define PUBLIC_MEMORY_LENGTH 10

#define SERIALIZER_ADDRESS 3
#define DESERIALIZER_ADDRESS 4

class TestStream
{
    friend int read(TestStream &stream, void *dest, unsigned length);
    friend int write(TestStream &stream, const void *dest, unsigned length);

public:
void WriteByte(unsigned byte_index, unsigned byte);
unsigned ReadByte(unsigned byte_index);
int GetQueueSize();

private:
    std::deque<uint8_t> queue_;
};

int read(TestStream &stream, void *dest, unsigned length);
int write(TestStream &stream, const void *dest, unsigned length);

class TestHydrolibBusDatalink : public ::testing::Test
{
protected:
    TestHydrolibBusDatalink();

protected:
    TestStream stream;

    hydrolib::bus::datalink::StreamManager<
        TestStream, decltype(hydrolib::logger::mock_logger)>
        sender_manager;

    hydrolib::bus::datalink::StreamManager<
        TestStream, decltype(hydrolib::logger::mock_logger)>
        receiver_manager;

    uint8_t test_data[PUBLIC_MEMORY_LENGTH];
};

class TestHydrolibBusDatalinkParametrized
    : public TestHydrolibBusDatalink,
      public ::testing::WithParamInterface<std::tuple<uint16_t, uint16_t>>
{
};
#ifndef TEST_HYDROLIB_SP_SERIALIZE_ENV_H_
#define TEST_HYDROLIB_SP_SERIALIZE_ENV_H_

#include "hydrolib_bus_datalink_deserializer.hpp"
#include "hydrolib_bus_datalink_serializer.hpp"
#include "hydrolib_bus_datalink_stream.hpp"
#include "hydrolib_common.h"
#include "hydrolib_log_distributor.hpp"

#include <deque>
#include <iostream>

#include <gtest/gtest.h>

#define PUBLIC_MEMORY_LENGTH 20

#define SERIALIZER_ADDRESS 3
#define DESERIALIZER_ADDRESS 4

class TestLogStream
{
public:
    hydrolib_ReturnCode Push(const void *data, unsigned length)
    {
        for (unsigned i = 0; i < length; i++)
        {
            std::cout << (reinterpret_cast<const char *>(data))[i];
        }
        return HYDROLIB_RETURN_OK;
    }
    hydrolib_ReturnCode Open() { return HYDROLIB_RETURN_OK; };
    hydrolib_ReturnCode Close() { return HYDROLIB_RETURN_OK; };
};

int write([[maybe_unused]] TestLogStream &stream, const void *dest,
          unsigned length);

class TestStream
{
    friend int read(TestStream &stream, void *dest, unsigned length);
    friend int write(TestStream &stream, const void *dest, unsigned length);

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

    hydrolib::bus::datalink::Serializer<
        TestStream, hydrolib::logger::LogDistributor<TestLogStream>>
        serializer;
    hydrolib::bus::datalink::Deserializer<
        TestStream, hydrolib::logger::LogDistributor<TestLogStream>>
        deserializer;

    hydrolib::bus::datalink::Stream<
        TestStream, hydrolib::logger::LogDistributor<TestLogStream>>
        tx_stream;

    hydrolib::bus::datalink::Stream<
        TestStream, hydrolib::logger::LogDistributor<TestLogStream>>
        rx_stream;

    uint8_t test_data[PUBLIC_MEMORY_LENGTH];
};

class TestHydrolibBusDatalinkParametrized
    : public TestHydrolibBusDatalink,
      public ::testing::WithParamInterface<std::tuple<uint16_t, uint16_t>>
{
};

#endif

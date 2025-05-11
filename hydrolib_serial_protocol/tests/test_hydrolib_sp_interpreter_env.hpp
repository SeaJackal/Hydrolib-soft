#include "hydrolib_common.h"
#include "hydrolib_log_distributor.hpp"
#include "hydrolib_serial_protocol_commands.hpp"
#include "hydrolib_serial_protocol_interpreter.hpp"

#include <cstddef>
#include <cstring>
#include <deque>
#include <iostream>
#include <utility>

#include <gtest/gtest.h>

using namespace hydrolib::serial_protocol;
using namespace hydrolib::logger;

#define PUBLIC_MEMORY_LENGTH 20

class TestLogStream
{
public:
    hydrolib_ReturnCode Push(const uint8_t *data, unsigned length)
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

class TestPublicMemory
{
public:
    TestPublicMemory()
    {
        for (int i = 0; i < PUBLIC_MEMORY_LENGTH; i++)
        {
            memory[i] = i;
        }
    }

public:
    const uint8_t *Read(unsigned address, unsigned length)
    {
        if (address + length > PUBLIC_MEMORY_LENGTH)
        {
            return nullptr;
        }
        return memory + address;
    }

public:
    hydrolib_ReturnCode Write(const void *write_buffer, unsigned address,
                              unsigned length)
    {
        if (address + length > PUBLIC_MEMORY_LENGTH)
        {
            return HYDROLIB_RETURN_FAIL;
        }
        memcpy(&memory[address], write_buffer, length);
        return HYDROLIB_RETURN_OK;
    }

public:
    uint8_t memory[PUBLIC_MEMORY_LENGTH];
};

class TestTransmitter
{
public:
    hydrolib_ReturnCode Process(Command com, CommandInfo info)
    {
        command = com;
        command_info = info;
        return HYDROLIB_RETURN_OK;
    }

public:
    Command command;
    CommandInfo command_info;
};

class TestHydrolibSerialProtocolInterpreter : public ::testing::Test
{
protected:
    TestHydrolibSerialProtocolInterpreter();

protected:
    TestPublicMemory public_memory;
    TestTransmitter transmitter;
    uint8_t test_data[PUBLIC_MEMORY_LENGTH];
    Interpreter<TestPublicMemory, LogDistributor<TestLogStream>,
                TestTransmitter>
        interpreter;
};

class TestHydrolibSerialProtocolInterpreterParametrized
    : public TestHydrolibSerialProtocolInterpreter,
      public ::testing::WithParamInterface<std::tuple<uint16_t, uint16_t>>
{
};

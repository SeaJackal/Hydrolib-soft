#include "test_hydrolib_sp_interpreter_env.hpp"
#include "hydrolib_common.h"
#include "hydrolib_logger_mock.hpp"
#include "hydrolib_serial_protocol_commands.hpp"

using hydrolib::logger::mock_distributor;
using hydrolib::logger::mock_logger;

TestHydrolibSerialProtocolInterpreter::TestHydrolibSerialProtocolInterpreter()
    : interpreter(public_memory, transmitter, mock_logger)
{
    mock_distributor.SetAllFilters(0, LogLevel::DEBUG);
    for (int i = 0; i < PUBLIC_MEMORY_LENGTH; i++)
    {
        test_data[i] = i;
    }
}

// INSTANTIATE_TEST_CASE_P(
//     Test, TestHydrolibSerialProtocolSerializeParametrized,
//     ::testing::Combine(::testing::Range<uint16_t>(0, PUBLIC_MEMORY_LENGTH),
//                        ::testing::Range<uint16_t>(1,
//                                                   PUBLIC_MEMORY_LENGTH +
//                                                   1)));

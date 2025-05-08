#ifndef HYDROLIB_SERIAL_PROTOCOL_INTERPRETER_H_
#define HYDROLIB_SERIAL_PROTOCOL_INTERPRETER_H_

#include <cstdint>

#include "hydrolib_common.h"
#include "hydrolib_logger.hpp"
#include "hydrolib_serial_protocol_commands.hpp"

namespace hydrolib::serial_protocol
{

template <typename T>
concept PublicMemoryConcept =
    requires(T mem, void *read_buffer, const void *write_buffer,
             unsigned address, unsigned length) {
        {
            mem.Read(read_buffer, address, length)
        } -> std::same_as<hydrolib_ReturnCode>;

        {
            mem.Write(write_buffer, address, length)
        } -> std::same_as<hydrolib_ReturnCode>;
    };

template <PublicMemoryConcept Memory, logger::LogDistributorConcept Distributor>
class Interpreter
{
public:
    constexpr Interpreter(Memory &memory, logger::Logger<Distributor> &logger);

    hydrolib_ReturnCode Process(Command command, CommandInfo command_info);

private:
    logger::Logger<Distributor> &logger_;

    Memory &memory_;
};

template <PublicMemoryConcept Memory>
constexpr Interpreter<Memory>::Interpreter(Memory &memory) : memory_(memory)
{
}

template <PublicMemoryConcept Memory>
hydrolib_ReturnCode Interpreter<Memory>::Process(Command command,
                                                 CommandInfo command_info);

} // namespace hydrolib::serial_protocol

#endif

#pragma once

#include "hydrolib_shell_interpreter.hpp"
#include "hydrolib_shell_terminal.hpp"

#include "hydrolib_return_codes.hpp"

namespace hydrolib::shell
{
template <concepts::stream::ByteFullStreamConcept Stream, CommandMapConcept Map>
class Shell
{
public:
    constexpr Shell(Stream &stream, Map &handlers);

public:
    hydrolib::ReturnCode Process();

private:
    Terminal<Stream> terminal_;
    Interpreter<Map> interpreter_;

    int last_error_code_;
};

template <concepts::stream::ByteFullStreamConcept Stream, CommandMapConcept Map>
constexpr Shell<Stream, Map>::Shell(Stream &stream, Map &handlers)
    : terminal_(stream), interpreter_(handlers), last_error_code_(0)
{
}

template <concepts::stream::ByteFullStreamConcept Stream, CommandMapConcept Map>
hydrolib::ReturnCode Shell<Stream, Map>::Process()
{
    auto terminal_result = terminal_.Process();
    if (terminal_result != hydrolib::ReturnCode::OK)
    {
        return terminal_result;
    }
    auto command = terminal_.GetCommand();
    if (command.has_value())
    {
        last_error_code_ = interpreter_.Process(command.value());
        if (last_error_code_ != 0)
        {
            return hydrolib::ReturnCode::ERROR;
        }
        else
        {
            return hydrolib::ReturnCode::OK;
        }
    }
    else
    {
        return hydrolib::ReturnCode::ERROR;
    }
}

} // namespace hydrolib::shell

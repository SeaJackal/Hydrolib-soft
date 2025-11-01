#pragma once

#include "hydrolib_shell_interpreter.hpp"
#include "hydrolib_shell_terminal.hpp"

#include "hydrolib_return_codes.hpp"

namespace hydrolib::shell
{
template <concepts::stream::ByteFullStreamConcept Stream, typename Func,
          CommandMapConcept<Func> Map>
class Shell
{
public:
    constexpr Shell(Stream &stream, Map &handlers);

public:
    hydrolib::ReturnCode Process();

private:
    Terminal<Stream> terminal_;
    Interpreter<Func, Map> interpreter_;

    int last_error_code_;
};

template <concepts::stream::ByteFullStreamConcept Stream, typename Func,
          CommandMapConcept<Func> Map>
constexpr Shell<Stream, Func, Map>::Shell(Stream &stream, Map &handlers)
    : terminal_(stream), interpreter_(handlers), last_error_code_(0)
{
}

template <concepts::stream::ByteFullStreamConcept Stream, typename Func,
          CommandMapConcept<Func> Map>
hydrolib::ReturnCode Shell<Stream, Func, Map>::Process()
{
    auto terminal_result = terminal_.Process();
    if (terminal_result != hydrolib::ReturnCode::OK)
    {
        return terminal_result;
    }
    auto command = terminal_.GetCommand();
    last_error_code_ = interpreter_.Process(command);
    if (last_error_code_ != 0)
    {
        return hydrolib::ReturnCode::ERROR;
    }
    else
    {
        return hydrolib::ReturnCode::OK;
    }
}

} // namespace hydrolib::shell

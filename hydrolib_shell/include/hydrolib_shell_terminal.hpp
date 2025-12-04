#pragma once

#include "hydrolib_shell_params.hpp"

#include "hydrolib_return_codes.hpp"
#include "hydrolib_stream_concepts.hpp"

namespace hydrolib::shell
{

template <concepts::stream::ByteFullStreamConcept Stream>
class Terminal
{
public:
    constexpr Terminal(Stream &stream);

public:
    hydrolib::ReturnCode Process();

    CommandString GetCommand();

private:
    hydrolib::ReturnCode InterpretChar_(char last_char);

private:
    Stream &stream_;

    char command_string_[kMaxCommandLength] = {};
    int current_command_string_length_;
    int last_command_string_length_;
};

template <concepts::stream::ByteFullStreamConcept Stream>
constexpr Terminal<Stream>::Terminal(Stream &stream)
    : stream_(stream),
      current_command_string_length_(0),
      last_command_string_length_(0)
{
}

template <concepts::stream::ByteFullStreamConcept Stream>
hydrolib::ReturnCode Terminal<Stream>::Process()
{
    char last_char;
    int read_length = read(stream_, &last_char, 1);
    while (read_length != 0)
    {
        if (read_length < 0)
        {
            return hydrolib::ReturnCode::ERROR;
        }

        auto interpret_result = InterpretChar_(last_char);

        if (interpret_result == hydrolib::ReturnCode::OK)
        {
            command_string_[current_command_string_length_] = '\0';
            last_command_string_length_ = current_command_string_length_ + 1;
            current_command_string_length_ = 0;
        }

        if (interpret_result == hydrolib::ReturnCode::NO_DATA)
        {
            read_length = read(stream_, &last_char, 1);
        }
        else
        {
            return interpret_result;
        }
    }
    return hydrolib::ReturnCode::NO_DATA;
}

template <concepts::stream::ByteFullStreamConcept Stream>
CommandString Terminal<Stream>::GetCommand()
{
    return CommandString(
        command_string_,
        last_command_string_length_); // TODO: unnessessary copy
}

template <concepts::stream::ByteFullStreamConcept Stream>
hydrolib::ReturnCode Terminal<Stream>::InterpretChar_(char last_char)
{
    switch (last_char)
    {
    case '\n':
        write(stream_, &last_char, 1);
        return hydrolib::ReturnCode::OK;
    case '\r':
        write(stream_, &last_char, 1);
        return hydrolib::ReturnCode::NO_DATA;
    case 0x7F:
    case 0x08:
        if (current_command_string_length_ != 0)
        {
            current_command_string_length_--;
        }
        write(stream_, "\b \b", 3);
        return hydrolib::ReturnCode::NO_DATA;
    default:
        if (current_command_string_length_ >= kMaxCommandLength)
        {
            return hydrolib::ReturnCode::OVERFLOW;
        }
        write(stream_, &last_char, 1);
        command_string_[current_command_string_length_] = last_char;
        current_command_string_length_++;
        return hydrolib::ReturnCode::NO_DATA;
    }
}
} // namespace hydrolib::shell

#ifndef HYDROLIB_SERIAL_PROTOCOL_INTERPRETER_H_
#define HYDROLIB_SERIAL_PROTOCOL_INTERPRETER_H_

#include <cstdint>

namespace hydrolib::serial_protocol
{

enum Command : uint8_t
{
    WRITE = 1,
    READ,
    RESPONCE
};

union CommandInfo
{
public:
    struct Read
    {
        unsigned source_address;
        unsigned dest_address;
        unsigned memory_address;
        unsigned memory_access_length;
    };

    struct Write
    {
        unsigned source_address;
        unsigned dest_address;
        unsigned memory_address;
        unsigned memory_access_length;

        const uint8_t *data;
    };

    struct Responce
    {
        unsigned source_address;
        unsigned dest_address;
        unsigned data_length;

        const uint8_t *data;
    };

public:
    Read read;
    Write write;
    Responce responce;
};

} // namespace hydrolib::serial_protocol

#endif

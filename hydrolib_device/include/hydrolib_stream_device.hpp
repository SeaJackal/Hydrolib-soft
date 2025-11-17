#pragma once

#include "hydrolib_device.hpp"

#include "hydrolib_stream_concepts.hpp"

namespace hydrolib::device
{
class IStreamDevice : public Device
{
public:
    static constexpr DeviceType kSelfType = DeviceType::STREAM;

public:
    IStreamDevice(std::string_view name) : Device(name, kSelfType) {}

public:
    virtual int Read(void *data, int length) = 0;
    virtual int Write(const void *data, int length) = 0;
};

template <concepts::stream::ByteFullStreamConcept Stream>
class StreamDevice : public IStreamDevice
{
public:
    StreamDevice(std::string_view name, Stream &stream);

public:
    int Read(void *data, int length) override;
    int Write(const void *data, int length) override;

private:
    Stream &stream_;
};

template <concepts::stream::ByteFullStreamConcept Stream>
StreamDevice<Stream>::StreamDevice(std::string_view name, Stream &stream)
    : IStreamDevice(name), stream_(stream)
{
}

template <concepts::stream::ByteFullStreamConcept Stream>
int StreamDevice<Stream>::Read(void *data, int length)
{
    return read(stream_, data, length);
}

template <concepts::stream::ByteFullStreamConcept Stream>
int StreamDevice<Stream>::Write(const void *data, int length)
{
    return write(stream_, data, length);
}
} // namespace hydrolib::device

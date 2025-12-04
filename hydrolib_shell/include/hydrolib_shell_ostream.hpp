#pragma once

#include <cstring>
#include <string_view>

#include "hydrolib_stream_concepts.hpp"

namespace hydrolib::shell
{
class StreamInterface
{
public:
    virtual ~StreamInterface() = default;

public:
    virtual int Read(void *dest, unsigned length) = 0;
    virtual int Write(const void *source, unsigned length) = 0;
};

template <concepts::stream::ByteFullStreamConcept Stream>
class StreamWrapper : public StreamInterface
{
public:
    explicit StreamWrapper(Stream &stream) : stream_(stream) {}

public:
    int Read(void *dest, unsigned length) override { return read(stream_, dest, length); }
    int Write(const void *source, unsigned length) override { return write(stream_, source, length); }

private:
    Stream &stream_;
};

class Ostream
{
public:
    Ostream() = default;
    explicit Ostream(StreamInterface &stream) : stream_(&stream) {}

public:
    Ostream &operator<<(std::string_view string)
    {
        if (stream_)
        {
            stream_->Write(string.data(), static_cast<unsigned>(string.length()));
        }
        return *this;
    }

    Ostream &operator<<(char ch)
    {
        if (stream_)
        {
            stream_->Write(&ch, 1);
        }
        return *this;
    }

private:
    StreamInterface *stream_{};
};

} // namespace hydrolib::shell

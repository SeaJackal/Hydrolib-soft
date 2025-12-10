#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>

namespace hydrolib::streams::mock
{

class MockByteStream
{
    friend int read(MockByteStream &stream, void *dest, unsigned length);
    friend int write(MockByteStream &stream, const void *source,
                     unsigned length);

public:
    MockByteStream();

    std::size_t GetSize() const noexcept;

    bool IsEmpty() const noexcept;

    void Clear() noexcept;

    uint8_t &operator[](std::size_t index);
    const uint8_t &operator[](std::size_t index) const;

    void AddAvailableBytes(int available_bytes);
    void MakeAllbytesAvailable();

private:
    std::deque<uint8_t> buffer_;

    int available_bytes_ = 0;
};

int write(MockByteStream &stream, const void *source, unsigned length);
int read(MockByteStream &stream, void *dest, unsigned length);

} // namespace hydrolib::streams::mock

#include "mock_stream.hpp"

namespace hydrolib::streams::mock
{

MockByteStream::MockByteStream() = default;

std::size_t MockByteStream::GetSize() const noexcept
{
    return buffer_.size();
}

bool MockByteStream::IsEmpty() const noexcept
{
    return buffer_.empty();
}

void MockByteStream::Clear() noexcept
{
    buffer_.clear();
}

uint8_t &MockByteStream::operator[](std::size_t index)
{
    return buffer_[index];
}

const uint8_t &MockByteStream::operator[](std::size_t index) const
{
    return buffer_[index];
}

int write(MockByteStream &stream, const void *source, unsigned length)
{
    if (length == 0)
        return 0;
    if (source == nullptr)
        return -1;

    const auto *bytes = static_cast<const uint8_t *>(source);
    for (unsigned i = 0; i < length; ++i)
    {
        stream.buffer_.push_back(bytes[i]);
    }
    return static_cast<int>(length);
}

int read(MockByteStream &stream, void *dest, unsigned length)
{
    if (length == 0)
        return 0;
    if (dest == nullptr)
        return -1;

    auto *bytes = static_cast<uint8_t *>(dest);
    const unsigned available = static_cast<unsigned>(stream.buffer_.size());
    const unsigned to_read = (length < available) ? length : available;

    for (unsigned i = 0; i < to_read; ++i)
    {
        bytes[i] = stream.buffer_.front();
        stream.buffer_.pop_front();
    }
    return static_cast<int>(to_read);
}

} // namespace hydrolib::streams::mock



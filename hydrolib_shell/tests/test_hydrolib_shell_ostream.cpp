#include "hydrolib_shell_ostream.hpp"

#include <deque>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

using hydrolib::shell::Ostream;
using hydrolib::shell::StreamWrapper;

namespace
{

// Minimal fake stream satisfying ByteFullStreamConcept via free read/write
struct FakeStream
{
    std::deque<char> input_;
    std::vector<char> output_;
};

// read is required by the concept but unused by Ostream; provide a stub
[[maybe_unused]] int read(FakeStream &stream, void *dest, unsigned length)
{
    if (length == 0 || stream.input_.empty())
    {
        return 0;
    }
    const unsigned available = static_cast<unsigned>(stream.input_.size());
    const unsigned to_copy = std::min(length, available);
    auto *char_dest = static_cast<char *>(dest);
    for (unsigned i = 0; i < to_copy; ++i)
    {
        char_dest[i] = stream.input_.front();
        stream.input_.pop_front();
    }
    return static_cast<int>(to_copy);
}

[[maybe_unused]] int write(FakeStream &stream, const void *source, unsigned length)
{
    const auto *char_source = static_cast<const char *>(source);
    stream.output_.insert(stream.output_.end(), char_source, char_source + length);
    return static_cast<int>(length);
}

std::string OutputAsString(const FakeStream &stream)
{
    return std::string(stream.output_.begin(), stream.output_.end());
}

} // namespace

TEST(HydrolibShellOstream, WritesStringView)
{
    FakeStream stream;
    StreamWrapper<FakeStream> wrapper(stream);
    Ostream os(wrapper);

    os << std::string_view("Hello, world!");

    EXPECT_EQ(OutputAsString(stream), "Hello, world!");
}

TEST(HydrolibShellOstream, WritesConstCharPointerTerminates)
{
    FakeStream stream;
    StreamWrapper<FakeStream> wrapper(stream);
    Ostream os(wrapper);

    const char text[] = {'A', '\0', 'B', 'C'};
    os << text;

    EXPECT_EQ(OutputAsString(stream), "A");
}

TEST(HydrolibShellOstream, WritesSingleChar)
{
    FakeStream stream;
    StreamWrapper<FakeStream> wrapper(stream);
    Ostream os(wrapper);

    os << 'X';

    EXPECT_EQ(OutputAsString(stream), "X");
}

TEST(HydrolibShellOstream, SupportsChaining)
{
    FakeStream stream;
    StreamWrapper<FakeStream> wrapper(stream);
    Ostream os(wrapper);

    os << "A" << 'B' << std::string_view("CDE");

    EXPECT_EQ(OutputAsString(stream), "ABCDE");
}

TEST(HydrolibShellOstream, DefaultConstructed_DoesNothingAndReturnsSelf)
{
    Ostream os_default;
    auto &ref = os_default << "ignored";

    // No crash and returns self reference for chaining
    EXPECT_EQ(&ref, &os_default);
}



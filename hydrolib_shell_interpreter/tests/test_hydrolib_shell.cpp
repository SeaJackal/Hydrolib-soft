#include "hydrolib_shell.hpp"

#include <algorithm>
#include <deque>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

namespace
{

struct FakeStream
{
    FakeStream() = default;

    explicit FakeStream(std::string_view initial_data)
    {
        AppendInput(initial_data);
    }

    void AppendInput(std::string_view data)
    {
        input_.insert(input_.end(), data.begin(), data.end());
    }

    void AppendChar(char ch) { input_.push_back(ch); }

    std::string OutputAsString() const
    {
        return std::string(output_.begin(), output_.end());
    }

    std::deque<char> input_;
    std::vector<char> output_;
    bool fail_next_read_ = false;
};

[[maybe_unused]] int read(FakeStream &stream, void *dest, unsigned length)
{
    if (stream.fail_next_read_)
    {
        stream.fail_next_read_ = false;
        return -1;
    }

    if (length == 0)
    {
        return 0;
    }

    if (stream.input_.empty())
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

[[maybe_unused]] int write(FakeStream &stream, const void *source,
                           unsigned length)
{
    const auto *char_source = static_cast<const char *>(source);
    stream.output_.insert(stream.output_.end(), char_source,
                          char_source + length);
    return static_cast<int>(length);
}

} // namespace

using hydrolib::ReturnCode;

struct TestCommandMap
{
    using HandlerType = std::optional<std::function<int(int, char *[])>>;

    HandlerType &operator[](std::string_view key)
    {
        return storage[std::string(key)];
    }

    std::unordered_map<std::string, HandlerType> storage;
};

using ShellUnderTest = hydrolib::shell::Shell<FakeStream, TestCommandMap>;

TEST(ShellProcess, ReturnsTerminalResultWhenNoData)
{
    FakeStream stream;
    TestCommandMap handlers;
    ShellUnderTest shell(stream, handlers);

    EXPECT_EQ(ReturnCode::NO_DATA, shell.Process());
    EXPECT_TRUE(stream.OutputAsString().empty());
}

TEST(ShellProcess, PropagatesTerminalError)
{
    FakeStream stream;
    stream.fail_next_read_ = true;

    TestCommandMap handlers;
    ShellUnderTest shell(stream, handlers);

    EXPECT_EQ(ReturnCode::ERROR, shell.Process());
    EXPECT_TRUE(stream.OutputAsString().empty());
}

TEST(ShellProcess, InvokesHandlerAndReturnsOkOnSuccess)
{
    FakeStream stream;
    stream.AppendInput("run\n");

    TestCommandMap handlers;
    bool handler_called = false;
    std::vector<std::string> received_arguments;

    handlers.storage["run"] = std::function<int(int, char *[])>(
        [&](int argc, char *argv[])
        {
            handler_called = true;
            received_arguments.clear();
            for (int i = 0; i < argc; ++i)
            {
                received_arguments.emplace_back(argv[i]);
            }
            return 0;
        });

    ShellUnderTest shell(stream, handlers);

    EXPECT_EQ(ReturnCode::OK, shell.Process());
    EXPECT_TRUE(handler_called);
    ASSERT_EQ(1u, received_arguments.size());
    EXPECT_EQ("run", received_arguments.front());
    EXPECT_EQ("run\n", stream.OutputAsString());
}

TEST(ShellProcess, ReturnsErrorWhenHandlerFails)
{
    FakeStream stream;
    stream.AppendInput("fail\n");

    TestCommandMap handlers;
    bool handler_called = false;

    handlers.storage["fail"] = std::function<int(int, char *[])>(
        [&](int, char *[])
        {
            handler_called = true;
            return 7;
        });

    ShellUnderTest shell(stream, handlers);

    EXPECT_EQ(ReturnCode::ERROR, shell.Process());
    EXPECT_TRUE(handler_called);
    EXPECT_EQ("fail\n", stream.OutputAsString());
}

TEST(ShellProcess, ReturnsErrorWhenHandlerMissing)
{
    FakeStream stream;
    stream.AppendInput("unknown\n");

    TestCommandMap handlers;
    ShellUnderTest shell(stream, handlers);

    EXPECT_EQ(ReturnCode::ERROR, shell.Process());
    EXPECT_EQ("unknown\n", stream.OutputAsString());
}

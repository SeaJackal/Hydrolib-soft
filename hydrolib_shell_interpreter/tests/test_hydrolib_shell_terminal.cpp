#include "hydrolib_shell_terminal.hpp"

#include <algorithm>
#include <deque>
#include <optional>
#include <string>
#include <string_view>
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

    void AppendChar(char ch)
    {
        input_.push_back(ch);
    }

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

[[maybe_unused]] int write(FakeStream &stream, const void *source, unsigned length)
{
    const auto *char_source = static_cast<const char *>(source);
    stream.output_.insert(stream.output_.end(), char_source,
                          char_source + length);
    return static_cast<int>(length);
}

} // namespace

using hydrolib::ReturnCode;

namespace
{

std::string CommandToStdString(const hydrolib::shell::CommandString &command)
{
    const auto length = static_cast<std::size_t>(command.GetLength());
    const char *data = static_cast<const char *>(command);
    return std::string(data, data + length);
}

} // namespace

TEST(TerminalProcess, EchoesPrintableCharacters)
{
    FakeStream stream("a");
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    EXPECT_EQ(std::string("a"), stream.OutputAsString());
    EXPECT_FALSE(terminal.GetCommand().has_value());
}

TEST(TerminalProcess, HandlesBackspace)
{
    FakeStream stream;
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    stream.AppendInput("a");
    ASSERT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendInput(std::string_view("\b", 1));
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendInput("c");
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    EXPECT_EQ(std::string("a\b \bc"), stream.OutputAsString());
    EXPECT_FALSE(terminal.GetCommand().has_value());
}

TEST(TerminalProcess, HandlesDelete)
{
    FakeStream stream;
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    stream.AppendInput("a");
    ASSERT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendInput(std::string_view("\x7F", 1));
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendInput("z");
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    EXPECT_EQ(std::string("a\b \bz"), stream.OutputAsString());
    EXPECT_FALSE(terminal.GetCommand().has_value());
}

TEST(TerminalProcess, HandlesBackspaceWithoutPrintableCharacters)
{
    FakeStream stream(std::string_view("\b", 1));
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    EXPECT_EQ(std::string("\b \b"), stream.OutputAsString());
    EXPECT_FALSE(terminal.GetCommand().has_value());
}

TEST(TerminalProcess, HandlesDeleteWithoutPrintableCharacters)
{
    FakeStream stream(std::string_view("\x7F", 1));
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    EXPECT_EQ(std::string("\b \b"), stream.OutputAsString());
    EXPECT_FALSE(terminal.GetCommand().has_value());
}

TEST(TerminalProcess, MirrorsCarriageReturn)
{
    FakeStream stream(std::string_view("\r", 1));
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    EXPECT_EQ(std::string("\r"), stream.OutputAsString());
    EXPECT_FALSE(terminal.GetCommand().has_value());
}

TEST(TerminalProcess, ProducesCommandAfterNewline)
{
    FakeStream stream;
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    stream.AppendChar('r');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    EXPECT_FALSE(terminal.GetCommand().has_value());

    stream.AppendChar('u');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    EXPECT_FALSE(terminal.GetCommand().has_value());

    stream.AppendChar('n');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    EXPECT_FALSE(terminal.GetCommand().has_value());

    stream.AppendInput(std::string_view("\n", 1));
    EXPECT_EQ(ReturnCode::OK, terminal.Process());

    const auto command = terminal.GetCommand();
    ASSERT_TRUE(command.has_value());
    EXPECT_EQ(std::string("run"), CommandToStdString(*command));
    EXPECT_FALSE(terminal.GetCommand().has_value());
}

TEST(TerminalProcess, EchoesNewlineCharacter)
{
    FakeStream stream;
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    stream.AppendInput(std::string_view("\n", 1));
    EXPECT_EQ(ReturnCode::OK, terminal.Process());
    EXPECT_EQ(std::string("\n"), stream.OutputAsString());

    const auto command = terminal.GetCommand();
    ASSERT_TRUE(command.has_value());
    EXPECT_TRUE(CommandToStdString(*command).empty());
}

TEST(TerminalProcess, EchoesComplexSequence)
{
    FakeStream stream;
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    stream.AppendChar('a');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    stream.AppendChar('b');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    stream.AppendChar('c');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendInput(std::string_view("\b", 1));
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendChar('d');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendInput(std::string_view("\x7F", 1));
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendChar('e');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    stream.AppendChar('f');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    EXPECT_EQ(std::string("abc\b \bd\b \bef"), stream.OutputAsString());
    EXPECT_FALSE(terminal.GetCommand().has_value());
}

TEST(TerminalProcess, ProducesCommandWithDeletes)
{
    FakeStream stream;
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    stream.AppendChar('a');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    stream.AppendChar('b');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    stream.AppendChar('c');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendInput(std::string_view("\b", 1));
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendChar('d');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendInput(std::string_view("\x7F", 1));
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendChar('e');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    stream.AppendChar('f');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendInput(std::string_view("\n", 1));
    EXPECT_EQ(ReturnCode::OK, terminal.Process());

    const auto command = terminal.GetCommand();
    ASSERT_TRUE(command.has_value());
    EXPECT_EQ(std::string("abef"), CommandToStdString(*command));
}

TEST(TerminalProcess, ProducesCommandAfterExcessDeletes)
{
    FakeStream stream;
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    stream.AppendChar('a');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    stream.AppendChar('b');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendInput(std::string_view("\x7F", 1));
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    stream.AppendInput(std::string_view("\x7F", 1));
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    stream.AppendInput(std::string_view("\x7F", 1));
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendChar('c');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    stream.AppendChar('d');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendInput(std::string_view("\n", 1));
    EXPECT_EQ(ReturnCode::OK, terminal.Process());

    const auto command = terminal.GetCommand();
    ASSERT_TRUE(command.has_value());
    EXPECT_EQ(std::string("cd"), CommandToStdString(*command));
}

TEST(TerminalProcess, ProcessesSequentialCommands)
{
    FakeStream stream;
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    const std::string first_command = "status";
    const std::string second_command = "reset";

    auto feed_command = [&](std::string_view command) {
        for (char ch : command)
        {
            stream.AppendChar(ch);
            EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
        }
        stream.AppendInput(std::string_view("\n", 1));
        EXPECT_EQ(ReturnCode::OK, terminal.Process());
    };

    feed_command(first_command);
    const auto first = terminal.GetCommand();
    ASSERT_TRUE(first.has_value());
    EXPECT_EQ(first_command, CommandToStdString(*first));

    feed_command(second_command);
    const auto second = terminal.GetCommand();
    ASSERT_TRUE(second.has_value());
    EXPECT_EQ(second_command, CommandToStdString(*second));

    EXPECT_EQ(first_command + "\n" + second_command + "\n",
              stream.OutputAsString());
}

TEST(TerminalProcess, ProcessesSequentialCommandsWithEditing)
{
    FakeStream stream;
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    const std::string first_command = "alpha";
    for (char ch : first_command)
    {
        stream.AppendChar(ch);
        EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    }
    stream.AppendInput(std::string_view("\n", 1));
    EXPECT_EQ(ReturnCode::OK, terminal.Process());

    const auto first = terminal.GetCommand();
    ASSERT_TRUE(first.has_value());
    EXPECT_EQ(first_command, CommandToStdString(*first));

    stream.AppendChar('b');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    stream.AppendChar('e');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    stream.AppendChar('t');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendInput(std::string_view("\b", 1));
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendChar('s');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    stream.AppendChar('t');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendInput(std::string_view("\n", 1));
    EXPECT_EQ(ReturnCode::OK, terminal.Process());

    const auto second = terminal.GetCommand();
    ASSERT_TRUE(second.has_value());
    EXPECT_EQ(std::string("best"), CommandToStdString(*second));

    EXPECT_EQ(first_command + "\nbet\b \bst\n", stream.OutputAsString());
}

TEST(TerminalProcess, ReportsOverflowWhenCommandTooLong)
{
    FakeStream stream;
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    const std::string long_command(hydrolib::shell::kMaxCommandLength, 'x');

    for (char ch : long_command)
    {
        stream.AppendChar(ch);
        ASSERT_EQ(ReturnCode::NO_DATA, terminal.Process());
    }

    EXPECT_EQ(long_command, stream.OutputAsString());
    EXPECT_FALSE(terminal.GetCommand().has_value());

    stream.AppendChar('y');
    auto process_result = terminal.Process();
    EXPECT_EQ(ReturnCode::OVERFLOW, process_result);
}

TEST(TerminalProcess, CarriageReturnBeforeNewlineProducesCommand)
{
    FakeStream stream;
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    stream.AppendChar('h');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    stream.AppendChar('i');
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendInput(std::string_view("\r", 1));
    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());

    stream.AppendInput(std::string_view("\n", 1));
    EXPECT_EQ(ReturnCode::OK, terminal.Process());

    const auto command = terminal.GetCommand();
    ASSERT_TRUE(command.has_value());
    EXPECT_EQ(std::string("hi"), CommandToStdString(*command));
    EXPECT_EQ(std::string("hi\r\n"), stream.OutputAsString());
}

TEST(TerminalProcess, ProducesEmptyCommandsSequentially)
{
    FakeStream stream;
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    auto feed_empty_command = [&]() {
        stream.AppendInput(std::string_view("\n", 1));
        EXPECT_EQ(ReturnCode::OK, terminal.Process());
        const auto command = terminal.GetCommand();
        ASSERT_TRUE(command.has_value());
        EXPECT_TRUE(CommandToStdString(*command).empty());
    };

    feed_empty_command();
    feed_empty_command();

    EXPECT_EQ(std::string("\n\n"), stream.OutputAsString());
}

TEST(TerminalProcess, ReturnsNoDataForEmptyStream)
{
    FakeStream stream;
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    EXPECT_EQ(ReturnCode::NO_DATA, terminal.Process());
    EXPECT_TRUE(stream.OutputAsString().empty());
    EXPECT_FALSE(terminal.GetCommand().has_value());
}

TEST(TerminalProcess, ReturnsErrorWhenReadFails)
{
    FakeStream stream;
    stream.fail_next_read_ = true;
    hydrolib::shell::Terminal<FakeStream> terminal(stream);

    EXPECT_EQ(ReturnCode::ERROR, terminal.Process());
    EXPECT_TRUE(stream.OutputAsString().empty());
    EXPECT_FALSE(terminal.GetCommand().has_value());
}

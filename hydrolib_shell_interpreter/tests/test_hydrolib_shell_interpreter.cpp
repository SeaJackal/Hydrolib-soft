#include "hydrolib_shell_interpreter.hpp"

#include <functional>
#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

class RecordingHandlersMap
{
public:
    RecordingHandlersMap() = default;

    explicit RecordingHandlersMap(std::string command, int return_code = 0)
    {
        AddHandler(std::move(command), return_code);
    }

    void AddHandler(std::string command, int return_code = 0)
    {
        handlers_.emplace(std::move(command), return_code);
    }

    std::optional<std::function<int(int, char *[])>>
    operator[](std::string_view command)
    {
        const auto it = handlers_.find(std::string(command));
        if (it == handlers_.end())
        {
            return std::nullopt;
        }

        const std::string command_string(it->first);
        const int return_code = it->second;

        return [this, command_string, return_code](int argc, char **argv)
        {
            Invocation invocation{command_string, {}, argc};
            invocation.args.reserve(argc);
            for (int i = 0; i < argc; ++i)
            {
                invocation.args.emplace_back(argv[i]);
            }
            invocations.emplace_back(std::move(invocation));
            return return_code;
        };
    }

    struct Invocation
    {
        std::string command;
        std::vector<std::string> args;
        int argc = 0;
    };
    std::vector<Invocation> invocations;

private:
    std::unordered_map<std::string, int> handlers_;
};

TEST(InterpreterProcess, TrimsLeadingSpaces)
{
    RecordingHandlersMap handlers;
    handlers.AddHandler("run", 7);
    handlers.AddHandler("execute", 8);
    hydrolib::shell::Interpreter interpreter(handlers);

    const int run_result = interpreter.Process("   run arg1 arg2");
    const int execute_result =
        interpreter.Process(" execute    paramX   paramY   paramZ ");

    EXPECT_EQ(7, run_result);
    EXPECT_EQ(8, execute_result);

    ASSERT_EQ(2u, handlers.invocations.size());
    EXPECT_EQ("run", handlers.invocations[0].command);
    EXPECT_EQ((std::vector<std::string>{"run", "arg1", "arg2"}),
              handlers.invocations[0].args);
    EXPECT_EQ(3, handlers.invocations[0].argc);
    EXPECT_EQ("execute", handlers.invocations[1].command);
    EXPECT_EQ(
        (std::vector<std::string>{"execute", "paramX", "paramY", "paramZ"}),
        handlers.invocations[1].args);
    EXPECT_EQ(4, handlers.invocations[1].argc);
}

TEST(InterpreterProcess, TrimsTrailingSpaces)
{
    RecordingHandlersMap handlers;
    handlers.AddHandler("run", 11);
    handlers.AddHandler("deploy", 12);
    hydrolib::shell::Interpreter interpreter(handlers);

    const int run_result = interpreter.Process("run arg1 arg2   ");
    const int deploy_result = interpreter.Process("  deploy    release   ");

    EXPECT_EQ(11, run_result);
    EXPECT_EQ(12, deploy_result);

    ASSERT_EQ(2u, handlers.invocations.size());
    EXPECT_EQ("run", handlers.invocations[0].command);
    EXPECT_EQ((std::vector<std::string>{"run", "arg1", "arg2"}),
              handlers.invocations[0].args);
    EXPECT_EQ(3, handlers.invocations[0].argc);
    EXPECT_EQ("deploy", handlers.invocations[1].command);
    EXPECT_EQ((std::vector<std::string>{"deploy", "release"}),
              handlers.invocations[1].args);
    EXPECT_EQ(2, handlers.invocations[1].argc);
}

TEST(InterpreterProcess, HandlesConsecutiveSpaces)
{
    RecordingHandlersMap handlers;
    handlers.AddHandler("run", 5);
    handlers.AddHandler("launch", 6);
    hydrolib::shell::Interpreter interpreter(handlers);

    const int run_result = interpreter.Process("run   arg1     arg2    arg3");
    const int launch_result = interpreter.Process(" launch    one      two  ");

    EXPECT_EQ(5, run_result);
    EXPECT_EQ(6, launch_result);

    ASSERT_EQ(2u, handlers.invocations.size());
    EXPECT_EQ("run", handlers.invocations[0].command);
    EXPECT_EQ((std::vector<std::string>{"run", "arg1", "arg2", "arg3"}),
              handlers.invocations[0].args);
    EXPECT_EQ(4, handlers.invocations[0].argc);
    EXPECT_EQ("launch", handlers.invocations[1].command);
    EXPECT_EQ((std::vector<std::string>{"launch", "one", "two"}),
              handlers.invocations[1].args);
    EXPECT_EQ(3, handlers.invocations[1].argc);
}

TEST(InterpreterProcess, ReturnsErrorForUnknownCommand)
{
    RecordingHandlersMap handlers;
    handlers.AddHandler("expected", 13);
    handlers.AddHandler("expected_alt", 14);
    hydrolib::shell::Interpreter interpreter(handlers);

    const int unknown_result = interpreter.Process("unknown arg");
    EXPECT_EQ(-2, unknown_result);
    EXPECT_TRUE(handlers.invocations.empty());

    const int expected_result =
        interpreter.Process(" expected    alpha   beta  ");
    const int alt_result =
        interpreter.Process("expected_alt gamma   delta   epsilon");

    EXPECT_EQ(13, expected_result);
    EXPECT_EQ(14, alt_result);

    ASSERT_EQ(2u, handlers.invocations.size());
    EXPECT_EQ("expected", handlers.invocations[0].command);
    EXPECT_EQ((std::vector<std::string>{"expected", "alpha", "beta"}),
              handlers.invocations[0].args);
    EXPECT_EQ(3, handlers.invocations[0].argc);
    EXPECT_EQ("expected_alt", handlers.invocations[1].command);
    EXPECT_EQ(
        (std::vector<std::string>{"expected_alt", "gamma", "delta", "epsilon"}),
        handlers.invocations[1].args);
    EXPECT_EQ(4, handlers.invocations[1].argc);
}

TEST(InterpreterProcess, HandlesCommandWithoutArguments)
{
    RecordingHandlersMap handlers;
    handlers.AddHandler("run", 3);
    handlers.AddHandler("start", 4);
    hydrolib::shell::Interpreter interpreter(handlers);

    const int run_result = interpreter.Process("run");
    const int start_result = interpreter.Process(" start   ");

    EXPECT_EQ(3, run_result);
    EXPECT_EQ(4, start_result);

    ASSERT_EQ(2u, handlers.invocations.size());
    EXPECT_EQ("run", handlers.invocations[0].command);
    EXPECT_EQ((std::vector<std::string>{"run"}), handlers.invocations[0].args);
    EXPECT_EQ(1, handlers.invocations[0].argc);
    EXPECT_EQ("start", handlers.invocations[1].command);
    EXPECT_EQ((std::vector<std::string>{"start"}),
              handlers.invocations[1].args);
    EXPECT_EQ(1, handlers.invocations[1].argc);
}

TEST(InterpreterProcess, ProcessesMultipleCommandsSequentially)
{
    RecordingHandlersMap handlers;
    handlers.AddHandler("run", 1);
    handlers.AddHandler("stop", 2);
    handlers.AddHandler("status", 3);
    hydrolib::shell::Interpreter interpreter(handlers);

    const int run_result = interpreter.Process("run    start");
    const int stop_result = interpreter.Process("   stop   final   ");
    const int status_result = interpreter.Process(" status   check    now  ");

    EXPECT_EQ(1, run_result);
    EXPECT_EQ(2, stop_result);
    EXPECT_EQ(3, status_result);

    ASSERT_EQ(3u, handlers.invocations.size());
    EXPECT_EQ("run", handlers.invocations[0].command);
    EXPECT_EQ((std::vector<std::string>{"run", "start"}),
              handlers.invocations[0].args);
    EXPECT_EQ(2, handlers.invocations[0].argc);
    EXPECT_EQ("stop", handlers.invocations[1].command);
    EXPECT_EQ((std::vector<std::string>{"stop", "final"}),
              handlers.invocations[1].args);
    EXPECT_EQ(2, handlers.invocations[1].argc);
    EXPECT_EQ("status", handlers.invocations[2].command);
    EXPECT_EQ((std::vector<std::string>{"status", "check", "now"}),
              handlers.invocations[2].args);
    EXPECT_EQ(3, handlers.invocations[2].argc);
}

TEST(InterpreterProcess, MultipleCommandsWithIntermediateUnknown)
{
    RecordingHandlersMap handlers;
    handlers.AddHandler("first", 3);
    handlers.AddHandler("second", 4);
    handlers.AddHandler("third", 5);
    hydrolib::shell::Interpreter interpreter(handlers);

    EXPECT_EQ(3, interpreter.Process("first   arg"));
    EXPECT_EQ(-2, interpreter.Process("unknown"));
    EXPECT_EQ(4, interpreter.Process("  second    value  "));
    EXPECT_EQ(5, interpreter.Process(" third   more    data    items"));

    ASSERT_EQ(3u, handlers.invocations.size());
    EXPECT_EQ("first", handlers.invocations[0].command);
    EXPECT_EQ((std::vector<std::string>{"first", "arg"}),
              handlers.invocations[0].args);
    EXPECT_EQ(2, handlers.invocations[0].argc);
    EXPECT_EQ("second", handlers.invocations[1].command);
    EXPECT_EQ((std::vector<std::string>{"second", "value"}),
              handlers.invocations[1].args);
    EXPECT_EQ(2, handlers.invocations[1].argc);
    EXPECT_EQ("third", handlers.invocations[2].command);
    EXPECT_EQ((std::vector<std::string>{"third", "more", "data", "items"}),
              handlers.invocations[2].args);
    EXPECT_EQ(4, handlers.invocations[2].argc);
}

TEST(InterpreterProcess, ReturnsErrorForEmptyCommand)
{
    RecordingHandlersMap handlers;
    hydrolib::shell::Interpreter interpreter(handlers);

    const int result = interpreter.Process("");

    EXPECT_EQ(-2, result);
    EXPECT_TRUE(handlers.invocations.empty());
}

#pragma once

#include "hydrolib_shell_params.hpp"

#include <concepts>
#include <cstring>
#include <functional>
#include <optional>
#include <string_view>

namespace hydrolib::shell
{

template <typename T, typename KeyType, typename ValueType>
concept ReadableMapConcept = requires(T map, KeyType key) {
    { map[key] } -> std::convertible_to<ValueType>;
};

template <typename T>
concept CommandMapConcept =
    ReadableMapConcept<T, std::string_view,
                       std::optional<std::function<int(int, char *[])>>>;

template <CommandMapConcept Map>
class Interpreter
{
public:
    constexpr Interpreter(Map &handlers);

public:
    int Process(CommandString command);

private:
    Map &handlers_;
};

<<<<<<< HEAD
=======
<<<<<<<< HEAD:hydrolib_shell_interpreter/include/hydrolib_shell_interpreter.hpp
template <CommandMapConcept Map>
constexpr Interpreter<Map>::Interpreter(Map &handlers) : handlers_(handlers)
========
>>>>>>> ddeb302 (Added hydrolib shell and hydrolib cat)
inline bool g_is_running = false;

template <typename Func, CommandMapConcept<Func> Map>
constexpr Interpreter<Func, Map>::Interpreter(Map &handlers)
    : handlers_(handlers)
<<<<<<< HEAD
{
}

template <typename Func, CommandMapConcept<Func> Map>
int Interpreter<Func, Map>::Process(CommandString command)
=======
>>>>>>>> ddeb302 (Added hydrolib shell and hydrolib cat):hydrolib_shell/include/hydrolib_shell_interpreter.hpp
{
}

template <CommandMapConcept Map>
int Interpreter<Map>::Process(CommandString command)
>>>>>>> ddeb302 (Added hydrolib shell and hydrolib cat)
{
    int argc = 0;
    char *argv[kMaxArgsCount];
    if (command[0] != ' ')
    {
        argv[0] = static_cast<char *>(command);
        argc++;
    }
    for (int pos = 0; pos < command.GetLength(); pos++)
    {
        if (command[pos] == ' ')
        {
            command[pos] = '\0';
            pos++;
            while (command[pos] == ' ')
            {
                pos++;
            }
            if (command[pos])
            {
                if (argc > kMaxArgsCount)
                {
                    return -1;
                }
                argv[argc] = static_cast<char *>(command) + pos;
                argc++;
            }
        }
    }
    if (handlers_[static_cast<std::string_view>(argv[0])])
    {
<<<<<<< HEAD
        g_is_running = true; // TODO: Add tests
        optind = 1;
        opterr = 0;
=======
        g_is_running = true; //TODO: Add tests
>>>>>>> ddeb302 (Added hydrolib shell and hydrolib cat)
        auto result =
            (*handlers_[static_cast<std::string_view>(argv[0])])(argc, argv);
        g_is_running = false;
        return result;
    }
    return -2;
}
} // namespace hydrolib::shell

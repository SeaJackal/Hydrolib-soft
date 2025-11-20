#pragma once

#include "hydrolib_logger.hpp"

#define LOG(logger_, level, message, ...)                                      \
    hydrolib::logger::LoggingCase(logger_ __VA_OPT__(,) __VA_ARGS__)                        \
        .WriteLog(level, message __VA_OPT__(,) __VA_ARGS__)

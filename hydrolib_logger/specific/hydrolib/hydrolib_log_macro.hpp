#pragma once

#include "hydrolib_logger.hpp"

#define LOG(logger_, level, message, ...)                           \
  hydrolib::logger::LoggingCase(logger_ __VA_OPT__(, ) __VA_ARGS__) \
      .WriteLog(level, message __VA_OPT__(, ) __VA_ARGS__)

#define LOG_DEBUG(logger_, message, ...) \
  LOG(logger_, hydrolib::logger::LogLevel::DEBUG, message, __VA_ARGS__)
#define LOG_INFO(logger_, message, ...) \
  LOG(logger_, hydrolib::logger::LogLevel::INFO, message, __VA_ARGS__)
#define LOG_WARNING(logger_, message, ...) \
  LOG(logger_, hydrolib::logger::LogLevel::WARNING, message, __VA_ARGS__)
#define LOG_ERROR(logger_, message, ...) \
  LOG(logger_, hydrolib::logger::LogLevel::ERROR, message, __VA_ARGS__)
#define LOG_CRITICAL(logger_, message, ...) \
  LOG(logger_, hydrolib::logger::LogLevel::CRITICAL, message, __VA_ARGS__)

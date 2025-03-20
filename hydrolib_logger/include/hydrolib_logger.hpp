#ifndef HYDROLIB_LOGGER_H_
#define HYDROLIB_LOGGER_H_

#include <cstdint>

#include "hydrolib_common.h"
#include "hydrolib_strings.hpp"

namespace hydrolib::Logger
{
    enum class LogLevel
    {
        DEBUG = 0,
        INFO = 1,
        WARNING = 2,
        ERROR = 3,
        CRITICAL = 4
    };

    class Logger;
    class LogDistributor;
    class LogTranslator;

    class Logger
    {
        friend class LogDistributor;

    public:
        constexpr static uint32_t MAX_NAME_LENGTH = 50;

    public:
        struct Log
        {
        public:
            strings::StaticFormatableString message;
            LogLevel level;
            strings::CString<MAX_NAME_LENGTH> *process_name;
            uint32_t process_id;
        };

    public:
        explicit Logger(char *name, LogDistributor *distributor = nullptr);

    public:
        hydrolib_ReturnCode WriteLog(LogLevel level, strings::StaticFormatableString message);
        Log GetLastLog();

    private:
        strings::CString<MAX_NAME_LENGTH> name_;
        uint32_t id_;

        LogDistributor *distributor_;

        Log last_log_;
    };

    class LogDistributor
    {
    public:
        constexpr static int32_t ADDING_ERROR = -1;

    private:
        constexpr static int32_t ALL_PUBLISHERS_ = -1;

    private:
        constexpr static uint32_t MAX_SUBSCRIBERS_COUNT_ = 10;
        constexpr static uint32_t MAX_PUBLISHERS_COUNT_ = 10;

    public:
        class SubscriberQueueInterface
        {
        public:
            virtual hydrolib_ReturnCode Push(Logger::Log &log) = 0;
        };

    private:
        struct Subscriber_
        {
        public:
            SubscriberQueueInterface *queue;
            LogLevel level_filter;
            int32_t publisher_filter;
        };

    public:
        LogDistributor();

    public:
        int32_t AddSubscriber(SubscriberQueueInterface &queue, LogLevel level_filter,
                              Logger *publisher);
        int32_t AddPublisher(Logger &publisher);

        void DistributeLog(Logger::Log &log);

    private:
        uint32_t publishers_count_;
        Logger *publishers_[MAX_PUBLISHERS_COUNT_];

        Subscriber_ subscribers_[MAX_SUBSCRIBERS_COUNT_];
        uint32_t subscribers_count_;
    };

    class LogTranslator
    {
    public:
        constexpr static uint32_t MAX_FORMAT_STRING_LENGTH = 20;
        constexpr static int32_t TRANSLATION_ERROR = -1;

        constexpr static char DEBUG_STR[] = "DEBUG";
        constexpr static char INFO_STR[] = "INFO";
        constexpr static char WARNING_STR[] = "WARNING";
        constexpr static char ERROR_STR[] = "ERROR";
        constexpr static char CRITICAL_STR[] = "CRITICAL";

    public:
        enum SpecialSymbols
        {
            MESSAGE = 'm',
            SOURCE_PROCESS = 's',
            LEVEL = 'l'
        };

    public:
        LogTranslator();

    public:
        void StartTranslatingToBytes(Logger::Log &log);
        int32_t DoTranslation(char *buffer, uint32_t length);
        void ResetTranslation();
        void SetFormatString(const char *format_string);

    private:
        int32_t TranslateMessage_(char *buffer, uint32_t length);
        int32_t TranslateLevel_(char *buffer, uint32_t length);
        int32_t TranslateSource_(char *buffer, uint32_t length);

    private:
        char format_string_[MAX_FORMAT_STRING_LENGTH];
        Logger::Log current_log_;

        uint32_t format_index_;
        uint32_t format_subindex_;
    };
}

#endif

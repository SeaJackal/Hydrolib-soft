#ifndef HYDROLIB_LOG_OBSERVER_H_
#define HYDROLIB_LOG_OBSERVER_H_

#include <cstdint>

#include "hydrolib_logger.hpp"

namespace hydrolib::Logger
{
    // template <typename T>
    // concept QueueConcept = requires(T queue, const void *data, void *buffer) {
    //     { queue.Push(data) } -> std::same_as<hydrolib_ReturnCode>;
    //     { queue.Pull(buffer) } -> std::same_as<hydrolib_ReturnCode>;
    // };

    class NoObserver
    {
    public:
        template <typename... Ts>
        void Notify(unsigned source_id, Log &log,
                    [[maybe_unused]] Ts... others)
        {
            (void)source_id;
            (void)log;
        }
    };

    template <strings::ByteStreamConcept Stream, LogObserverConcept Observer = NoObserver>
    class LogObserver
    {
    public:
        static constexpr size_t MAX_LOGGERS_COUNT = 50; // TODO make template

    public:
        LogObserver(Stream &stream, char *format_string, Observer *next_observer = nullptr);

    public:
        template <typename... Ts>
        void Notify(unsigned source_id, Log &log, Ts... params)
        {
            if (level_filter_[source_id] != LogLevel::NO_LEVEL &&
                log.level >= level_filter_[source_id])
            {
                log.ToBytes(format_string_, output_stream_, params...);
            }

            if (next_observer_)
            {
                next_observer_->Notify(source_id, log, params...);
            }
        }

        void SetFormatString(char *format_string);
        void SetLogFiltration(unsigned logger_id, LogLevel level);

    private:
        LogLevel level_filter_[MAX_LOGGERS_COUNT];
        Stream &output_stream_;
        char *format_string_; // TODO Make CString (and add coping)

        Observer *next_observer_;
    };

    template <strings::ByteStreamConcept Stream, LogObserverConcept Observer>
    LogObserver<Stream, Observer>::LogObserver(Stream &stream,
                                               char *format_string,
                                               Observer *next_observer)
        : output_stream_(stream),
          format_string_(format_string),
          next_observer_(next_observer)

    {
        for (size_t i = 0; i < MAX_LOGGERS_COUNT; i++)
        {
            level_filter_[i] = LogLevel::NO_LEVEL;
        }
    }

    template <strings::ByteStreamConcept Stream, LogObserverConcept Observer>
    void LogObserver<Stream, Observer>::SetFormatString(char *format_string)
    {
        format_string_ = format_string;
    }

    template <strings::ByteStreamConcept Stream, LogObserverConcept Observer>
    void LogObserver<Stream, Observer>::SetLogFiltration(unsigned logger_id, LogLevel level)
    {
        level_filter_[logger_id] = level;
    }
}

#endif

#ifndef HYDROLIB_LOG_OBSERVER_H_
#define HYDROLIB_LOG_OBSERVER_H_

#include <cstdint>

#include "hydrolib_logger.hpp"

namespace hydrolib::Logger
{
    template <typename T>
    concept QueueConcept = requires(T queue, const void *data, void *buffer) {
        { queue.Push(data) } -> std::same_as<hydrolib_ReturnCode>;
        { queue.Pull(buffer) } -> std::same_as<hydrolib_ReturnCode>;
    };

    template <strings::ByteStreamConcept Stream, QueueConcept Queue> //TODO Try to make specialization without queue
    class LogObserver
    {
    public:
        static constexpr size_t MAX_LOGGERS_COUNT = 50; // TODO make template

    public:
        LogObserver(Stream &stream, Queue &queue, char *format_string);

    public:
        void Notify(const void *source, const void *data);
        void Process();
        void SetFormatString(char *format_string);
        void SetLogFiltration(unsigned logger_id, LogLevel level);

    private:
        LogLevel level_filter_[MAX_LOGGERS_COUNT];
        Stream &output_stream_;
        Queue &queue_;
        char *format_string_; // TODO Make CString (and add coping)
    };

    template <strings::ByteStreamConcept Stream, QueueConcept Queue>
    LogObserver<Stream, Queue>::LogObserver(Stream &stream, Queue &queue,
                                            char *format_string)
        : output_stream_(stream),
          queue_(queue),
          format_string_(format_string)
    {
        for (size_t i = 0; i < MAX_LOGGERS_COUNT; i++)
        {
            level_filter_[i] = LogLevel::NO_LEVEL;
        }
    }

    template <strings::ByteStreamConcept Stream, QueueConcept Queue>
    void LogObserver<Stream, Queue>::Notify(const void *source_ptr, const void *data_ptr)
    {
        unsigned source_id = *static_cast<const unsigned *>(source_ptr);
        const Log &data = *static_cast<const Log *>(data_ptr);

        if (level_filter_[source_id] != LogLevel::NO_LEVEL &&
            data.level >= level_filter_[source_id])
        {
            queue_.Push(data_ptr);
        }
    }

    template <strings::ByteStreamConcept Stream, QueueConcept Queue>
    void LogObserver<Stream, Queue>::Process()
    {
        Log log;
        if (queue_.Pull(&log) != HYDROLIB_RETURN_OK)
        {
            return;
        }

        log.ToBytes(format_string_, output_stream_);
    }

    template <strings::ByteStreamConcept Stream, QueueConcept Queue>
    void LogObserver<Stream, Queue>::SetFormatString(char *format_string)
    {
        format_string_ = format_string;
    }

    template <strings::ByteStreamConcept Stream, QueueConcept Queue>
    void LogObserver<Stream, Queue>::SetLogFiltration(unsigned logger_id, LogLevel level)
    {
        level_filter_[logger_id] = level;
    }
}

#endif

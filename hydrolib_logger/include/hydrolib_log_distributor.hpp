#ifndef HYDROLIB_LOG_DISTRIBUTOR_H_
#define HYDROLIB_LOG_DISTRIBUTOR_H_

#include <cstdint>

namespace hydrolib::Logger
{
    template <typename T>
    concept SynchronousByteStreamConcept =
        strings::ByteStreamConcept<T> &&
        requires(T stream) {
            { stream.Open() } -> std::same_as<hydrolib_ReturnCode>;
            { stream.Close() } -> std::same_as<hydrolib_ReturnCode>;
        };

    template <SynchronousByteStreamConcept... Streams>
    class LogDistributor
    {
    public:
        static constexpr size_t MAX_LOGGERS_COUNT = 50; // TODO make template

    public:
        consteval LogDistributor(char *format_string, Streams &...streams);

    public:
        template <typename... Ts>
        void Notify(unsigned source_id, Log &log, Ts... params) const
        {
            distributing_list_.head_node->Notify(source_id, log, params...);
        }

        hydrolib_ReturnCode SetFilter(unsigned stream_number, unsigned logger_id,
                                      LogLevel level);
        hydrolib_ReturnCode SetAllFilters(unsigned logger_id, LogLevel level);

    private:
        template <typename Observer,
                  SynchronousByteStreamConcept... Streams_>
        class LogDistributingNode_;

        template <typename NextNode,
                  SynchronousByteStreamConcept Stream,
                  SynchronousByteStreamConcept... Streams_>
        class LogDistributingNode_<NextNode, Stream, Streams_...>
            : public LogDistributingNode_<LogDistributingNode_<NextNode, Stream, Streams_...>,
                                          Streams_...>
        {
        public:
            consteval LogDistributingNode_(char *format_string,
                                           NextNode *next_node,
                                           Stream &stream,
                                           Streams_ &...streams)
                : LogDistributingNode_<LogDistributingNode_<NextNode, Stream, Streams_...>,
                                       Streams_...>(format_string,
                                                    this, streams...),
                  output_stream_(stream),
                  format_string_(format_string),
                  next_node_(next_node)

            {
                for (size_t i = 0; i < MAX_LOGGERS_COUNT; i++)
                {
                    level_filter_[i] = LogLevel::NO_LEVEL;
                }
            }

        public:
            template <typename... Ts>
            void Notify(unsigned source_id, Log &log, Ts... params) const
            {
                if (level_filter_[source_id] != LogLevel::NO_LEVEL &&
                    log.level >= level_filter_[source_id])
                {
                    output_stream_.Open();
                    log.ToBytes(format_string_, output_stream_, params...);
                    output_stream_.Close();
                }
                if (next_node_) // TODO search for if constexpr
                {
                    next_node_->Notify(source_id, log, params...);
                }
            }

            hydrolib_ReturnCode SetFormatString(unsigned depth, char *format_string)
            {
                if (depth == 0)
                {
                    format_string_ = format_string;
                    return HYDROLIB_RETURN_OK;
                }
                if (next_node_)
                {
                    return next_node_->SetFormatString(depth - 1, format_string);
                }
                else
                {
                    return HYDROLIB_RETURN_FAIL;
                }
            }

            hydrolib_ReturnCode SetLogFiltration(unsigned depth, unsigned logger_id, LogLevel level)
            {
                if (depth == 0)
                {
                    level_filter_[logger_id] = level;
                    return HYDROLIB_RETURN_OK;
                }
                if (next_node_)
                {
                    return next_node_->SetLogFiltration(depth - 1, logger_id, level);
                }
                else
                {
                    return HYDROLIB_RETURN_FAIL;
                }
            }

            void SetLogFiltrationsForAll(unsigned logger_id, LogLevel level)
            {
                level_filter_[logger_id] = level;
                if (next_node_)
                {
                    next_node_->SetLogFiltrationsForAll(logger_id, level);
                }
            }

        private:
            LogLevel level_filter_[MAX_LOGGERS_COUNT];
            Stream &output_stream_;
            char *format_string_; // TODO Make CString (and add coping)

            NextNode *next_node_;
        };

        template <typename NextNode>
        class LogDistributingNode_<NextNode>
        {
        public:
            consteval LogDistributingNode_(
                [[maybe_unused]] char *format_string,
                NextNode *next_node)
                : head_node(next_node)
            {
            }

        public:
            template <typename... Ts>
            void Notify([[maybe_unused]] unsigned source_id,
                        [[maybe_unused]] Log &log,
                        [[maybe_unused]] Ts...) const // TODO workaround, need fix
            {
            }

            hydrolib_ReturnCode SetLogFiltration(
                [[maybe_unused]] unsigned depth,
                [[maybe_unused]] unsigned logger_id,
                [[maybe_unused]] LogLevel level)
            {
                return HYDROLIB_RETURN_OK;
            }

            void SetLogFiltrationsForAll([[maybe_unused]] unsigned logger_id,
                                         [[maybe_unused]] LogLevel level)
            {
            }

        public:
            NextNode *head_node;
        };

    private:
        LogDistributingNode_<LogDistributingNode_<void>, Streams...> distributing_list_;
    };

    template <SynchronousByteStreamConcept... Streams>
    consteval LogDistributor<Streams...>::LogDistributor(char *format_string, Streams &...streams)
        : distributing_list_(format_string, nullptr, streams...)
    {
    }

    template <SynchronousByteStreamConcept... Streams>
    hydrolib_ReturnCode LogDistributor<Streams...>::SetFilter(
        unsigned stream_number, unsigned logger_id,
        LogLevel level)
    {
        if (logger_id >= MAX_LOGGERS_COUNT)
        {
            return HYDROLIB_RETURN_FAIL;
        }
        return distributing_list_.head_node->SetLogFiltration(
            sizeof...(Streams) - 1 - stream_number,
            logger_id,
            level);
    }

    template <SynchronousByteStreamConcept... Streams>
    hydrolib_ReturnCode LogDistributor<Streams...>::SetAllFilters(
        unsigned logger_id, LogLevel level)
    {
        if (logger_id >= MAX_LOGGERS_COUNT)
        {
            return HYDROLIB_RETURN_FAIL;
        }
        distributing_list_.head_node->SetLogFiltrationsForAll(
            logger_id,
            level);
        return HYDROLIB_RETURN_OK;
    }

}

#endif

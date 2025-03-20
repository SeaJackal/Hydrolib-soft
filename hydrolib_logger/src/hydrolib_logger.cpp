#include "hydrolib_logger.hpp"

#include <cstring>

namespace hydrolib::Logger
{
    LogDistributor::LogDistributor() : publishers_count_(0),
                                       subscribers_count_(0)
    {
    }

    int32_t LogDistributor::AddSubscriber(SubscriberQueueInterface &queue, LogLevel level_filter,
                                          Logger *publisher)
    {
        if (subscribers_count_ == MAX_SUBSCRIBERS_COUNT_)
        {
            return ADDING_ERROR;
        }
        subscribers_[subscribers_count_].level_filter = level_filter;
        if (publisher)
        {
            subscribers_[subscribers_count_].publisher_filter = publisher->id_;
        }
        else
        {
            subscribers_[subscribers_count_].publisher_filter = ALL_PUBLISHERS_;
        }
        subscribers_[subscribers_count_].queue = &queue;

        subscribers_count_++;

        return subscribers_count_ - 1;
    }

    int32_t LogDistributor::AddPublisher(Logger &publisher)
    {
        if (publishers_count_ == MAX_PUBLISHERS_COUNT_)
        {
            return ADDING_ERROR;
        }
        publishers_[publishers_count_] = &publisher;
        publisher.id_ = publishers_count_;

        publishers_count_++;

        return publishers_count_ - 1;
    }

    void LogDistributor::DistributeLog(Logger::Log &log)
    {
        for (int i = 0; i < subscribers_count_; i++)
        {
            if (log.level >= subscribers_[i].level_filter)
            {
                if (subscribers_[i].publisher_filter == ALL_PUBLISHERS_ ||
                    log.process_id == subscribers_[i].publisher_filter)
                {
                    subscribers_[i].queue->Push(log);
                }
            }
        }
    }

    Logger::Logger(char *name, LogDistributor *distributor)
        : name_(name),
          distributor_(distributor)
    {
        if (distributor)
        {
            id_ = distributor->AddPublisher(*this);
        }
    }

    hydrolib_ReturnCode Logger::WriteLog(LogLevel level, strings::StaticFormatableString message)
    {
        last_log_.level = level;
        last_log_.message = message;
        last_log_.process_id = id_;
        last_log_.process_name = &name_;

        if (distributor_)
        {
            distributor_->DistributeLog(last_log_);
        }

        return HYDROLIB_RETURN_OK;
    }

    Logger::Log Logger::GetLastLog()
    {
        return last_log_;
    }

    LogTranslator::LogTranslator() : format_index_(0),
                                     format_subindex_(0)
    {
    }

    void LogTranslator::StartTranslatingToBytes(Logger::Log &log)
    {
        current_log_ = log;
        format_index_ = 0;
        format_subindex_ = 0;
    }

    int32_t LogTranslator::DoTranslation(char *buffer, uint32_t length)
    {
        uint32_t translated_length = 0;
        while (translated_length < length)
        {
            if (format_string_[format_index_] == '%' || format_subindex_ != 0)
            {
                if (format_string_[format_index_] == '%')
                {
                    format_index_++;
                }
                switch (format_string_[format_index_])
                {
                case SpecialSymbols::MESSAGE:
                {
                    int message_length = TranslateMessage_(buffer + translated_length, length - translated_length);
                    if (message_length == TRANSLATION_ERROR)
                    {
                        return TRANSLATION_ERROR;
                    }
                    translated_length += message_length;
                    break;
                }

                case SpecialSymbols::LEVEL:
                {
                    int level_length = TranslateLevel_(buffer + translated_length, length - translated_length);
                    if (level_length == TRANSLATION_ERROR)
                    {
                        return TRANSLATION_ERROR;
                    }
                    translated_length += level_length;
                    break;
                }
                case SpecialSymbols::SOURCE_PROCESS:
                {
                    int source_length = TranslateSource_(buffer + translated_length, length - translated_length);
                    if (source_length == TRANSLATION_ERROR)
                    {
                        return TRANSLATION_ERROR;
                    }
                    translated_length += source_length;
                    break;
                }
                }
            }
            else if (!format_string_[format_index_])
            {
                break;
            }
            else
            {
                buffer[translated_length] = format_string_[format_index_];
                translated_length++;
                format_index_++;
            }
        }
        return translated_length;
    }

    void LogTranslator::ResetTranslation()
    {
        format_index_ = 0;
        format_subindex_ = 0;
    }

    void LogTranslator::SetFormatString(const char *format_string)
    {
        strcpy(format_string_, format_string);
    }

    int32_t LogTranslator::TranslateMessage_(char *buffer, uint32_t length)
    {
        uint32_t message_length = current_log_.message.GetLength() - format_subindex_;
        if (message_length > length)
        {
            memcpy(buffer, current_log_.message.GetString() + format_subindex_, length);
            format_subindex_ += length;
            return length;
        }
        else
        {
            memcpy(buffer, current_log_.message.GetString() + format_subindex_,
                   message_length);
            format_subindex_ = 0;
            format_index_++;
            return message_length;
        }
    }

    int32_t LogTranslator::TranslateLevel_(char *buffer, uint32_t length)
    {
        const char *level_str;
        uint32_t str_length;
        switch (current_log_.level)
        {
        case LogLevel::DEBUG:
            level_str = DEBUG_STR;
            str_length = sizeof(DEBUG_STR) - 1;
            break;
        case LogLevel::INFO:
            level_str = INFO_STR;
            str_length = sizeof(INFO_STR) - 1;
            break;
        case LogLevel::WARNING:
            level_str = WARNING_STR;
            str_length = sizeof(WARNING_STR) - 1;
            break;
        case LogLevel::ERROR:
            level_str = ERROR_STR;
            str_length = sizeof(ERROR_STR) - 1;
            break;
        case LogLevel::CRITICAL:
            level_str = CRITICAL_STR;
            str_length = sizeof(CRITICAL_STR) - 1;
            break;
        default:
            return TRANSLATION_ERROR;
        };

        uint32_t remaining_length = str_length - format_subindex_;

        if (length > remaining_length)
        {
            memcpy(buffer, level_str + format_subindex_, remaining_length);
            format_subindex_ = 0;
            format_index_++;
            return remaining_length;
        }
        else
        {
            memcpy(buffer, level_str + format_subindex_, length);
            format_subindex_ += length;
            return length;
        }
    }

    int32_t LogTranslator::TranslateSource_(char *buffer, uint32_t length)
    {
        uint32_t source_length = current_log_.process_name->GetLength() - format_subindex_;
        if (source_length > length)
        {
            memcpy(buffer, current_log_.process_name->GetString() + format_subindex_, length);
            format_subindex_ += length;
            return length;
        }
        else
        {
            memcpy(buffer, current_log_.process_name->GetString() + format_subindex_,
                   source_length);
            format_subindex_ = 0;
            format_index_++;
            return source_length;
        }
    }
}

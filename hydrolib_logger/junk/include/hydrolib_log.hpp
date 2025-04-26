#ifndef HYDROLIB_LOG_H_
#define HYDROLIB_LOG_H_

#include <cstdint>
#include <cstring>

#include "hydrolib_common.h"
#include "hydrolib_formatable_string.hpp"
#include "hydrolib_cstring.hpp"

namespace hydrolib::Logger
{

    enum class LogLevel
    {
        NO_LEVEL = 0,
        DEBUG = 1,
        INFO = 2,
        WARNING = 3,
        ERROR = 4,
        CRITICAL = 5
    };

    class Log
    {
    public:
        constexpr static size_t MAX_NAME_LENGTH = 50;
        constexpr static size_t MAX_FORMAT_STRING_LENGTH = 20;
        constexpr static int TRANSLATION_ERROR = -1;

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
        template <strings::ByteStreamConcept DestType>
        hydrolib_ReturnCode ToBytes(const char *format_string, DestType &buffer) const;

    private:
        template <strings::ByteStreamConcept DestType>
        hydrolib_ReturnCode TranslateMessage_(DestType &buffer) const;

        template <strings::ByteStreamConcept DestType>
        hydrolib_ReturnCode TranslateLevel_(DestType &buffer) const;

        template <strings::ByteStreamConcept DestType>
        hydrolib_ReturnCode TranslateSource_(DestType &buffer) const;

        template <strings::ByteStreamConcept DestType>
        hydrolib_ReturnCode BurstPush_(
            DestType &buffer,
            const char *burst_start, unsigned &burst_length) const;

    public:
        strings::StaticFormatableString message;
        LogLevel level;
        const strings::CString<MAX_NAME_LENGTH> *process_name;
    };

    template <strings::ByteStreamConcept DestType>
    hydrolib_ReturnCode Log::ToBytes(const char *format_string, DestType &buffer) const
    {
        unsigned format_index = 0;

        const char *burst_start = nullptr;
        unsigned burst_length = 0;

        while (format_string[format_index])
        {
            if (format_string[format_index] == '%')
            {
                hydrolib_ReturnCode burst_res = BurstPush_(buffer, burst_start, burst_length);
                if (burst_res != HYDROLIB_RETURN_OK)
                {
                    return burst_res;
                }

                format_index++;

                switch (format_string[format_index])
                {
                case SpecialSymbols::MESSAGE:
                {
                    hydrolib_ReturnCode message_res = TranslateMessage_(buffer);
                    if (message_res != HYDROLIB_RETURN_OK)
                    {
                        return message_res;
                    }
                    format_index++;
                    break;
                }

                case SpecialSymbols::LEVEL:
                {
                    hydrolib_ReturnCode level_res = TranslateLevel_(buffer);
                    if (level_res != HYDROLIB_RETURN_OK)
                    {
                        return level_res;
                    }
                    format_index++;
                    break;
                }
                case SpecialSymbols::SOURCE_PROCESS:
                {
                    hydrolib_ReturnCode source_res = TranslateSource_(buffer);
                    if (source_res != HYDROLIB_RETURN_OK)
                    {
                        return source_res;
                    }
                    format_index++;
                    break;
                }
                default:
                    return HYDROLIB_RETURN_FAIL;
                }
            }
            else
            {
                if (burst_length == 0)
                {
                    burst_start = format_string + format_index;
                }
                burst_length++;
                format_index++;
            }
        }
        hydrolib_ReturnCode burst_res = BurstPush_(buffer, burst_start, burst_length);
        if (burst_res != HYDROLIB_RETURN_OK)
        {
            return burst_res;
        }
        return HYDROLIB_RETURN_OK;
    }

    template <strings::ByteStreamConcept DestType>
    hydrolib_ReturnCode Log::TranslateMessage_(DestType &buffer) const
    {
        unsigned message_length = message.GetLength();

        return buffer.Push(reinterpret_cast<const uint8_t *>(message.GetString()), message_length);
    }

    template <strings::ByteStreamConcept DestType>
    hydrolib_ReturnCode Log::TranslateLevel_(DestType &buffer) const
    {
        const char *level_str;
        unsigned str_length;
        switch (level)
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
            return HYDROLIB_RETURN_FAIL;
        };

        return buffer.Push(reinterpret_cast<const uint8_t *>(level_str), str_length);
    }

    template <strings::ByteStreamConcept DestType>
    hydrolib_ReturnCode Log::TranslateSource_(DestType &buffer) const
    {
        unsigned source_length = process_name->GetLength();

        return buffer.Push(
            reinterpret_cast<const uint8_t *>(process_name->GetConstString()),
            source_length);
    }

    template <strings::ByteStreamConcept DestType>
    hydrolib_ReturnCode Log::BurstPush_(
        DestType &buffer,
        const char *burst_start, unsigned &burst_length) const
    {
        if (burst_length != 0)
        {
            hydrolib_ReturnCode burst_res =
                buffer.Push(reinterpret_cast<const uint8_t *>(burst_start), burst_length);
            if (burst_res != HYDROLIB_RETURN_OK)
            {
                return burst_res;
            }
            burst_length = 0;
        }
        return HYDROLIB_RETURN_OK;
    }

}

#endif

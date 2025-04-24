#ifndef HYDROLIB_LOGGER_H_
#define HYDROLIB_LOGGER_H_

#include <cstdint>

#include "hydrolib_common.h"
#include "hydrolib_observer.hpp"
#include "hydrolib_strings.hpp"
#include "hydrolib_log.hpp"

namespace hydrolib::Logger
{
    template <observer::Observer auto &...OBSERVERS>
    class Logger
    {
    public:
        constexpr Logger(const char *name, unsigned id) : name_(name),
                                                          id_(id)
        {
        }

    public:
        void WriteLog(LogLevel level, strings::StaticFormatableString message)
        {
            Log log;
            log.message = message;
            log.level = level;
            log.process_name = &name_;

            observer::NotifyAll(&id_, &log, OBSERVERS...);
        }

    private:
        const strings::CString<Log::MAX_NAME_LENGTH> name_;
        const unsigned id_;
    };
}

#endif

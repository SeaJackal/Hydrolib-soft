#ifndef HYDROLIB_LOGGER_H_
#define HYDROLIB_LOGGER_H_

#include <cstdint>

#include "hydrolib_common.h"
#include "hydrolib_observer.hpp"
#include "hydrolib_formatable_string.hpp"
#include "hydrolib_cstring.hpp"
#include "hydrolib_log.hpp"

namespace hydrolib::Logger
{
    template <typename T, typename... Ts>
    concept LogObserverConcept = requires(T observer, unsigned source_id, Log &log, Ts... params) {
        observer.Notify(source_id, log, params...);
    };

    template <LogObserverConcept Observer>
    class Logger
    {
    public:
        constexpr Logger(const char *name, unsigned id, Observer &observer)
            : name_(name),
              id_(id),
              observer_(observer)
        {
        }

    public:
        template <typename... Ts>
        void WriteLog(LogLevel level, strings::StaticFormatableString message, Ts... params)
        {
            Log log{
                .message = message,
                .level = level,
                .process_name = &name_};

            observer_.Notify(id_, log, params...);
        }

    private:
        const strings::CString<Log::MAX_NAME_LENGTH> name_;
        const unsigned id_;

        Observer &observer_;
    };
}

#endif

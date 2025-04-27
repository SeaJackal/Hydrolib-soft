#ifndef HYDROLIB_LOGGER_H_
#define HYDROLIB_LOGGER_H_

#include <cstdint>
#include <concepts>

#include "hydrolib_common.h"
#include "hydrolib_formatable_string.hpp"
#include "hydrolib_cstring.hpp"
#include "hydrolib_log.hpp"

namespace hydrolib::Logger
{
    template <typename T, typename... Ts>
    concept LogDistributorConcept = requires(T distributor, unsigned source_id, Log &log, Ts... params) {
        distributor.Notify(source_id, log, params...);
    };

    template <LogDistributorConcept Distributor>
    class Logger
    {
    public:
        consteval Logger(const char *name, unsigned id, const Distributor &distributor)
            : name_(name),
              id_(id),
              distributor_(distributor)
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

            distributor_.Notify(id_, log, params...);
        }

    private:
        const strings::CString<Log::MAX_NAME_LENGTH> name_;
        const unsigned id_;

        const Distributor &distributor_;
    };
}

#endif

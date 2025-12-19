#pragma once

#include "hydrolib_log_distributor.hpp"
#include "hydrolib_logger.hpp"

namespace hydrolib::logger {
struct CoutStream {};

int write([[maybe_unused]] CoutStream &stream, const void *source,
          unsigned length);

extern Logger<LogDistributor<CoutStream>> mock_logger;
extern LogDistributor<CoutStream> mock_distributor;
}  // namespace hydrolib::logger

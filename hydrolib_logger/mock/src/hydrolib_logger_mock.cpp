#include "hydrolib_logger_mock.hpp"

#include <iostream>

namespace hydrolib::logger
{

constexpr char *kDefaultFormat = "[%s] [%l] %m\n";
constinit CoutStream cout_stream{};
constinit LogDistributor<CoutStream> mock_distributor{kDefaultFormat,
                                                      cout_stream};
constinit Logger<LogDistributor<CoutStream>> mock_logger{"Mock", 0,
                                                         mock_distributor};

int write([[maybe_unused]] CoutStream &stream, const void *source,
          unsigned length)
{
    const char *chars = reinterpret_cast<const char *>(source);
    for (unsigned i = 0; i < length; i++)
    {
        std::cout << chars[i];
    }
    return static_cast<int>(length);
}

} // namespace hydrolib::logger

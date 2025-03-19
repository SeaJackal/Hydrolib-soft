cmake_minimum_required(VERSION 3.10)

project(Hydrolib VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(hydrolib_common)
add_subdirectory(hydrolib_ring_queue)
add_subdirectory(hydrolib_serial_protocol)
add_subdirectory(hydrolib_strings)
add_subdirectory(hydrolib_logger)

enable_testing()

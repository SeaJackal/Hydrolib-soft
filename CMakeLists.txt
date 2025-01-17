cmake_minimum_required(VERSION 3.10)

project(HydrolibRingBuffer VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(hydrolib_ring_queue)

enable_testing()

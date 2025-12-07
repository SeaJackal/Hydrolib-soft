cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(HydrolibStreams INTERFACE)

target_include_directories(HydrolibStreams INTERFACE include)

include(CTest)

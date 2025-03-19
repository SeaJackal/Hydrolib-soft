cmake_minimum_required(VERSION 3.10)

project(HydrolibStrings VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(HydrolibStrings src/hydrolib_strings.cpp)

target_include_directories(HydrolibStrings PUBLIC include)

include(CTest)
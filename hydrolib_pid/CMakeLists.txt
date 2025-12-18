cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)

add_library(HydrolibPID INTERFACE)

target_include_directories(HydrolibPID INTERFACE include)

include(${CMAKE_SOURCE_DIR}/cmake/HydrolibGTest.cmake)
hydrolib_add_tests_for_target(HydrolibPID)

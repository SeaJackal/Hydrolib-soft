cmake_minimum_required(VERSION 3.10)

project(HydrolibSerialProtocol VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)

include_directories(include)

add_library(HydrolibSerialProtocol src/hydrolib_serial_protocol_core.cpp)

target_link_libraries(HydrolibSerialProtocol HydrolibCommon)

enable_testing()

find_package(GTest REQUIRED)
include(GoogleTest)

file(GLOB TESTS tests/*.cpp)
add_executable(TestSerialProtocol tests/test_hydrolib_sp_core_env.cpp)
set_target_properties(TestSerialProtocol
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Testing"
)
target_compile_features(TestSerialProtocol PUBLIC cxx_std_20)
target_compile_options(TestSerialProtocol PRIVATE -Wall -Wextra -Wpedantic -Wc++20-compat -Wno-format-security
    -Woverloaded-virtual -Wsuggest-override)
target_link_libraries(TestSerialProtocol HydrolibSerialProtocol GTest::gtest GTest::gtest_main -pthread)
gtest_discover_tests(TestSerialProtocol)

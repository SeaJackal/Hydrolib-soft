cmake_minimum_required(VERSION 3.10)

project(HydrolibLogger VERSION 1.0)

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(HydrolibLogger src/hydrolib_logger.cpp)

target_include_directories(HydrolibLogger PUBLIC include)

target_link_libraries(HydrolibLogger HydrolibCommon HydrolibStrings)

include(CTest)

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    file(GLOB TESTS tests/*.cpp)
    add_executable(TestLogger ${TESTS})
    set_target_properties(TestLogger
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Testing"
    )
    target_compile_features(TestLogger PUBLIC cxx_std_20)
    target_compile_options(TestLogger PRIVATE -Wall -Wextra -Wpedantic -Wc++20-compat -Wno-format-security
        -Woverloaded-virtual -Wsuggest-override)
    target_link_libraries(TestLogger HydrolibLogger GTest::gtest GTest::gtest_main -pthread)
    gtest_discover_tests(TestLogger)
endif()

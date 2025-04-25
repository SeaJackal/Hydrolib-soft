cmake_minimum_required(VERSION 3.10)

project(HydrolibStrings VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(HydrolibStrings INTERFACE)

target_include_directories(HydrolibStrings INTERFACE include)

target_link_libraries(HydrolibStrings INTERFACE HydrolibCommon)

include(CTest)

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    file(GLOB TESTS tests/*.cpp)
    add_executable(TestStrings ${TESTS})
    set_target_properties(TestStrings
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Testing"
    )
    target_compile_features(TestStrings PUBLIC cxx_std_20)
    target_compile_options(TestStrings PRIVATE -Wall -Wextra -Wpedantic -Wc++20-compat -Wno-format-security
        -Woverloaded-virtual -Wsuggest-override)
    target_link_libraries(TestStrings HydrolibStrings GTest::gtest GTest::gtest_main -pthread)
    gtest_discover_tests(TestStrings)
endif()

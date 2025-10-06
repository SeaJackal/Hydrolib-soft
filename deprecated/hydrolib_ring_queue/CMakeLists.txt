cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(HydrolibRingQueue INTERFACE)

target_include_directories(HydrolibRingQueue INTERFACE include)

target_link_libraries(HydrolibRingQueue INTERFACE HydrolibReturnCodes)

include(CTest)

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    file(GLOB TESTS tests/* .cpp)
    add_executable(TestRingBuffer ${TESTS})
    set_target_properties(TestRingBuffer
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Testing"
    )
    target_compile_features(TestRingBuffer PUBLIC cxx_std_20)
    target_compile_options(TestRingBuffer PRIVATE -Wall -Wextra -Wpedantic -Wc++20-compat -Wno-format-security
        -Woverloaded-virtual -Wsuggest-override)
    target_link_libraries(TestRingBuffer HydrolibRingQueue GTest::gtest GTest::gtest_main -pthread)
    gtest_discover_tests(TestRingBuffer)
endif()

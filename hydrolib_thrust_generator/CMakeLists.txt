set(LIBRARY_NAME HydrolibThrustGenerator)

add_library(${LIBRARY_NAME} INTERFACE)

target_include_directories(${LIBRARY_NAME} INTERFACE include)

target_link_libraries(${LIBRARY_NAME} INTERFACE HydrolibThrustersControl)

include(CTest)

if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    include(GoogleTest)

    file(GLOB TESTS tests/*.cpp)

    add_executable(TestThrustGenerator ${TESTS})
    set_target_properties(TestThrustGenerator
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Testing"
    )
    target_compile_features(TestThrustGenerator PUBLIC cxx_std_20)

    target_compile_options(TestThrustGenerator PRIVATE -Wall -Wextra -Wpedantic -Wc++20-compat -Wno-format-security
        -Woverloaded-virtual -Wsuggest-override -fsanitize=address -fsanitize=undefined)
    target_link_options(TestThrustGenerator PRIVATE -fsanitize=address -fsanitize=undefined)
    target_link_libraries(TestThrustGenerator ${LIBRARY_NAME} GTest::gtest GTest::gtest_main -pthread)
    gtest_discover_tests(TestThrustGenerator)
endif()

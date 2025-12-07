set(LIBRARY_NAME HydrolibLogger)
set(TEST_TARGET_NAME Test${LIBRARY_NAME})

message(STATUS "Logger type: ${LOGGER_TYPE}")
if(LOGGER_TYPE STREQUAL "hydrolib")
    add_library(${LIBRARY_NAME} INTERFACE)
    target_include_directories(${LIBRARY_NAME} INTERFACE common/include)
    target_include_directories(${LIBRARY_NAME} INTERFACE specific/hydrolib)
    target_link_libraries(${LIBRARY_NAME} INTERFACE HydrolibReturnCodes INTERFACE HydrolibStrings INTERFACE HydrolibStreams)

    if(BUILD_TESTS)
        include(CTest)

        find_package(GTest REQUIRED)
        include(GoogleTest)

        file(GLOB TESTS tests/*.cpp)
        add_executable(${TEST_TARGET_NAME} ${TESTS})
        set_target_properties(${TEST_TARGET_NAME}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Testing"
        )
        target_compile_features(${TEST_TARGET_NAME} PUBLIC cxx_std_20)
        target_compile_options(${TEST_TARGET_NAME} PRIVATE 
            -Wall -Wextra -Wpedantic -Wc++20-compat -Wno-format-security
            -Woverloaded-virtual -Wsuggest-override -fsanitize=address -fsanitize=undefined)
        target_link_options(${TEST_TARGET_NAME} PRIVATE -fsanitize=address -fsanitize=undefined)
        target_link_libraries(${TEST_TARGET_NAME} HydrolibLogger GTest::gtest GTest::gtest_main -pthread)
        gtest_discover_tests(${TEST_TARGET_NAME})

        add_subdirectory(mock)
    endif()

elseif(LOGGER_TYPE STREQUAL "none")
    add_library(${LIBRARY_NAME} INTERFACE)
    target_include_directories(${LIBRARY_NAME} INTERFACE specific/none)
elseif(NOT LOGGER_TYPE STREQUAL "custom")
    message(ERROR 
        "Invalid logger type: ${LOGGER_TYPE}, supported types are: hydrolib, none, custom")
endif()

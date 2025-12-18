function(hydrolib_add_tests_for_target TESTED_TARGET)
    if(BUILD_TESTS)
        include(CTest)

        find_package(GTest REQUIRED)
        include(GoogleTest)

        file(GLOB TESTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")

        set(TEST_EXECUTABLE_NAME "Test${TESTED_TARGET}")
        set(HYDROLIB_TEST_TARGET ${TEST_EXECUTABLE_NAME} PARENT_SCOPE)

        add_executable(${TEST_EXECUTABLE_NAME} ${TESTS})
        set_target_properties(${TEST_EXECUTABLE_NAME}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Testing"
        )
        target_compile_features(${TEST_EXECUTABLE_NAME} PUBLIC cxx_std_20)

        target_compile_options(${TEST_EXECUTABLE_NAME} PRIVATE --coverage -Wall -Wextra -Wpedantic -Wc++20-compat
            -Wno-format-security -Woverloaded-virtual -Wsuggest-override -fno-exceptions -fno-rtti -fsanitize=address -fsanitize=undefined)
        target_link_options(${TEST_EXECUTABLE_NAME} PRIVATE --coverage -fsanitize=address -fsanitize=undefined)
        target_link_libraries(${TEST_EXECUTABLE_NAME} ${TESTED_TARGET} GTest::gtest GTest::gtest_main -pthread)

        gtest_discover_tests(${TEST_EXECUTABLE_NAME})
    endif()
endfunction()



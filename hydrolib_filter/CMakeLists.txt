set(LIBRARY_NAME HydrolibFilter)

add_library(${LIBRARY_NAME} INTERFACE)

target_include_directories(${LIBRARY_NAME} INTERFACE include)

include(${HYDROLIB_ROOT_DIR}/cmake/HydrolibGTest.cmake)
hydrolib_add_tests_for_target(${LIBRARY_NAME})

if(BUILD_TESTS)
    target_link_libraries(${HYDROLIB_TEST_TARGET} HydrolibMath)
endif()

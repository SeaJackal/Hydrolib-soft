set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(HydrolibStrings INTERFACE)

target_include_directories(HydrolibStrings INTERFACE include)

target_link_libraries(HydrolibStrings INTERFACE HydrolibReturnCodes INTERFACE HydrolibConcepts
    INTERFACE HydrolibMath INTERFACE HydrolibStreams)

include(${HYDROLIB_ROOT_DIR}/cmake/HydrolibGTest.cmake)
hydrolib_add_tests_for_target(HydrolibStrings)
if(BUILD_TESTS)
    target_link_libraries(${HYDROLIB_TEST_TARGET} HydrolibStreamMock)
endif()

add_library(HydrolibBusApplication INTERFACE)

target_include_directories(HydrolibBusApplication INTERFACE include)

target_link_libraries(HydrolibBusApplication INTERFACE HydrolibConcepts
    INTERFACE HydrolibLogger INTERFACE HydrolibReturnCodes INTERFACE HydrolibStreams)

include(${HYDROLIB_ROOT_DIR}/cmake/HydrolibGTest.cmake)
hydrolib_add_tests_for_target(HydrolibBusApplication)
if(BUILD_TESTS)
    target_link_libraries(${HYDROLIB_TEST_TARGET} HydrolibLoggerMock)
endif()

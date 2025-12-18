add_library(HydrolibBusDatalink INTERFACE)

target_include_directories(HydrolibBusDatalink INTERFACE include)

target_link_libraries(HydrolibBusDatalink INTERFACE HydrolibConcepts
    INTERFACE HydrolibLogger INTERFACE HydrolibReturnCodes INTERFACE HydrolibCRC INTERFACE HydrolibStreams)

include(${CMAKE_SOURCE_DIR}/cmake/HydrolibGTest.cmake)
hydrolib_add_tests_for_target(HydrolibBusDatalink)

target_link_libraries(${HYDROLIB_TEST_TARGET} HydrolibLoggerMock)
target_link_libraries(${HYDROLIB_TEST_TARGET} HydrolibStreamMock)

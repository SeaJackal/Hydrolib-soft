cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(HydrolibVectorNAV INTERFACE)

target_include_directories(HydrolibVectorNAV INTERFACE include)

target_link_libraries(HydrolibVectorNAV INTERFACE HydrolibLogger INTERFACE HydrolibIMU INTERFACE HydrolibReturnCodes INTERFACE HydrolibStreams)

include(${CMAKE_SOURCE_DIR}/cmake/HydrolibGTest.cmake)
hydrolib_add_tests_for_target(HydrolibVectorNAV)
target_link_libraries(${HYDROLIB_TEST_TARGET} HydrolibLogger HydrolibLoggerMock)

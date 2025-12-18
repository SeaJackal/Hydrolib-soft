set(LIBRARY_NAME HydrolibShell)
set(TEST_TARGET_NAME Test${LIBRARY_NAME})

add_library(${LIBRARY_NAME} INTERFACE)

target_include_directories(${LIBRARY_NAME} INTERFACE include)

target_link_libraries(${LIBRARY_NAME} INTERFACE HydrolibConcepts INTERFACE HydrolibStrings INTERFACE HydrolibStreams)

include(${CMAKE_SOURCE_DIR}/cmake/HydrolibGTest.cmake)
hydrolib_add_tests_for_target(${LIBRARY_NAME})
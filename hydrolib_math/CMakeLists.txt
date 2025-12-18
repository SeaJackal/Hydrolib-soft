set(LIBRARY_NAME HydrolibMath)

add_library(${LIBRARY_NAME} INTERFACE)

target_include_directories(${LIBRARY_NAME} INTERFACE include)

include(${CMAKE_SOURCE_DIR}/cmake/HydrolibGTest.cmake)
hydrolib_add_tests_for_target(${LIBRARY_NAME})

set(LIBRARY_NAME HydrolibMath)

add_library(${LIBRARY_NAME} INTERFACE)

target_include_directories(${LIBRARY_NAME} INTERFACE include)

target_link_libraries(${LIBRARY_NAME} INTERFACE HydrolibStreams INTERFACE HydrolibReturnCodes INTERFACE HydrolibStrings)

include(${HYDROLIB_ROOT_DIR}/cmake/HydrolibGTest.cmake)
hydrolib_add_tests_for_target(${LIBRARY_NAME})

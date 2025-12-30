set(LIBRARY_NAME HydrolibRingQueue)

add_library(${LIBRARY_NAME} INTERFACE)

target_include_directories(${LIBRARY_NAME} INTERFACE include)

target_link_libraries(${LIBRARY_NAME} INTERFACE HydrolibReturnCodes)

hydrolib_add_tests_for_target(${LIBRARY_NAME})

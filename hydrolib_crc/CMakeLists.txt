add_library(HydrolibCRC INTERFACE)

target_include_directories(HydrolibCRC INTERFACE include)

include(${HYDROLIB_ROOT_DIR}/cmake/HydrolibGTest.cmake)
hydrolib_add_tests_for_target(HydrolibCRC)

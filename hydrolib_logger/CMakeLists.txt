set(LIBRARY_NAME HydrolibLogger)
set(TEST_TARGET_NAME Test${LIBRARY_NAME})

message(STATUS "Logger type: ${LOGGER_TYPE}")
if(LOGGER_TYPE STREQUAL "hydrolib")
    add_library(${LIBRARY_NAME} INTERFACE)
    target_include_directories(${LIBRARY_NAME} INTERFACE common/include)
    target_include_directories(${LIBRARY_NAME} INTERFACE specific/hydrolib)
    target_link_libraries(${LIBRARY_NAME} INTERFACE HydrolibReturnCodes INTERFACE HydrolibStrings INTERFACE HydrolibStreams)

    include(${CMAKE_SOURCE_DIR}/cmake/HydrolibGTest.cmake)
    hydrolib_add_tests_for_target(${LIBRARY_NAME})
    if(BUILD_TESTS)
        add_subdirectory(mock)
    endif()

elseif(LOGGER_TYPE STREQUAL "none")
    add_library(${LIBRARY_NAME} INTERFACE)
    target_include_directories(${LIBRARY_NAME} INTERFACE specific/none)
elseif(NOT LOGGER_TYPE STREQUAL "custom")
    message(ERROR 
        "Invalid logger type: ${LOGGER_TYPE}, supported types are: hydrolib, none, custom")
endif()

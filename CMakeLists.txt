cmake_minimum_required(VERSION 3.10)

project(Hydrolib VERSION 1.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(HYDROLIB_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

if(BUILD_TESTS)
    include(CTest)
endif()

# add_subdirectory(hydrolib_ring_queue)
# add_subdirectory(hydrolib_serial_protocol)
# add_subdirectory(hydrolib_streambuf)
add_subdirectory(hydrolib_acs)
add_subdirectory(hydrolib_bus/hydrolib_bus_application)
add_subdirectory(hydrolib_bus/hydrolib_bus_datalink)
add_subdirectory(hydrolib_concepts)
add_subdirectory(hydrolib_crc)
add_subdirectory(hydrolib_device)
add_subdirectory(hydrolib_filter)
add_subdirectory(hydrolib_imu)
add_subdirectory(hydrolib_logger)
add_subdirectory(hydrolib_math)
add_subdirectory(hydrolib_pid)
add_subdirectory(hydrolib_pressure_sensor)
add_subdirectory(hydrolib_return_codes)
add_subdirectory(hydrolib_shell_commands)
add_subdirectory(hydrolib_shell)
add_subdirectory(hydrolib_streams)
add_subdirectory(hydrolib_strings)
add_subdirectory(hydrolib_thrust_generator)
add_subdirectory(hydrolib_vectornav)
add_subdirectory(hydrolib_ring_queue)

if(BUILD_TESTS AND COVERAGE)
	find_program(LCOV_EXECUTABLE lcov REQUIRED)
	find_program(GENHTML_EXECUTABLE genhtml REQUIRED)

	add_custom_target(coverage ALL
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/coverage
		COMMAND ${LCOV_EXECUTABLE} --directory ${CMAKE_BINARY_DIR} --zerocounters
		COMMAND ${LCOV_EXECUTABLE} --capture --initial --directory ${CMAKE_BINARY_DIR} --output-file ${CMAKE_BINARY_DIR}/coverage/base.info
		COMMAND ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_BINARY_DIR} --output-on-failure
		COMMAND ${LCOV_EXECUTABLE} --capture --directory ${CMAKE_BINARY_DIR} --output-file ${CMAKE_BINARY_DIR}/coverage/test.info
		COMMAND ${LCOV_EXECUTABLE} --add-tracefile ${CMAKE_BINARY_DIR}/coverage/base.info --add-tracefile ${CMAKE_BINARY_DIR}/coverage/test.info --output-file ${CMAKE_BINARY_DIR}/coverage/coverage.info
		COMMAND ${LCOV_EXECUTABLE} --remove ${CMAKE_BINARY_DIR}/coverage/coverage.info /usr/* ${CMAKE_BINARY_DIR}/* */tests/* */external/* */_deps/* --output-file ${CMAKE_BINARY_DIR}/coverage/coverage.info
		COMMAND ${GENHTML_EXECUTABLE} -o ${CMAKE_BINARY_DIR}/coverage_html ${CMAKE_BINARY_DIR}/coverage/coverage.info --legend --branch-coverage
		COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated in ${CMAKE_BINARY_DIR}/coverage_html/index.html"
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		COMMENT "Generating code coverage report with lcov"
		VERBATIM
	)
	get_property(HYDROLIB_ALL_TEST_TARGETS GLOBAL PROPERTY HYDROLIB_ALL_TEST_TARGETS)
	if(HYDROLIB_ALL_TEST_TARGETS)
		add_dependencies(coverage ${HYDROLIB_ALL_TEST_TARGETS})
	endif()
endif()
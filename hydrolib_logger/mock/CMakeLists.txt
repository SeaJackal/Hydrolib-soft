set(TARGET_NAME HydrolibLoggerMock)

add_library(${TARGET_NAME} STATIC src/hydrolib_logger_mock.cpp)

target_include_directories(${TARGET_NAME} PUBLIC include)

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_20)

target_link_libraries(${TARGET_NAME} PUBLIC HydrolibLogger)

target_compile_features(${TARGET_NAME} PUBLIC cxx_std_20)

target_compile_options(${TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic -Wc++20-compat -Wno-format-security
    -Woverloaded-virtual -Wsuggest-override -fsanitize=address -fsanitize=undefined)
target_link_options(${TARGET_NAME} PRIVATE -fsanitize=address -fsanitize=undefined)

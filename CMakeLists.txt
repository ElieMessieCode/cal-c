cmake_minimum_required(VERSION 3.20)
project(cal-c VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Werror
    )
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Core Library
set(CALC_SOURCES
    src/arena.c
    src/error.c
    src/value.c
    src/vars.c
    src/lexer.c
    src/ast.c
    src/parser.c
    src/eval.c
)

add_library(calc STATIC ${CALC_SOURCES})
target_include_directories(calc PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

if(UNIX AND NOT APPLE)
    target_link_libraries(calc PUBLIC m)
endif()

# CLI Executable
add_executable(calc_cli app/main.c)
set_target_properties(calc_cli PROPERTIES OUTPUT_NAME "calc")
target_link_libraries(calc_cli PRIVATE calc)

# Testing
enable_testing()

set(TEST_SOURCES
    tests/test_arena.c
    tests/test_value.c
    tests/test_vars.c
    tests/test_lexer.c
    tests/test_parser.c
    tests/test_eval.c
    tests/test_error.c
    tests/test_cases.c
)

foreach(TEST_SRC ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SRC} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SRC})
    target_link_libraries(${TEST_NAME} PRIVATE calc)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

add_executable(fuzz_eval fuzz/fuzz_eval.c)
target_link_libraries(fuzz_eval PRIVATE calc)
add_test(NAME fuzz_eval COMMAND fuzz_eval)

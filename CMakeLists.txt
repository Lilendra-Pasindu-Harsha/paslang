cmake_minimum_required(VERSION 3.16)
project(PasLang VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(compiler)

# Core Compiler Library Sources
set(COMPILER_SOURCES
    compiler/diagnostics/diagnostics.hpp
    compiler/lexer/lexer.hpp
    compiler/lexer/lexer.cpp
    compiler/ast/ast.hpp
    compiler/parser/parser.hpp
    compiler/parser/parser.cpp
    compiler/semantic/semantic.hpp
    compiler/semantic/semantic.cpp
    compiler/evaluator/evaluator.hpp
    compiler/evaluator/evaluator.cpp
    compiler/codegen/codegen_c.hpp
    compiler/codegen/codegen_c.cpp
)

add_library(paslang_core ${COMPILER_SOURCES})

# CLI Executable
add_executable(paslang compiler/driver/main.cpp)
target_link_libraries(paslang PRIVATE paslang_core)

# Test Suite Executable
set(TEST_SOURCES
    tests/test_main.cpp
    tests/test_lexer.cpp
    tests/test_parser.cpp
    tests/test_evaluator.cpp
    tests/test_codegen.cpp
)

add_executable(paslang_tests ${TEST_SOURCES})
target_link_libraries(paslang_tests PRIVATE paslang_core)

message(STATUS "PasLang Compiler v0.1 CMake configuration complete.")

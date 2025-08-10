cmake_minimum_required(VERSION 3.15)
project(mips-project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directory
include_directories(include)

# Collect core sources
set(CORE_SOURCES
    src/core/machine_state.cpp
    src/core/instruction.cpp
)

# Collect parser sources
set(PARSER_SOURCES
    src/parser/parser.cpp
)

# Collect assembler sources
set(ASSEMBLER_SOURCES
    src/assembler/assembler.cpp
)

# Collect interpreter sources
set(INTERPRETER_SOURCES
    src/interpreter/interpreter.cpp
)

# Collect executor sources
set(EXECUTOR_SOURCES
    src/executor/executor.cpp
)

# Main executables
add_executable(mips_assembler
    src/main/main_assembler.cpp
    ${CORE_SOURCES}
    ${PARSER_SOURCES}
    ${ASSEMBLER_SOURCES}
)

add_executable(mips_interpreter
    src/main/main_interpreter.cpp
    ${CORE_SOURCES}
    ${PARSER_SOURCES}
    ${INTERPRETER_SOURCES}
)

add_executable(mips_executor
    src/main/main_executor.cpp
    ${CORE_SOURCES}
    ${EXECUTOR_SOURCES}
)

# Tests
enable_testing()

# Helper macro to add tests
macro(add_test_executable name sources)
    add_executable(${name} ${sources} ${CORE_SOURCES})
    target_include_directories(${name} PRIVATE include)
    add_test(NAME ${name} COMMAND ${name})
endmacro()

add_test_executable(test_machine_state tests/test_machine_state.cpp)
add_test_executable(test_parser tests/test_parser.cpp)
add_test_executable(test_instruction tests/test_instruction.cpp)

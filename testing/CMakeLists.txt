cmake_minimum_required(VERSION 3.20)
project(memu_tester LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        10.1.1
)
FetchContent_MakeAvailable(fmt)

# We include the core emulator and assembler files directly from the main project
set(CORE_DIR ../src/core)
set(ASM_DIR ../src/assembler)

set(SOURCES
    src/main.cpp
    src/test_runner.cpp
    ${CORE_DIR}/cpu.cpp
    ${CORE_DIR}/cpu_exec.cpp
    ${CORE_DIR}/emulator.cpp
    ${CORE_DIR}/interrupts.cpp
    ${CORE_DIR}/disassembler.cpp
    ${CORE_DIR}/debugger.cpp
    ${ASM_DIR}/assembler.cpp
)

add_executable(memu_tester ${SOURCES})

target_include_directories(memu_tester PRIVATE 
    src
    ../src
)

target_link_libraries(memu_tester PRIVATE fmt::fmt)

if(MSVC)
    target_compile_options(memu_tester PRIVATE /W4 /Zc:__cplusplus /permissive-)
else()
    target_compile_options(memu_tester PRIVATE -Wall -Wextra)
endif()

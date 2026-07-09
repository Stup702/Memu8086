cmake_minimum_required(VERSION 3.16)
project(memu8086_wasm)

set(CMAKE_CXX_STANDARD 17)

# Include paths for the core emulator files
include_directories(../../src)

# Define sources (everything except the Qt UI and main)
set(SOURCES
    ../../src/core/cpu.cpp
    ../../src/core/cpu_exec.cpp
    ../../src/core/emulator.cpp
    ../../src/core/debugger.cpp
    ../../src/core/interrupts.cpp
    ../../src/core/disassembler.cpp
    ../../src/assembler/assembler.cpp
    ../../src/wasm_api.cpp
)

add_executable(memu8086_core ${SOURCES})

# Add fmt library (download it via FetchContent since it's used in the core)
include(FetchContent)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.1.0
)
FetchContent_MakeAvailable(fmt)

target_link_libraries(memu8086_core PRIVATE fmt::fmt)

# Emscripten specific flags for Embind
set_target_properties(memu8086_core PROPERTIES
    LINK_FLAGS "-lembind -s WASM=1 -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap'] -s MODULARIZE=1 -s EXPORT_NAME='createMemu8086'"
)

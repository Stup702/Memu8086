cmake_minimum_required(VERSION 3.20)
project(memu8086 VERSION 0.1.0 LANGUAGES CXX)

# Generate compile_commands.json for VS Code / clangd IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enforce C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)
qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

qt_add_executable(memu8086 
    src/main.cpp
    src/ui/mainwindow.h
    src/ui/mainwindow.cpp
    src/ui/theme.h
    src/ui/theme.cpp
    src/ui/editor_panel.h
    src/ui/editor_panel.cpp
    src/ui/highlighter.h
    src/ui/highlighter.cpp
    src/ui/registers_panel.h
    src/ui/registers_panel.cpp
    src/ui/memory_panel.h
    src/ui/memory_panel.cpp
    src/ui/stack_panel.h
    src/ui/stack_panel.cpp
    src/ui/variables_panel.h
    src/ui/variables_panel.cpp
    src/ui/console_panel_widget.h
    src/ui/console_panel_widget.cpp
    src/ui/toolbar.h
    src/ui/toolbar.cpp
    src/ui/settings_dialog.h
    src/ui/settings_dialog.cpp
    src/ui/examples_dialog.h
    src/ui/examples_dialog.cpp
    src/resources.qrc
    src/core/cpu.cpp
    src/core/cpu_exec.cpp
    src/core/emulator.cpp
    src/core/interrupts.cpp
    src/core/disassembler.cpp
    src/core/debugger.h
    src/core/debugger.cpp
    src/assembler/assembler.cpp
)

# Link libraries
target_link_libraries(memu8086 PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    fmt::fmt
    nlohmann_json::nlohmann_json
)

# Enable strict warnings based on the compiler
if(MSVC)
    target_compile_options(memu8086 PRIVATE /W4)
else()
    target_compile_options(memu8086 PRIVATE -Wall -Wextra)
endif()

# Define compile-time version string
target_compile_definitions(memu8086 PRIVATE MEMU8086_VERSION="${PROJECT_VERSION}")

# Include paths
target_include_directories(memu8086 PRIVATE src)

option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    add_executable(test_runner
        src/ui/test_main.cpp
        src/core/cpu.cpp src/core/cpu_exec.cpp src/core/emulator.cpp src/core/debugger.cpp
        src/core/interrupts.cpp src/core/disassembler.cpp src/assembler/assembler.cpp
    )
    target_link_libraries(test_runner PRIVATE fmt::fmt nlohmann_json::nlohmann_json)
    target_include_directories(test_runner PRIVATE src)
endif()
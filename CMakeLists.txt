# memu8086
# Copyright (C) 2026 Animesh Barua Mugdha
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.20)
project(memu8086 VERSION 1.0.3 LANGUAGES CXX)

# Generate compile_commands.json for VS Code / clangd IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enforce C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Global MSVC options
if(MSVC)
    add_compile_options(/FS)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)
qt_standard_project_setup()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(FetchContent)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        10.1.1
)
FetchContent_MakeAvailable(fmt)

set(SOURCES
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

if(WIN32)
    list(APPEND SOURCES appicon.rc)
endif()

qt_add_executable(memu8086 WIN32 MACOSX_BUNDLE
    ${SOURCES}
)

# Link libraries
target_link_libraries(memu8086 PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    fmt::fmt
)

# Enable strict warnings and MSVC compliance based on the compiler
if(MSVC)
    target_compile_options(memu8086 PRIVATE /W4 /Zc:__cplusplus /permissive-)
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
    target_link_libraries(test_runner PRIVATE fmt::fmt)
    target_include_directories(test_runner PRIVATE src)

    # Enable strict warnings and MSVC compliance based on the compiler
    if(MSVC)
        target_compile_options(test_runner PRIVATE /W4 /Zc:__cplusplus /permissive- /FS)
    else()
        target_compile_options(test_runner PRIVATE -Wall -Wextra)
    endif()
endif()

include(GNUInstallDirs)
install(TARGETS memu8086
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(COMMAND qt_generate_deploy_app_script)
    qt_generate_deploy_app_script(
        TARGET memu8086
        FILENAME_VARIABLE deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT "${deploy_script}")
endif()

# --- CPack Packaging Configuration ---
set(CPACK_PACKAGE_NAME "memu8086")
set(CPACK_PACKAGE_VENDOR "Animesh Barua Mugdha")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A modern Intel 8086 emulator and assembler.")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")

if(WIN32)
    set(CPACK_GENERATOR "ZIP") # Creates a portable Windows ZIP
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop") # Creates a macOS .dmg installer
else()
    set(CPACK_GENERATOR "DEB;TGZ") # Creates a Debian .deb and a portable Linux .tar.gz
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Animesh Barua Mugdha")
endif()

include(CPack)

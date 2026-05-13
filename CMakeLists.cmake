cmake_minimum_required(VERSION 3.20)
project(memu8086 VERSION 0.1.0 LANGUAGES CXX)

# Enforce C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find vcpkg dependencies
find_package(SDL2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Define the main executable
add_executable(memu8086 src/main.cpp)

# Link libraries (Using generator expressions for safety across OS differences)
target_link_libraries(memu8086 PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    SDL2::SDL2
    imgui::imgui
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

# Copy assets folder to the output directory post-build
add_custom_command(TARGET memu8086 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:memu8086>/assets
    COMMENT "Copying assets/ to build directory..."
)

# Hook up tests if the directory exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt")
    add_subdirectory(tests)
endif()

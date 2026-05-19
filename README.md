# memu8086

A modern, cross-platform Intel 8086 emulator and assembler with a rich graphical user interface. Built with C++17 and Qt 6.

## Features

- **Integrated Assembler:** Write your 8086 assembly code and assemble it instantly within the application.
- **Advanced Debugger:** 
  - Step, Step Over, Run, Pause, and Stop execution.
  - Set visual breakpoints directly in the code editor.
  - Adjustable execution speed to visualize code flow in real-time.
- **Interactive Console:** Built-in terminal emulator to view text output and send keyboard input directly to the emulated CPU via DOS/BIOS interrupts.
- **Comprehensive Hardware Views:**
  - **Registers:** Real-time view of all CPU registers and flags.
  - **Memory:** Hex viewer with quick-jump functionality.
  - **Stack:** Visual representation of the call stack.
  - **Variables:** Track and inspect labeled memory locations and resolved symbols.
- **Customizable Workspace:** 
  - Fully dockable interface (drag, drop, tear-off, and resize panels to your liking).
  - Support for custom layouts with save/restore functionality.
  - Built-in Dark and Light themes with an embedded programmer's font (JetBrains Mono).

## Prerequisites

- **C++17** compatible compiler (GCC, Clang, or MSVC)
- **CMake** (>= 3.20)
- **Qt 6** (Core, Gui, Widgets components)

*(Note: The `fmt` library is required but will be automatically downloaded and configured by CMake).*

## Building from Source

### 1. Clone the repository

```bash
git clone https://github.com/yourusername/memu8086.git
cd memu8086
```

### 2. Configure the project

Create a build directory and configure the project with CMake. If Qt 6 is not installed in a standard system path, provide the path to your Qt installation using `CMAKE_PREFIX_PATH`:

```bash
# Example for Linux with a local Qt installation:
cmake -B build -DCMAKE_PREFIX_PATH="$HOME/Qt/6.x.x/gcc_64"

# Example for Windows (MSVC):
cmake -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64"
```

### 3. Build the executable

```bash
cmake --build build -j$(nproc)
```

### 4. Run the emulator

```bash
# On Linux/macOS:
./build/memu8086

# On Windows:
.\build\Debug\memu8086.exe
```

## License

This project is licensed under the GNU General Public License (GPL) - see the LICENSE file for details.
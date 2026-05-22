# 🍎 macOS Build Guide for memu8086

Welcome to the macOS build guide for **memu8086**, a modern Intel 8086 emulator and assembler. This guide provides step-by-step instructions to compile and run the project natively on macOS (fully compatible with both Apple Silicon M-Series and Intel MacBooks).

## 🛠 Prerequisites

Before building the project, ensure you have the necessary development tools installed. The easiest way to install these on macOS is via [Homebrew](https://brew.sh/).

Open your Terminal and install the required dependencies (CMake, Ninja, and Qt6):

```bash
brew install cmake ninja qt6
(Note: If you haven't installed Apple's Command Line Tools yet, you might be prompted to do so. You can manually install them via xcode-select --install).

🚀 Build Instructions
Follow these steps to configure and compile the emulator from the source code.

1. Clone the repository and navigate into it:

Bash
git clone [https://github.com/Stup702/Memu8086](https://github.com/Stup702/Memu8086)
cd memu8086
2. Create a dedicated build directory:
We use a separate build directory to keep the source code clean.

Bash
mkdir build
cd build
3. Configure the project with CMake:
This step tells CMake where to find the Qt6 framework installed by Homebrew.

Bash
cmake .. -G Ninja -DCMAKE_PREFIX_PATH=$(brew --prefix qt6)
4. Compile the application:

Bash
ninja
Note: The first time you run this, it may take a minute or two as it automatically fetches required libraries (like fmt) and compiles the core architecture.

🎮 Running the Emulator
Once the build process completes successfully, a native macOS application bundle (memu8086.app) will be generated inside your build directory.

Launch the emulator directly from your terminal using:

Bash
open memu8086.app
💡 Troubleshooting
CMake cannot find Qt6: Ensure Homebrew is correctly added to your system's $PATH. You can verify the Qt6 path by running echo $(brew --prefix qt6) in your terminal.

Build Artifacts: To clean up your project or start a fresh build, simply delete the build folder (rm -rf build) and repeat the steps above.
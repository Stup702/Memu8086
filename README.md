# Build Guide for memu8086

Welcome to the build guide for **memu8086**, a modern Intel 8086 emulator and assembler. This project is cross-platform and can be built on Linux, macOS, and Windows.

---

# 🐧 Linux Build Guide (Debian/Ubuntu)

This guide provides step-by-step instructions to compile and run the project on Linux distributions like Ubuntu, Debian, or Linux Mint.

## 🛠 Prerequisites

Install the necessary development tools and Qt6 libraries:

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build qt6-base-dev libgl1-mesa-dev
```

## 🚀 Build Instructions

1. **Clone the repository:**
   ```bash
   git clone https://github.com/Stup702/Memu8086
   cd memu8086
   ```

2. **Create a build directory:**
   ```bash
   mkdir build
   cd build
   ```

3. **Configure and compile:**
   ```bash
   cmake .. -G Ninja
   ninja
   ```

## 🎮 Running the Emulator

Launch the emulator directly from the build directory:

```bash
./memu8086
```

---

# 🍎 macOS Build Guide

This guide provides instructions to compile and run the project natively on macOS (Apple Silicon or Intel).

## 🛠 Prerequisites

The easiest way to install dependencies is via [Homebrew](https://brew.sh/):

```bash
brew install cmake ninja qt6
```
*(Note: If prompted, install Apple's Command Line Tools via `xcode-select --install`)*

## 🚀 Build Instructions

1. **Clone the repository:**
   ```bash
   git clone https://github.com/Stup702/Memu8086
   cd memu8086
   ```

2. **Create a build directory:**
   ```bash
   mkdir build
   cd build
   ```

3. **Configure the project:**
   This step tells CMake where to find the Qt6 framework installed by Homebrew.
   ```bash
   cmake .. -G Ninja -DCMAKE_PREFIX_PATH=$(brew --prefix qt6)
   ```

4. **Compile the application:**
   ```bash
   ninja
   ```

## 🎮 Running the Emulator

A native macOS application bundle (`memu8086.app`) will be generated. Launch it using:

```bash
open memu8086.app
```

---

## 💡 Troubleshooting

- **CMake cannot find Qt6:** On macOS, ensure Homebrew's path is correctly set. You can verify it with `echo $(brew --prefix qt6)`.
- **Clean Build:** To start fresh, delete the build folder (`rm -rf build`) and repeat the steps.



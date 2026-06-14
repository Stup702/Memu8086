# Build Guide for memu8086

Welcome to the build guide for **memu8086**, a modern Intel 8086 emulator and assembler. This project is cross-platform and can be built on Linux, macOS, and Windows.

---

# Linux Build Guide (Debian/Ubuntu)

This guide provides step-by-step instructions to compile and run the project on Linux distributions like Ubuntu, Debian, or Linux Mint.

## Prerequisites

Install the necessary development tools and Qt6 libraries:

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build qt6-base-dev libgl1-mesa-dev
```

## Build Instructions

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

## Running the Emulator

Launch the emulator directly from the build directory:

```bash
./memu8086
```

---

# macOS Build Guide

This guide provides instructions to compile and run the project natively on macOS (Apple Silicon or Intel).

## Prerequisites

The easiest way to install dependencies is via [Homebrew](https://brew.sh/):

```bash
brew install cmake ninja qt6
```
*(Note: If prompted, install Apple's Command Line Tools via `xcode-select --install`)*

## Build Instructions

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

## Running the Emulator

A native macOS application bundle (`memu8086.app`) will be generated. Launch it using:

```bash
open memu8086.app
```

---

## Troubleshooting

- **CMake cannot find Qt6:** On macOS, ensure Homebrew's path is correctly set. You can verify it with `echo $(brew --prefix qt6)`.
- **Clean Build:** To start fresh, delete the build folder (`rm -rf build`) and repeat the steps.

# Running the app

## Running on linux
Nothing required. Download and run

## Running On macOS

If you are new to macOS or just want to get the emulator up and running quickly, follow these simple steps:

Step 1: Download the App
Scroll down to the "Assets" section at the bottom of this page and click on memu8086.dmg to download it.

Step 2: Open the Installer
Once the download is finished, double-click the memu8086.dmg file. A small window will pop up showing the Memu8086 app icon.

Step 3: Install It
To install the app, simply click and drag the Memu8086 app icon into your Applications folder. You can now close the little installer window and eject the .dmg file from your desktop.

Step 4: The First Launch (Important!)
Note: Because this app is built by independent developers and not downloaded from the official Mac App Store, macOS will try to block it the very first time you open it.

To bypass this safely:

Open your Applications folder and find Memu8086.

Do not double-click it. Instead, Right-click (or hold Control on your keyboard and click) the app icon, then select Open from the drop-down menu.

A warning pop-up will appear saying the developer cannot be verified. Click the Open button in that pop-up.
(If the "Open" button is missing, click "Cancel", then open your Mac's System Settings > Privacy & Security. Scroll down to the security section and click Open Anyway next to the Memu8086 message).

Step 5: You're Done! 🎉
The emulator will now launch! You only have to do the right-click trick once. From now on, you can just double-click the app normally from your Launchpad or Applications folder whenever you want to write some assembly code.

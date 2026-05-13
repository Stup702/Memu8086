# memu8086

A modern, cross-platform 8086 emulator & assembler IDE built with C++17, Dear ImGui, and SDL2.
Inspired by the classic `emu8086`.

## Features
- Full 8086 instruction set emulation.
- Built-in Assembler (Intel syntax) with real-time error tracking.
- Hardware-accelerated Dear ImGui debugger interface (Registers, Flags, Stack, Memory).
- Support for basic DOS `INT 21h` and BIOS `INT 10h` software interrupts.

## Building
**Dependencies (Linux)**:
`sudo apt install libsdl2-dev libfmt-dev nlohmann-json3-dev cmake`

**Compile**:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
./build/memu8086
```

## Quick Start
Paste the following into the code editor, press **F5** to assemble, and **F9** to run:
```asm
; Hello World
org 100h
mov ah, 09h
mov dx, msg
int 21h
mov ah, 4ch
int 21h
msg db 'Hello, World!$'
```

## License
MIT
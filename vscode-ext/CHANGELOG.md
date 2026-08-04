# Changelog

## [0.8.0]
* **Assembler Forward Reference Fix:** Resolved an issue where `CALL` instruction targets for forward-referenced procedures were calculated incorrectly in `.MODEL SMALL`.
* **Assembler Label Fix:** Fixed a bug where `ENDP` directives would incorrectly overwrite the starting address of their corresponding procedure label.
* **Emulator Run-To/Step-Over Bug:** Implemented a stack-pointer depth guard to prevent "Step Over" or "Continue" from pausing prematurely inside subroutines.

## [0.7.99]
* **VS Code UI — Assembly Errors:** Surfaced assembly compilation errors so they trigger a visible VS Code warning popup, rather than failing silently.
* **VS Code UI — High/Low Register Decimals:** Enhanced the General Registers tooltips to independently display the Unsigned, Signed, and Binary values of their High (AH) and Low (AL) 8-bit components.
* **Memory Model Fix:** Fixed a critical bug where the assembler's calculated `@DATA` segment value could desynchronize from the emulator's runtime data placement.

## [0.7.98]
* **Interrupt System:** Fixed `INT 16h` (Keyboard) to correctly suspend and resume in a non-blocking I/O environment.
* **Buffered Input:** Fixed `INT 21h AH=0Ah` (Buffered Input) so partial user input is persisted during non-blocking yields.
* **Assembler Expressions:** Fixed the `evaluate_expression` parser to correctly handle operator precedence.

## [0.7.97]
* **Assembler Type Checking:** Added strict operand size type checking to the assembler.
* **Assembler Expressions:** Added support for multiplication (`*`) and division (`/`) in assembly expressions.
* **New CPU Opcodes:** Implemented support for the 80186 `PUSHA` (0x60) and `POPA` (0x61) instructions.
* **CPU Flag Fixes:** Fixed the `OF` (Overflow) flag logic for the `ROL`, `ROR`, `RCL`, and `RCR` instructions.
* **Interrupt Enhancements:** Implemented missing DOS/BIOS interrupts (INT 21h 25h/30h/35h, INT 10h 08h/0Fh) and fixed scrolling logic.

## [0.7.96]
* **VS Code Extension:** Added a high-quality app icon to the VS Code extension for better marketplace visibility and UX.

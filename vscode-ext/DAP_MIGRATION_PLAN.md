# MEMU8086 Debug Adapter Protocol (DAP) Migration Plan

This document outlines the step-by-step architectural shift from our current Webview-based extension to a full-fledged VS Code Debug Adapter Protocol (DAP) implementation. This migration will unlock VS Code's native breakpoints, variables view, call stack, and memory viewer.

## Phase 1: Dependency & Contribution Setup
1. **Update `package.json`**: 
   - Add `@vscode/debugadapter` to dependencies.
   - Add a `debuggers` section to the `contributes` block to register `memu8086` as a valid debug type.
   - Define custom debug events (if necessary) for routing DOS terminal I/O.
2. **Install Dependencies**: Run `npm install @vscode/debugadapter` in the `vscode-ext` directory.

## Phase 2: WASM Node.js Integration
1. **Shift Execution Context**: Currently, `memu8086_core.wasm` runs inside a browser Webview. The Debug Adapter runs in a Node.js environment (the Extension Host). We will load and instantiate the WASM module directly in `extension.ts` or `debugAdapter.ts` using Node.js filesystem and WebAssembly APIs.
2. **Embind Porting**: Ensure that our existing Embind methods (`assemble`, `step`, `get_output`, `send_input`, `get_registers`) are exposed correctly to the Node.js context.

## Phase 3: The `MemuDebugSession` Class
Create a new class extending `DebugSession` (or `LoggingDebugSession`) from `@vscode/debugadapter`. We will implement the core DAP lifecycle requests:
- **`initializeRequest`**: Tell VS Code we support stepping, breakpoints, and variables.
- **`launchRequest`**: Retrieve the `.asm` source, call `emu.reset()` and `emu.assemble()`. If assembly fails, send an error event and terminate.
- **`setBreakPointsRequest`**: Map VS Code source lines to instruction addresses (LCs). We will need to add a `get_address_for_line(int line)` and `get_line_for_address(int address)` method to our C++ API to translate breakpoints!
- **`threadsRequest`**: Return a dummy "8086 CPU" thread.
- **`stackTraceRequest`**: Return a single frame for the current IP (Instruction Pointer).
- **`scopesRequest`**: Return scopes like "Registers", "Flags", and "Memory".
- **`variablesRequest`**: Return the live values of AX, BX, CX, DX, etc., and the status of Flags.
- **`nextRequest` / `stepInRequest`**: Call `emu.step()` and then trigger a `StoppedEvent`.
- **`continueRequest`**: Run a loop of `emu.step()` until halted or a breakpoint is hit.

## Phase 4: DOS Pseudoterminal Routing
1. **Custom DAP Events**: Since DAP doesn't natively handle DOS-style interactive terminals (it only has a standard debug console for logs), we will retain our `vscode.Pseudoterminal`.
2. **IPC**: When the `MemuDebugSession` (Node.js) gets output from the WASM emulator, it will emit a custom DAP event (`memu8086.terminalOutput`). The Extension Host will listen for this event and pipe the text into the `Pseudoterminal`.

## Phase 5: C++ WASM API Enhancements (Prerequisites)
To fully support the DAP, we need to add a few new methods to `src/wasm_api.cpp`:
1. `bool has_breakpoint_at(uint16_t addr)` / `void toggle_breakpoint(uint16_t addr, bool enabled)`
2. Line-to-Address mapping so we can convert VS Code's line numbers into 8086 memory addresses.
3. Access to arbitrary memory chunks for VS Code's Memory Hex Editor.

## Phase 6: Seamless UX (No `launch.json`)
1. Re-wire the `memu8086.chippingIn` command.
2. Instead of opening a Webview, the command will invoke: 
   \`vscode.debug.startDebugging(undefined, { type: 'memu8086', name: 'Debug DOS', request: 'launch', program: currentFile })\`
3. The old Dashboard HTML can be deleted.

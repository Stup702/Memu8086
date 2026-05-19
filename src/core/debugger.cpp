// memu8086
// Copyright (C) 2026 Animesh Barua Mugdha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "debugger.h"
#include <cstring>
#include <algorithm>

namespace emu8086::core {

// --- ConsoleState Implementation ---

void ConsoleState::put_char(char c, uint8_t attr) {
    if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\t') {
        cursor_x = (cursor_x / 8 + 1) * 8;
        if (cursor_x >= cols) {
            cursor_x = 0;
            cursor_y++;
        }
    } else if (c == '\b' || c == 0x08) {
        if (cursor_x > 0) {
            cursor_x--;
            screen[cursor_y][cursor_x] = 0x20;
            color[cursor_y][cursor_x] = attr;
        }
    } else if (c == '\f' || c == 0x0C) {
        clear_screen(attr);
    } else {
        if (cursor_x >= cols) {
            cursor_x = 0;
            cursor_y++;
        }
        if (cursor_y >= rows) {
            scroll_up(1);
            cursor_y = rows - 1;
        }
        screen[cursor_y][cursor_x] = c;
        color[cursor_y][cursor_x] = attr;
        cursor_x++;
    }

    if (cursor_y >= rows) {
        scroll_up(1);
        cursor_y = rows - 1;
    }
}

void ConsoleState::scroll_up(int lines) {
    if (lines <= 0) return;
    if (on_scroll_up) on_scroll_up();
    if (lines >= rows) {
        clear_screen(0x07);
        return;
    }
    int move_rows = rows - lines;
    std::memmove(screen[0], screen[lines], move_rows * MAX_COLS);
    std::memmove(color[0], color[lines], move_rows * MAX_COLS);
    std::memset(screen[move_rows], 0x20, lines * MAX_COLS);
    std::memset(color[move_rows], 0x07, lines * MAX_COLS);
}

void ConsoleState::clear_screen(uint8_t attr) {
        std::memset(screen, 0x20, sizeof(screen));
    std::memset(color, attr, sizeof(color));
    cursor_x = 0;
    cursor_y = 0;
}

void ConsoleState::set_cursor(int row, int col) {
    cursor_y = std::clamp(row, 0, rows - 1);
    cursor_x = std::clamp(col, 0, cols - 1);
}

void ConsoleState::resize(int new_cols, int new_rows) {
    cols = std::clamp(new_cols, 10, MAX_COLS);
    rows = std::clamp(new_rows, 10, MAX_ROWS);
    if (cursor_x >= cols) cursor_x = cols - 1;
    if (cursor_y >= rows) cursor_y = rows - 1;
}

// --- Debugger Implementation ---

Debugger::Debugger(CPU& cpu, Memory& mem, ConsoleState& console)
    : cpu(cpu), mem(mem), console(console) {
    
    emulator.set_speed_hz(static_cast<uint32_t>(speed_ips_));
    sync_state();
}

void Debugger::sync_state() {
    auto snap = emulator.snapshot();
    cpu.regs = snap.regs;
    cpu.regs.flags = snap.flags;
    last_error_ = snap.last_error;
    
    // Sync memory from internal emulator to external UI memory representation
    const auto& emu_mem = emulator.memory_view();
    std::memcpy(mem.raw_ptr(0), emu_mem.data(), MEMORY_SIZE);
}

void Debugger::save_prev_snapshot() {
    prev_snapshot_.regs = cpu.regs;
    prev_snapshot_.flags = cpu.regs.flags;
    prev_snapshot_.cycle_count = emulator.snapshot().cycle_count;
}

void Debugger::step() {
    save_prev_snapshot();
    emulator.step();
    sync_state();
}

void Debugger::step_over() {
    save_prev_snapshot();
    emulator.step_over();
    sync_state();
}

void Debugger::run() {
    save_prev_snapshot();
    emulator.start();
}

void Debugger::stop() {
    emulator.stop();
    emulator.send_key('\0'); // Poison-pill: wake up any DOS interrupts stuck in wait_and_read_key()!
    sync_state();
}

void Debugger::reset() {
    emulator.stop();
    emulator.send_key('\0'); // Poison-pill: wake up any DOS interrupts stuck in wait_and_read_key()!
    console.clear_screen();
    console.input_buffer.clear();
    console.waiting_for_input = false;
    
    last_error_.clear();
    
    // Optional: Could clear/reset memory or reload last assembly result here
    sync_state();
}

void Debugger::run_frame(float dt) {
    // ALWAYS drain any pending IO output from the emulator, even if halted
    auto outputs = emulator.io_output();
    for (const std::string& str : outputs) {
        for (char c : str) console.put_char(c, 0x07);
    }

    // When background thread is active (RUNNING state from emulator.start()),
    // do NOT also call step() — just drain IO output and sync state.
    if (emulator.state() == EmulatorState::RUNNING) {
        // Check if background thread hit a breakpoint or halted
        sync_state();
        return;
    }

    // Controlled-speed stepping mode (used when we want frame-rate-limited stepping
    // without a background thread — currently not exposed to UI but available for future use)
    if (emulator.state() != EmulatorState::PAUSED) return;

    instr_accumulator_ += speed_ips_ * dt;
    int iterations = 0;

    while (instr_accumulator_ >= 1.0f) {
        save_prev_snapshot();
        emulator.step();
        instr_accumulator_ -= 1.0f;
        ++iterations;

        auto s = emulator.state();
        if (s == EmulatorState::HALTED || s == EmulatorState::ERROR) break;

        auto snap = emulator.snapshot();
        if (breakpoints_.find(snap.regs.IP) != breakpoints_.end()) {
            emulator.pause();
            break;
        }
        if (iterations >= 50000) break;
    }

    outputs = emulator.io_output();
    for (const std::string& str : outputs) {
        for (char c : str) console.put_char(c, 0x07);
    }
    sync_state();
}

DebuggerState Debugger::get_state() const {
    switch (emulator.state()) {
        case EmulatorState::IDLE:    return DebuggerState::IDLE;
        case EmulatorState::RUNNING: return DebuggerState::RUNNING;
        case EmulatorState::PAUSED:  return DebuggerState::PAUSED;
        case EmulatorState::HALTED:  return DebuggerState::HALTED;
        case EmulatorState::ERROR:   return DebuggerState::ERROR;
    }
    return DebuggerState::IDLE;
}

std::string Debugger::get_last_error() const {
    return last_error_;
}

std::string Debugger::get_last_mnemonic() const {
    return ""; // UI can resolve via disassembler pass directly from memory, or fallback for now
}

uint64_t Debugger::get_cycle_count() const {
    return emulator.snapshot().cycle_count;
}

CPUSnapshot Debugger::get_prev_snapshot() const {
    return prev_snapshot_;
}

void Debugger::set_speed(float instructions_per_second) {
    speed_ips_ = instructions_per_second;
    emulator.set_speed_hz(static_cast<uint32_t>(instructions_per_second));
}

float Debugger::get_speed() const {
    return speed_ips_;
}

void Debugger::add_breakpoint(uint16_t ip) { emulator.add_breakpoint(ip); breakpoints_.insert(ip); }
void Debugger::remove_breakpoint(uint16_t ip) { emulator.remove_breakpoint(ip); breakpoints_.erase(ip); }
bool Debugger::has_breakpoint(uint16_t ip) const { return breakpoints_.find(ip) != breakpoints_.end(); }
const std::set<uint16_t>& Debugger::get_breakpoints() const { return breakpoints_; }

void Debugger::load_program(const emu8086::assembler::AssemblyResult& result) {
        emulator.stop();
        emulator.load_com(result.machine_code, 0x0100);
        prev_snapshot_ = CPUSnapshot{};
        sync_state();
}

void Debugger::send_key(char c) {
    emulator.send_key(c);
}

} // namespace emu8086::core
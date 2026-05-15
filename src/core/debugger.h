#pragma once

#include <cstdint>
#include <string>
#include <set>
#include <functional>

#include "cpu.h"
#include "emulator.h"
#include "../assembler/assembler.h"

namespace emu8086::core {

enum class DebuggerState { IDLE, RUNNING, PAUSED, HALTED, ERROR };

struct CPUSnapshot {
    Registers regs;
    Flags     flags;
    uint64_t  cycle_count;
};

struct ConsoleState {
    static constexpr int MAX_COLS = 256;
    static constexpr int MAX_ROWS = 256;
    int cols = 80;
    int rows = 25;
    uint8_t screen[MAX_ROWS][MAX_COLS]{};
    uint8_t color[MAX_ROWS][MAX_COLS]{};
    int cursor_x = 0, cursor_y = 0;
    bool waiting_for_input = false;
    std::string input_buffer;
    std::function<void(char)> on_key;
    std::function<void()> on_scroll_up;

    void put_char(char c, uint8_t attr);
    void scroll_up(int lines = 1);
    void clear_screen(uint8_t attr = 0x07);
    void set_cursor(int row, int col);
    void resize(int new_cols, int new_rows);
};

class Debugger {
public:
    Debugger(CPU& cpu, Memory& mem, ConsoleState& console);
    
    // Execution control
    void step();
    void step_over();
    void run();
    void stop();
    void reset();
    void run_frame(float dt);  // dt in seconds; executes speed*dt instructions

    // State query
    DebuggerState get_state() const;
    std::string   get_last_error() const;
    std::string   get_last_mnemonic() const;
    uint64_t      get_cycle_count() const;
    CPUSnapshot   get_prev_snapshot() const;

    // Speed control
    void     set_speed(float instructions_per_second);
    float    get_speed() const;

    // Breakpoints
    void add_breakpoint(uint16_t ip);
    void remove_breakpoint(uint16_t ip);
    bool has_breakpoint(uint16_t ip) const;
    const std::set<uint16_t>& get_breakpoints() const;

    // Program loading
    void load_program(const emu8086::assembler::AssemblyResult& result);

    // Console input
    void send_key(char c);

private:
    void sync_state();
    void save_prev_snapshot();

    Emulator emulator;          // owns the actual emulator
    CPU& cpu;
    Memory& mem;
    ConsoleState& console;
    CPUSnapshot prev_snapshot_;
    float speed_ips_ = 1000.0f;
    float instr_accumulator_ = 0.0f;
    std::set<uint16_t> breakpoints_;
    std::string last_error_;
};

} // namespace emu8086::core
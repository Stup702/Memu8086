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

#pragma once

#include "cpu.h"
#include "cpu_exec.h"
#include "interrupts.h"
#include "../assembler/assembler.h"

#include <atomic>
#include <thread>
#include <mutex>
#include <unordered_set>
#include <string>
#include <vector>
#include <memory>
#include <chrono>

namespace emu8086::core {

enum class EmulatorState : uint8_t {
    IDLE, RUNNING, PAUSED, HALTED, ERROR
};

struct EmulatorSnapshot {
    Registers  regs;
    Flags      flags;
    uint32_t   cycle_count{0};
    std::string last_error;
};

class Emulator {
public:
    Emulator();
    ~Emulator();

    // === LOADING ===
    bool load_com(const std::vector<uint8_t>& machine_code, uint16_t load_offset = 0x0100);
    bool load_from_assembly(const std::string& source);
    const assembler::AssemblyResult& last_assembly() const;

    // === EXECUTION CONTROL ===
    void start();
    void pause();
    void stop();
    void step();
    void step_over();
    void run_to(uint16_t ip_target);

    // === BREAKPOINTS ===
    void add_breakpoint(uint16_t ip);
    void remove_breakpoint(uint16_t ip);
    void clear_breakpoints();
    bool has_breakpoint(uint16_t ip) const;
    const std::unordered_set<uint16_t>& breakpoints() const;
    void toggle_breakpoint(uint16_t ip);
    void toggle_flag(int flag_idx);

    // === STATE QUERY ===
    EmulatorState    state() const;
    EmulatorSnapshot snapshot() const;
    const std::array<uint8_t, MEMORY_SIZE>& memory_view() const;
    void             write_memory(uint32_t addr, uint8_t val);
    std::vector<std::string> io_output();

    void     request_memory_view(uint32_t addr);
    uint32_t consume_memory_view_request();

    // === SPEED CONTROL ===
    void set_speed_hz(uint32_t instructions_per_second);
    uint32_t get_speed_hz() const;

    // === INPUT ===
    void send_key(char c);

private:
    CPU                                 cpu_;
    std::unique_ptr<IODevice>           io_device_;
    std::unique_ptr<InterruptHandler>   irq_;
    std::unique_ptr<Executor>           executor_;
    assembler::AssemblyResult           last_asm_;
    
    std::unordered_set<uint16_t>        breakpoints_;
    std::atomic<EmulatorState>          state_{EmulatorState::IDLE};
    std::atomic<bool>                   thread_active_{false};
    std::thread                         exec_thread_;
    mutable std::mutex                  cpu_mutex_;
    std::atomic<uint32_t>               speed_hz_{0};
    
    std::string                         output_buffer_;
    std::mutex                          output_mutex_;
    std::atomic<uint32_t>               memory_goto_request_{0xFFFFFFFF};
    uint16_t                            run_target_ip_{0xFFFF};

    void exec_loop_();
    void throttle_(std::chrono::steady_clock::time_point& last_tick);
    
    friend class StringIODevice;
};

} // namespace emu8086::core
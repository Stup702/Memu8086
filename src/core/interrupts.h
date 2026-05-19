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
#include <deque>
#include <mutex>
#include <string>
#include <vector>

namespace emu8086::core {

// --- Abstract I/O Interface ---
struct IODevice {
    virtual void write_char(char c) = 0;
    virtual int  read_char()        = 0;
    virtual void clear_screen()     = 0;
    virtual void set_cursor(int /*row*/, int /*col*/) {} // Optional for INT 10h AH=02h
    virtual ~IODevice() = default;
};

// --- Interrupt Handler ---
class InterruptHandler {
public:
    InterruptHandler(CPU& cpu, IODevice& io);

    bool handle(uint8_t interrupt_number);

    void enqueue_key(char c);
    bool has_input() const;

    std::vector<std::string> warnings;
    bool halted{false}; // Set by INT 20h and INT 21h AH=4Ch

private:
    CPU& cpu;
    IODevice& io;

    std::deque<char> input_queue;
    mutable std::mutex input_mutex;
};

} // namespace emu8086::core
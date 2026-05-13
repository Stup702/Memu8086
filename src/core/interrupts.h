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
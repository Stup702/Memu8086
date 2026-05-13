#include "interrupts.h"
#include <thread>
#include <chrono>
#include <cstdio>

namespace emu8086::core {

InterruptHandler::InterruptHandler(CPU& cpu, IODevice& io) : cpu(cpu), io(io) {}

void InterruptHandler::enqueue_key(char c) {
    std::lock_guard<std::mutex> lock(input_mutex);
    input_queue.push_back(c);
}

bool InterruptHandler::has_input() const {
    std::lock_guard<std::mutex> lock(input_mutex);
    return !input_queue.empty();
}

// --- Main Interrupt Dispatcher ---
bool InterruptHandler::handle(uint8_t interrupt_number) {
    // Helper lambda to fetch a keypress blockingly
    auto wait_and_read_key = [&]() -> char {
        while (!has_input()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
        std::lock_guard<std::mutex> lock(input_mutex);
        char c = input_queue.front();
        input_queue.pop_front();
        return c;
    };

    switch (interrupt_number) {
        case 0x10: { // BIOS Video Services
            uint8_t ah = cpu.regs.AH();
            switch (ah) {
                case 0x00: // Set video mode (stub)
                    io.clear_screen();
                    break;
                case 0x01: // Set cursor shape (stub)
                    break;
                case 0x02: // Set cursor position
                    io.set_cursor(cpu.regs.DH(), cpu.regs.DL());
                    break;
                case 0x03: // Get cursor position (stub)
                    cpu.regs.DH() = 0;
                    cpu.regs.DL() = 0;
                    cpu.regs.CX = 0;
                    break;
                case 0x06: // Scroll window up (stub)
                    io.clear_screen();
                    break;
                case 0x07: // Scroll window down (stub)
                    break;
                case 0x09: // Write char+attr at cursor
                case 0x0A: { // Write char at cursor
                    char c = static_cast<char>(cpu.regs.AL());
                    for (int i = 0; i < cpu.regs.CX; ++i) {
                        io.write_char(c);
                    }
                    break;
                }
                case 0x0E: // Teletype output
                    io.write_char(static_cast<char>(cpu.regs.AL()));
                    break;
                default: {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "Unimplemented INT 10h func: AH=%02X", ah);
                    warnings.push_back(std::string(buf));
                    break;
                }
            }
            return true;
        }

        case 0x16: { // BIOS Keyboard Services
            uint8_t ah = cpu.regs.AH();
            switch (ah) {
                case 0x00: // Wait for keypress
                    cpu.regs.AL() = wait_and_read_key();
                    break;
                case 0x01: // Check keystroke buffer
                    if (has_input()) {
                        cpu.regs.flags.ZF = false;
                        std::lock_guard<std::mutex> lock(input_mutex);
                        cpu.regs.AL() = input_queue.front(); // Peek without consuming
                    } else {
                        cpu.regs.flags.ZF = true;
                    }
                    break;
                default: {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "Unimplemented INT 16h func: AH=%02X", ah);
                    warnings.push_back(std::string(buf));
                    break;
                }
            }
            return true;
        }

        case 0x20: { // Program Terminate
            halted = true;
            return true;
        }

        case 0x21: { // DOS Services
            uint8_t ah = cpu.regs.AH();
            switch (ah) {
                case 0x01: { // Read character with echo
                    char c = wait_and_read_key();
                    cpu.regs.AL() = c;
                    io.write_char(c);
                    break;
                }
                case 0x02: // Write character in DL to stdout
                    io.write_char(static_cast<char>(cpu.regs.DL()));
                    break;
                case 0x06: { // Direct console I/O
                    if (cpu.regs.DL() != 0xFF) {
                        io.write_char(static_cast<char>(cpu.regs.DL()));
                    } else {
                        if (has_input()) {
                            cpu.regs.AL() = wait_and_read_key();
                            cpu.regs.flags.ZF = false; // standard is ZF=0 if char read
                        } else {
                            cpu.regs.AL() = 0x00;
                            cpu.regs.flags.ZF = true;
                        }
                    }
                    break;
                }
                case 0x07: // Read char no echo
                case 0x08: // Read char no echo
                    cpu.regs.AL() = wait_and_read_key();
                    break;
                case 0x09: { // Print string
                    uint32_t addr = cpu.ds_addr(cpu.regs.DX);
                    while (true) {
                        char c = static_cast<char>(cpu.mem.read8(addr++));
                        if (c == '$') break;
                        io.write_char(c);
                    }
                    break;
                }
                case 0x0A: { // Buffered input
                    uint32_t addr = cpu.ds_addr(cpu.regs.DX);
                    uint8_t max_chars = cpu.mem.read8(addr); // DOS standard: length max is at DX+0
                    uint8_t count = 0;
                    
                    while (count < max_chars) {
                        char c = wait_and_read_key();
                        if (c == '\r' || c == '\n') {
                            io.write_char('\r');
                            break;
                        } else if (c == '\b' || c == 0x7F) { // Handle backspace
                            if (count > 0) {
                                count--;
                                io.write_char('\b'); io.write_char(' '); io.write_char('\b');
                            }
                        } else {
                            cpu.mem.write8(addr + 2 + count, c);
                            count++;
                            io.write_char(c);
                        }
                    }
                    cpu.mem.write8(addr + 1, count); // Set actual read count at DX+1
                    break;
                }
                case 0x0B: // Check input status
                    cpu.regs.AL() = has_input() ? 0xFF : 0x00;
                    break;
                case 0x2A: // Get date (stub)
                    cpu.regs.CX = 2024; cpu.regs.DH() = 1; cpu.regs.DL() = 1;
                    break;
                case 0x2C: // Get time (stub)
                    cpu.regs.CH() = 12; cpu.regs.CL() = 0; cpu.regs.DH() = 0; cpu.regs.DL() = 0;
                    break;
                case 0x4C: // Terminate with return code
                    halted = true;
                    break;
                default: {
                    char buf[64];
                    snprintf(buf, sizeof(buf), "Unimplemented INT 21h func: AH=%02X", ah);
                    warnings.push_back(std::string(buf));
                    break;
                }
            }
            return true;
        }
        default: {
            char buf[64];
            snprintf(buf, sizeof(buf), "Unimplemented INT %02Xh", interrupt_number);
            warnings.push_back(std::string(buf));
            return true;
        }
    }
}

} // namespace emu8086::core
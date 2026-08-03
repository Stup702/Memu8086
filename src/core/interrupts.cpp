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
            if (non_blocking) return '\0';
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
                case 0x00: // Set video mode (stub - mode 3 text is default, just clear)
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
                case 0x06: // Scroll window up
                    // Real implementation would scroll a region; we treat it as clear if AL=0
                    if (cpu.regs.AL() == 0) io.clear_screen();
                    break;
                case 0x07: // Scroll window down (stub)
                    if (cpu.regs.AL() == 0) io.clear_screen();
                    break;
                case 0x08: // Read char+attr at cursor (stub - return space with default attr)
                    cpu.regs.AL() = ' ';
                    cpu.regs.AH() = 0x07; // white on black
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
                case 0x0F: // Get current video mode
                    cpu.regs.AL() = 0x03; // Mode 3: 80x25 color text
                    cpu.regs.AH() = 80;   // 80 columns
                    cpu.regs.BH() = 0;    // Active page 0
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
                case 0x00: // Wait for keypress - returns ASCII in AL, scan code in AH
                    {
                        char c = wait_and_read_key();
                        if (c == '\0') {
                            // Non-blocking: no key available — suspend and retry
                            cpu.regs.IP -= 2;
                            interrupt_suspended = true;
                            break;
                        }
                        interrupt_suspended = false;
                        cpu.regs.AL() = static_cast<uint8_t>(c);
                        cpu.regs.AH() = static_cast<uint8_t>(c); // simplified scan code approximation
                    }
                    break;
                case 0x01: // Check keystroke buffer - ZF=1 if no key, ZF=0 if key available
                    if (has_input()) {
                        cpu.regs.flags.ZF = false;
                        std::lock_guard<std::mutex> lock(input_mutex);
                        cpu.regs.AL() = input_queue.front(); // Peek without consuming
                        cpu.regs.AH() = input_queue.front(); // Scan code approximation
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
                    if (c == '\0') {
                        cpu.regs.IP -= 2;
                        interrupt_suspended = true;
                        break;
                    }
                    interrupt_suspended = false;
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
                            char c = wait_and_read_key();
                            if (c == '\0') break;
                            cpu.regs.AL() = c;
                            cpu.regs.flags.ZF = false;
                        } else {
                            cpu.regs.AL() = 0x00;
                            cpu.regs.flags.ZF = true;
                        }
                    }
                    break;
                }
                case 0x07: // Read char no echo
                case 0x08: // Read char no echo (alternate)
                    {
                        char c = wait_and_read_key();
                        if (c == '\0') {
                            cpu.regs.IP -= 2;
                            interrupt_suspended = true;
                            break;
                        }
                        interrupt_suspended = false;
                        cpu.regs.AL() = c;
                    }
                    break;
                case 0x09: { // Print string ($-terminated)
                    uint32_t addr = cpu.ds_addr(cpu.regs.DX);
                    int safety_limit = 0xFFFF;
                    while (safety_limit-- > 0) {
                        char c = static_cast<char>(cpu.mem.read8(addr++));
                        if (c == '$') break;
                        io.write_char(c);
                    }
                    break;
                }
                case 0x0A: { // Buffered keyboard input
                    // This interrupt is difficult to make non-blocking mid-input because
                    // the buffer state lives on the stack. We use a persistent input_line
                    // accumulator on the InterruptHandler to survive yields.
                    uint32_t addr = cpu.ds_addr(cpu.regs.DX);
                    uint8_t max_chars = cpu.mem.read8(addr);
                    uint8_t writable = (max_chars > 0) ? (max_chars - 1) : 0;

                    while (true) {
                        char c = wait_and_read_key();
                        if (c == '\0') {
                            // No input yet — suspend and retry. Partial state is preserved
                            // in input_line_buf / input_line_count on this object.
                            cpu.regs.IP -= 2;
                            interrupt_suspended = true;
                            // Write current partial count so caller can see progress
                            cpu.mem.write8(addr + 1, input_line_count);
                            break;
                        }
                        interrupt_suspended = false;

                        if (c == '\r' || c == '\n') {
                            // Flush accumulated line to the DOS buffer
                            for (uint8_t i = 0; i < input_line_count; i++) {
                                cpu.mem.write8(addr + 2 + i, input_line_buf[i]);
                            }
                            cpu.mem.write8(addr + 2 + input_line_count, '\r');
                            cpu.mem.write8(addr + 1, input_line_count);
                            io.write_char('\r');
                            io.write_char('\n');
                            input_line_count = 0; // reset for next call
                            break;
                        } else if (c == '\b' || c == 0x7F) { // Backspace
                            if (input_line_count > 0) {
                                input_line_count--;
                                io.write_char('\b'); io.write_char(' '); io.write_char('\b');
                            }
                        } else if (input_line_count < writable) {
                            input_line_buf[input_line_count++] = c;
                            io.write_char(c);
                        }
                    }
                    break;
                }
                case 0x0B: // Check input status
                    cpu.regs.AL() = has_input() ? 0xFF : 0x00;
                    break;
                case 0x25: { // Set interrupt vector (DS:DX -> IVT[AL*4])
                    uint8_t vec = cpu.regs.AL();
                    uint32_t ivt_addr = vec * 4;
                    cpu.mem.write16(ivt_addr,     cpu.regs.DX);
                    cpu.mem.write16(ivt_addr + 2, cpu.regs.DS);
                    break;
                }
                case 0x2A: // Get date (stub)
                    cpu.regs.CX = 2024; cpu.regs.DH() = 1; cpu.regs.DL() = 1;
                    cpu.regs.AL() = 1; // Monday
                    break;
                case 0x2C: // Get time (stub)
                    cpu.regs.CH() = 12; cpu.regs.CL() = 0; cpu.regs.DH() = 0; cpu.regs.DL() = 0;
                    break;
                case 0x30: // Get DOS version
                    cpu.regs.AL() = 5; // DOS 5.0 (same as emu8086)
                    cpu.regs.AH() = 0;
                    cpu.regs.BH() = 0xFF; // MS-DOS
                    break;
                case 0x35: { // Get interrupt vector -> ES:BX = IVT[AL*4]
                    uint8_t vec = cpu.regs.AL();
                    uint32_t ivt_addr = vec * 4;
                    cpu.regs.BX = cpu.mem.read16(ivt_addr);
                    cpu.regs.ES = cpu.mem.read16(ivt_addr + 2);
                    break;
                }
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
            return false;
        }
    }
}

} // namespace emu8086::core
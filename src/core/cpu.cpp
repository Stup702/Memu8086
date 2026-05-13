#include "cpu.h"

namespace emu8086::core {

// --- Flags ---

// Pack flags into a 16-bit word (mirrors 8086 FLAGS register architecture)
uint16_t Flags::to_word() const {
    return (CF << 0) | (1 << 1) | (PF << 2) | (AF << 4) |
           (ZF << 6) | (SF << 7) | (TF << 8) | (IF << 9) |
           (DF << 10) | (OF << 11) | 0xF000;
}

// Unpack 16-bit word into the individual boolean flags
void Flags::from_word(uint16_t w) {
    CF = (w >> 0) & 1;
    PF = (w >> 2) & 1;
    AF = (w >> 4) & 1;
    ZF = (w >> 6) & 1;
    SF = (w >> 7) & 1;
    TF = (w >> 8) & 1;
    IF = (w >> 9) & 1;
    DF = (w >> 10) & 1;
    OF = (w >> 11) & 1;
}

// Clear all flags back to zero
void Flags::reset() {
    from_word(0);
}

// --- Registers ---

// Reset general purpose, segment, and pointer registers to their boot state
void Registers::reset() {
    AX = BX = CX = DX = 0;
    SI = DI = BP = 0;
    CS = DS = ES = SS = 0x0000;
    SP = 0xFFFE;
    IP = 0x0000;
    flags.reset();
}

// --- Memory ---

// Clear entire 1MB memory space
void Memory::reset() {
    data.fill(0);
}

// Read an 8-bit byte from physical memory
uint8_t Memory::read8(uint32_t physical_addr) const {
    return data[physical_addr % MEMORY_SIZE];
}

// Read a 16-bit word from physical memory (little-endian: low byte at lower address)
uint16_t Memory::read16(uint32_t physical_addr) const {
    uint8_t lo = read8(physical_addr);
    uint8_t hi = read8(physical_addr + 1);
    return static_cast<uint16_t>(lo | (hi << 8));
}

// Write an 8-bit byte to physical memory
void Memory::write8(uint32_t physical_addr, uint8_t val) {
    data[physical_addr % MEMORY_SIZE] = val;
}

// Write a 16-bit word to physical memory (little-endian)
void Memory::write16(uint32_t physical_addr, uint16_t val) {
    write8(physical_addr, static_cast<uint8_t>(val & 0xFF));
    write8(physical_addr + 1, static_cast<uint8_t>(val >> 8));
}

// Return direct pointer for memory access (e.g., DMA, ImGui video buffer visualization)
uint8_t* Memory::raw_ptr(uint32_t physical_addr) {
    return &data[physical_addr % MEMORY_SIZE];
}

// Calculate physical address using x86 16-bit segmented memory model
uint32_t Memory::segment_offset(uint16_t seg, uint16_t off) {
    return (static_cast<uint32_t>(seg) << 4) + off;
}

// --- CPU ---

// Reset the CPU state and empty the memory
void CPU::reset() {
    regs.reset();
    mem.reset();
}

// Calculate the physical address of the current instruction pointer
uint32_t CPU::cs_ip() const {
    return Memory::segment_offset(regs.CS, regs.IP);
}

// Calculate the physical address of the top of the stack
uint32_t CPU::ss_sp() const {
    return Memory::segment_offset(regs.SS, regs.SP);
}

// Calculate a physical address relative to the current Data Segment
uint32_t CPU::ds_addr(uint16_t offset) const {
    return Memory::segment_offset(regs.DS, offset);
}

// Calculate a physical address relative to the current Extra Segment
uint32_t CPU::es_addr(uint16_t offset) const {
    return Memory::segment_offset(regs.ES, offset);
}

// Calculate the parity of a byte (returns true if the number of set bits is even)
bool CPU::calc_parity(uint8_t val) {
    val ^= val >> 4;
    val ^= val >> 2;
    val ^= val >> 1;
    return (val & 1) == 0;
}

// Update flags after a logical operation (AND, OR, XOR, etc.)
void CPU::update_flags_logical(uint16_t result, bool word) {
    regs.flags.CF = false;
    regs.flags.OF = false;
    regs.flags.ZF = (result == 0);
    regs.flags.SF = word ? (result & 0x8000) != 0 : (result & 0x80) != 0;
    regs.flags.PF = calc_parity(static_cast<uint8_t>(result & 0xFF));
}

// Update flags after an addition operation
void CPU::update_flags_add(uint32_t a, uint32_t b, uint32_t result, bool word) {
    regs.flags.CF = word ? (result > 0xFFFF) : (result > 0xFF);
    uint16_t mask = word ? 0x8000 : 0x80;
    regs.flags.OF = ((a ^ result) & (b ^ result) & mask) != 0;
}

// Update flags after a subtraction operation
void CPU::update_flags_sub(uint32_t a, uint32_t b, uint32_t result, bool word) {
    regs.flags.CF = b > a; // unsigned borrow
    uint16_t mask = word ? 0x8000 : 0x80;
    regs.flags.OF = ((a ^ b) & (a ^ result) & mask) != 0;
}

} // namespace emu8086::core
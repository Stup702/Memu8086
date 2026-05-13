#pragma once

#include <cstdint>
#include <array>

namespace emu8086::core {

// --- Constants ---
constexpr uint32_t MEMORY_SIZE = 1 << 20; // 1MB
constexpr uint16_t STACK_SEGMENT_DEFAULT = 0x0000;
constexpr uint16_t CODE_SEGMENT_DEFAULT  = 0x0100;

// --- Flags ---
struct Flags {
    bool CF{false}; // Carry
    bool PF{false}; // Parity
    bool AF{false}; // Auxiliary Carry
    bool ZF{false}; // Zero
    bool SF{false}; // Sign
    bool TF{false}; // Trap
    bool IF{false}; // Interrupt Enable
    bool DF{false}; // Direction
    bool OF{false}; // Overflow

    uint16_t to_word() const;
    void     from_word(uint16_t w);
    void     reset();
};

// --- Registers ---
struct Registers {
    uint16_t AX{0}, BX{0}, CX{0}, DX{0};
    uint16_t SI{0}, DI{0}, SP{0}, BP{0};
    uint16_t CS{0}, DS{0}, ES{0}, SS{0};
    uint16_t IP{0};
    Flags    flags;

    // Byte-level references (assuming little-endian host architecture)
    inline uint8_t& AL() { return reinterpret_cast<uint8_t*>(&AX)[0]; }
    inline uint8_t& AH() { return reinterpret_cast<uint8_t*>(&AX)[1]; }
    inline uint8_t& BL() { return reinterpret_cast<uint8_t*>(&BX)[0]; }
    inline uint8_t& BH() { return reinterpret_cast<uint8_t*>(&BX)[1]; }
    inline uint8_t& CL() { return reinterpret_cast<uint8_t*>(&CX)[0]; }
    inline uint8_t& CH() { return reinterpret_cast<uint8_t*>(&CX)[1]; }
    inline uint8_t& DL() { return reinterpret_cast<uint8_t*>(&DX)[0]; }
    inline uint8_t& DH() { return reinterpret_cast<uint8_t*>(&DX)[1]; }

    void reset();
};

// --- Memory ---
class Memory {
private:
    std::array<uint8_t, MEMORY_SIZE> data{};

public:
    void     reset();
    uint8_t  read8 (uint32_t physical_addr) const;
    uint16_t read16(uint32_t physical_addr) const;
    void     write8 (uint32_t physical_addr, uint8_t  val);
    void     write16(uint32_t physical_addr, uint16_t val);
    uint8_t* raw_ptr(uint32_t physical_addr);
    const std::array<uint8_t, MEMORY_SIZE>& get_data() const { return data; }
    static uint32_t segment_offset(uint16_t seg, uint16_t off);
};

// --- CPU ---
struct CPU {
    Registers regs;
    Memory    mem;

    void reset();
    uint32_t cs_ip() const;
    uint32_t ss_sp() const;
    uint32_t ds_addr(uint16_t offset) const;
    uint32_t es_addr(uint16_t offset) const;

    void update_flags_logical(uint16_t result, bool word);
    void update_flags_add(uint32_t a, uint32_t b, uint32_t result, bool word);
    void update_flags_sub(uint32_t a, uint32_t b, uint32_t result, bool word);

    static bool calc_parity(uint8_t val);
};

} // namespace emu8086::core
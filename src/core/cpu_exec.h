#pragma once

#include "cpu.h"

namespace emu8086::core {

enum class ExecResult : uint8_t {
    OK,
    HALT,
    ILLEGAL_OPCODE,
    DIVISION_BY_ZERO,
    BREAKPOINT,
    INTERRUPT_PENDING
};

struct ModRMResult {
    union {
        uint16_t* reg_ptr; // Pointer to target register
        uint8_t*  reg8_ptr;
    };
    uint32_t mem_addr; // Resolved physical memory address
    uint16_t offset;   // Effective address offset
    bool is_mem;       // True if operating on memory
    uint8_t reg_idx;   // Extracted reg index
};

class Executor {
public:
    explicit Executor(CPU& cpu);

    ExecResult step();
    void       reset();

    uint8_t    last_opcode{0};
    uint32_t   cycle_count{0};
    uint8_t    last_interrupt_num{0};

private:
    CPU& cpu;

    uint8_t  fetch8();
    uint16_t fetch16();
    int8_t   fetch_s8();
    int16_t  fetch_s16();

    ModRMResult decode_modrm(uint8_t modrm, bool word, uint16_t seg_override);
    uint32_t read_rm(const ModRMResult& rm, bool word);
    void     write_rm(const ModRMResult& rm, bool word, uint32_t val);
    uint16_t* get_reg16(uint8_t idx);
    uint8_t*  get_reg8(uint8_t idx);
    uint16_t* get_seg_reg(uint8_t idx);

    bool check_jcc(uint8_t cond);
    void alu_op(uint8_t op_type, bool word, uint32_t v1, uint32_t v2, uint32_t& out_res, bool& write_back);
};

} // namespace emu8086::core
#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace emu8086::core {

struct DisasmLine {
    uint32_t address;
    std::vector<uint8_t> raw_bytes;
    std::string mnemonic;
    std::string operands;
    std::string comment;
};

class Disassembler {
public:
    std::vector<DisasmLine> disassemble(const uint8_t* mem, uint32_t start_addr, int count);

private:
    std::string parse_modrm(const uint8_t* mem, uint32_t& addr, std::vector<uint8_t>& raw_bytes, bool word, int& out_reg);
};

} // namespace emu8086::core
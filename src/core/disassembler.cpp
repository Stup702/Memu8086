#include "disassembler.h"
#include <fmt/core.h>

namespace emu8086::core {

static const char* reg8_names[] = {"AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH"};
static const char* reg16_names[] = {"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"};
static const char* seg_names[] = {"ES", "CS", "SS", "DS"};
static const char* alu_ops[] = {"ADD", "OR", "ADC", "SBB", "AND", "SUB", "XOR", "CMP"};

std::string Disassembler::parse_modrm(const uint8_t* mem, uint32_t& addr, std::vector<uint8_t>& raw_bytes, bool word, int& out_reg) {
    uint8_t modrm = mem[addr++];
    raw_bytes.push_back(modrm);
    
    uint8_t mod = (modrm >> 6) & 3;
    out_reg = (modrm >> 3) & 7;
    uint8_t rm = modrm & 7;
    
    if (mod == 3) {
        return word ? reg16_names[rm] : reg8_names[rm];
    }
    
    std::string base;
    switch (rm) {
        case 0: base = "BX+SI"; break;
        case 1: base = "BX+DI"; break;
        case 2: base = "BP+SI"; break;
        case 3: base = "BP+DI"; break;
        case 4: base = "SI"; break;
        case 5: base = "DI"; break;
        case 6: base = (mod == 0) ? "" : "BP"; break;
        case 7: base = "BX"; break;
    }
    
    int16_t disp = 0;
    if (mod == 1) {
        disp = static_cast<int8_t>(mem[addr++]);
        raw_bytes.push_back(disp & 0xFF);
    } else if (mod == 2 || (mod == 0 && rm == 6)) {
        uint8_t lo = mem[addr++]; raw_bytes.push_back(lo);
        uint8_t hi = mem[addr++]; raw_bytes.push_back(hi);
        disp = static_cast<int16_t>((hi << 8) | lo);
    }
    
    if (mod == 0 && rm == 6) {
        return fmt::format("[0x{:04X}]", static_cast<uint16_t>(disp));
    }
    
    if (disp == 0) return fmt::format("[{}]", base);
    if (disp > 0) return fmt::format("[{}+0x{:02X}]", base, disp);
    return fmt::format("[{}-0x{:02X}]", base, -disp);
}

std::vector<DisasmLine> Disassembler::disassemble(const uint8_t* mem, uint32_t start_addr, int count) {
    std::vector<DisasmLine> lines;
    uint32_t addr = start_addr;
    
    for (int i = 0; i < count && addr < 0xFFFFF; i++) {
        DisasmLine line;
        line.address = addr;
        
        auto fetch8 = [&]() { uint8_t b = mem[addr++]; line.raw_bytes.push_back(b); return b; };
        auto fetch16 = [&]() { uint8_t lo = fetch8(); uint8_t hi = fetch8(); return static_cast<uint16_t>((hi << 8) | lo); };
        auto fetch_s8 = [&]() { return static_cast<int8_t>(fetch8()); };
        
        uint8_t op = fetch8();
        
        if (op < 0x40) {
            uint8_t op_type = (op >> 3) & 7;
            bool dir = (op >> 1) & 1;
            bool word = op & 1;
            line.mnemonic = alu_ops[op_type];
            
            if ((op & 0xC) == 0x4) {
                line.operands = fmt::format("{}, 0x{:0X}", word ? "AX" : "AL", word ? fetch16() : fetch8());
            } else {
                int reg;
                std::string rm_str = parse_modrm(mem, addr, line.raw_bytes, word, reg);
                std::string reg_str = word ? reg16_names[reg] : reg8_names[reg];
                if (dir) line.operands = fmt::format("{}, {}", reg_str, rm_str);
                else line.operands = fmt::format("{}, {}", rm_str, reg_str);
            }
        }
        else if (op >= 0x40 && op <= 0x47) { line.mnemonic = "INC"; line.operands = reg16_names[op & 7]; }
        else if (op >= 0x48 && op <= 0x4F) { line.mnemonic = "DEC"; line.operands = reg16_names[op & 7]; }
        else if (op >= 0x50 && op <= 0x57) { line.mnemonic = "PUSH"; line.operands = reg16_names[op & 7]; }
        else if (op >= 0x58 && op <= 0x5F) { line.mnemonic = "POP"; line.operands = reg16_names[op & 7]; }
        else if (op >= 0x70 && op <= 0x7F) {
            const char* jcc[] = {"JO","JNO","JB","JAE","JE","JNE","JBE","JA","JS","JNS","JP","JNP","JL","JGE","JLE","JG"};
            line.mnemonic = jcc[op & 0xF];
            int8_t rel = fetch_s8();
            line.operands = fmt::format("0x{:04X}", (addr + rel) & 0xFFFF);
        }
        else if (op == 0x80 || op == 0x81 || op == 0x83) {
            bool word = (op != 0x80);
            int reg;
            std::string rm_str = parse_modrm(mem, addr, line.raw_bytes, word, reg);
            line.mnemonic = alu_ops[reg];
            uint16_t imm = (op == 0x81) ? fetch16() : fetch8();
            if (op == 0x83 && (imm & 0x80)) imm |= 0xFF00; // Sign extend
            line.operands = fmt::format("{}, 0x{:0X}", rm_str, imm);
        }
        else if (op == 0x88 || op == 0x89 || op == 0x8A || op == 0x8B) {
            bool word = op & 1;
            bool dir = op & 2;
            int reg;
            std::string rm_str = parse_modrm(mem, addr, line.raw_bytes, word, reg);
            std::string reg_str = word ? reg16_names[reg] : reg8_names[reg];
            line.mnemonic = "MOV";
            if (dir) line.operands = fmt::format("{}, {}", reg_str, rm_str);
            else line.operands = fmt::format("{}, {}", rm_str, reg_str);
        }
        else if (op == 0x8D) {
            int reg;
            std::string rm_str = parse_modrm(mem, addr, line.raw_bytes, true, reg);
            line.mnemonic = "LEA";
            line.operands = fmt::format("{}, {}", reg16_names[reg], rm_str);
        }
        else if (op == 0x90) { line.mnemonic = "NOP"; }
        else if (op >= 0xB0 && op <= 0xB7) {
            line.mnemonic = "MOV";
            line.operands = fmt::format("{}, 0x{:02X}", reg8_names[op & 7], fetch8());
        }
        else if (op >= 0xB8 && op <= 0xBF) {
            line.mnemonic = "MOV";
            line.operands = fmt::format("{}, 0x{:04X}", reg16_names[op & 7], fetch16());
        }
        else if (op == 0xC3) { line.mnemonic = "RET"; }
        else if (op == 0xCC) { line.mnemonic = "INT"; line.operands = "3"; }
        else if (op == 0xCD) { line.mnemonic = "INT"; line.operands = fmt::format("0x{:02X}", fetch8()); }
        else if (op == 0xE2) { line.mnemonic = "LOOP"; line.operands = fmt::format("0x{:04X}", (addr + fetch_s8()) & 0xFFFF); }
        else if (op == 0xE8) { line.mnemonic = "CALL"; line.operands = fmt::format("0x{:04X}", (addr + fetch16()) & 0xFFFF); }
        else if (op == 0xE9) { line.mnemonic = "JMP"; line.operands = fmt::format("0x{:04X}", (addr + fetch16()) & 0xFFFF); }
        else if (op == 0xEB) { line.mnemonic = "JMP"; line.operands = fmt::format("0x{:04X}", (addr + fetch_s8()) & 0xFFFF); }
        else if (op == 0xF4) { line.mnemonic = "HLT"; }
        else if (op == 0xFA) { line.mnemonic = "CLI"; }
        else if (op == 0xFB) { line.mnemonic = "STI"; }
        else {
            line.mnemonic = "DB";
            line.operands = fmt::format("0x{:02X}", op);
        }
        
        lines.push_back(line);
    }
    
    return lines;
}

} // namespace emu8086::core
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
        
        if (op < 0x40 && (op & 0x07) < 0x06) {
            uint8_t op_type = (op >> 3) & 7;
            bool dir = (op >> 1) & 1;
            bool word = op & 1;
            line.mnemonic = alu_ops[op_type];
            
            if ((op & 0x06) == 0x04) {
                line.operands = fmt::format("{}, 0x{:0X}", word ? "AX" : "AL", word ? fetch16() : fetch8());
            } else {
                int reg;
                std::string rm_str = parse_modrm(mem, addr, line.raw_bytes, word, reg);
                std::string reg_str = word ? reg16_names[reg] : reg8_names[reg];
                if (dir) line.operands = fmt::format("{}, {}", reg_str, rm_str);
                else line.operands = fmt::format("{}, {}", rm_str, reg_str);
            }
        }
        else if (op == 0x06 || op == 0x0E || op == 0x16 || op == 0x1E) { line.mnemonic = "PUSH"; line.operands = seg_names[(op >> 3) & 3]; }
        else if (op == 0x07 || op == 0x0F || op == 0x17 || op == 0x1F) { line.mnemonic = "POP"; line.operands = seg_names[(op >> 3) & 3]; }
        else if (op == 0x27) { line.mnemonic = "DAA"; }
        else if (op == 0x2F) { line.mnemonic = "DAS"; }
        else if (op == 0x37) { line.mnemonic = "AAA"; }
        else if (op == 0x3F) { line.mnemonic = "AAS"; }
        else if (op >= 0x40 && op <= 0x47) { line.mnemonic = "INC"; line.operands = reg16_names[op & 7]; }
        else if (op >= 0x48 && op <= 0x4F) { line.mnemonic = "DEC"; line.operands = reg16_names[op & 7]; }
        else if (op >= 0x50 && op <= 0x57) { line.mnemonic = "PUSH"; line.operands = reg16_names[op & 7]; }
        else if (op >= 0x58 && op <= 0x5F) { line.mnemonic = "POP"; line.operands = reg16_names[op & 7]; }
        else if (op == 0x68 || op == 0x6A) { line.mnemonic = "PUSH"; line.operands = fmt::format("0x{:0X}", (op == 0x68) ? fetch16() : fetch8()); }
        else if (op >= 0x70 && op <= 0x7F) {
            const char* jcc[] = {"JO","JNO","JB","JAE","JE","JNE","JBE","JA","JS","JNS","JP","JNP","JL","JGE","JLE","JG"};
            line.mnemonic = jcc[op & 0xF];
            int8_t rel = fetch_s8();
            line.operands = fmt::format("0x{:04X}", (addr + rel) & 0xFFFF);
        }
        else if (op >= 0x80 && op <= 0x83) {
            bool word = (op == 0x81 || op == 0x83);
            int reg;
            std::string rm_str = parse_modrm(mem, addr, line.raw_bytes, word, reg);
            line.mnemonic = alu_ops[reg];
            uint16_t imm = (op == 0x81) ? fetch16() : fetch8();
            if (op == 0x83 && (imm & 0x80)) imm |= 0xFF00; // Sign extend
            line.operands = fmt::format("{}, 0x{:0X}", rm_str, imm);
        }
        else if (op == 0x84 || op == 0x85) {
            bool word = op & 1; int reg; std::string rm = parse_modrm(mem, addr, line.raw_bytes, word, reg);
            line.mnemonic = "TEST"; line.operands = fmt::format("{}, {}", rm, word ? reg16_names[reg] : reg8_names[reg]);
        }
        else if (op == 0x86 || op == 0x87) {
            bool word = op & 1; int reg; std::string rm = parse_modrm(mem, addr, line.raw_bytes, word, reg);
            line.mnemonic = "XCHG"; line.operands = fmt::format("{}, {}", rm, word ? reg16_names[reg] : reg8_names[reg]);
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
        else if (op == 0x8C || op == 0x8E) {
            int reg; std::string rm = parse_modrm(mem, addr, line.raw_bytes, true, reg);
            line.mnemonic = "MOV"; line.operands = (op == 0x8C) ? fmt::format("{}, {}", rm, seg_names[reg & 3]) : fmt::format("{}, {}", seg_names[reg & 3], rm);
        }
        else if (op == 0x8F) { int reg; std::string rm = parse_modrm(mem, addr, line.raw_bytes, true, reg); line.mnemonic = "POP"; line.operands = rm; }
        else if (op == 0x8D) {
            int reg;
            std::string rm_str = parse_modrm(mem, addr, line.raw_bytes, true, reg);
            line.mnemonic = "LEA";
            line.operands = fmt::format("{}, {}", reg16_names[reg], rm_str);
        }
        else if (op == 0x90) { line.mnemonic = "NOP"; }
        else if (op >= 0x91 && op <= 0x97) { line.mnemonic = "XCHG"; line.operands = fmt::format("AX, {}", reg16_names[op & 7]); }
        else if (op == 0x98) { line.mnemonic = "CBW"; }
        else if (op == 0x99) { line.mnemonic = "CWD"; }
        else if (op == 0x9A) { uint16_t off = fetch16(); uint16_t seg = fetch16(); line.mnemonic = "CALL FAR"; line.operands = fmt::format("0x{:04X}:0x{:04X}", seg, off); }
        else if (op == 0x9C) { line.mnemonic = "PUSHF"; }
        else if (op == 0x9D) { line.mnemonic = "POPF"; }
        else if (op == 0x9E) { line.mnemonic = "SAHF"; }
        else if (op == 0x9F) { line.mnemonic = "LAHF"; }
        else if (op >= 0xA0 && op <= 0xA3) {
            bool word = op & 1; bool to_al = (op < 0xA2);
            line.mnemonic = "MOV";
            std::string moffs = fmt::format("[0x{:04X}]", fetch16());
            line.operands = to_al ? fmt::format("{}, {}", word ? "AX" : "AL", moffs) : fmt::format("{}, {}", moffs, word ? "AX" : "AL");
        }
        else if (op >= 0xA4 && op <= 0xA7) { line.mnemonic = (op == 0xA4 || op == 0xA5) ? "MOVS" : "CMPS"; line.mnemonic += (op & 1) ? "W" : "B"; }
        else if (op == 0xA8 || op == 0xA9) {
            bool word = op & 1;
            line.mnemonic = "TEST"; line.operands = fmt::format("{}, 0x{:0X}", word ? "AX" : "AL", word ? fetch16() : fetch8());
        }
        else if (op >= 0xAA && op <= 0xAF) {
            if (op == 0xAA || op == 0xAB) line.mnemonic = "STOS";
            else if (op == 0xAC || op == 0xAD) line.mnemonic = "LODS";
            else line.mnemonic = "SCAS";
            line.mnemonic += (op & 1) ? "W" : "B";
        }
        else if (op >= 0xB0 && op <= 0xB7) {
            line.mnemonic = "MOV";
            line.operands = fmt::format("{}, 0x{:02X}", reg8_names[op & 7], fetch8());
        }
        else if (op >= 0xB8 && op <= 0xBF) {
            line.mnemonic = "MOV";
            line.operands = fmt::format("{}, 0x{:04X}", reg16_names[op & 7], fetch16());
        }
        else if (op == 0xC2) { line.mnemonic = "RET"; line.operands = fmt::format("0x{:04X}", fetch16()); }
        else if (op == 0xC3) { line.mnemonic = "RET"; }
        else if (op == 0xC4 || op == 0xC5) {
            int reg; std::string rm = parse_modrm(mem, addr, line.raw_bytes, true, reg);
            line.mnemonic = (op == 0xC4) ? "LES" : "LDS"; line.operands = fmt::format("{}, {}", reg16_names[reg], rm);
        }
        else if (op == 0xC6 || op == 0xC7) {
            bool word = op & 1; int reg; std::string rm = parse_modrm(mem, addr, line.raw_bytes, word, reg);
            line.mnemonic = "MOV"; line.operands = fmt::format("{}, 0x{:0X}", rm, word ? fetch16() : fetch8());
        }
        else if (op == 0xCA) { line.mnemonic = "RETF"; line.operands = fmt::format("0x{:04X}", fetch16()); }
        else if (op == 0xCB) { line.mnemonic = "RETF"; }
        else if (op == 0xCC) { line.mnemonic = "INT"; line.operands = "3"; }
        else if (op == 0xCD) { line.mnemonic = "INT"; line.operands = fmt::format("0x{:02X}", fetch8()); }
        else if (op == 0xCE) { line.mnemonic = "INTO"; }
        else if (op == 0xCF) { line.mnemonic = "IRET"; }
        else if (op >= 0xD0 && op <= 0xD3) {
            bool word = op & 1; bool cl = op & 2; int reg; std::string rm = parse_modrm(mem, addr, line.raw_bytes, word, reg);
            const char* shifts[] = {"ROL", "ROR", "RCL", "RCR", "SHL", "SHR", "SAL", "SAR"};
            line.mnemonic = shifts[reg]; line.operands = fmt::format("{}, {}", rm, cl ? "CL" : "1");
        }
        else if (op == 0xD4) { line.mnemonic = "AAM"; line.operands = fmt::format("0x{:02X}", fetch8()); }
        else if (op == 0xD5) { line.mnemonic = "AAD"; line.operands = fmt::format("0x{:02X}", fetch8()); }
        else if (op == 0xD7) { line.mnemonic = "XLAT"; }
        else if (op == 0xE0) { line.mnemonic = "LOOPNE"; line.operands = fmt::format("0x{:04X}", (addr + fetch_s8()) & 0xFFFF); }
        else if (op == 0xE1) { line.mnemonic = "LOOPE"; line.operands = fmt::format("0x{:04X}", (addr + fetch_s8()) & 0xFFFF); }
        else if (op == 0xE2) { line.mnemonic = "LOOP"; line.operands = fmt::format("0x{:04X}", (addr + fetch_s8()) & 0xFFFF); }
        else if (op == 0xE3) { line.mnemonic = "JCXZ"; line.operands = fmt::format("0x{:04X}", (addr + fetch_s8()) & 0xFFFF); }
        else if (op == 0xE4 || op == 0xE5) { line.mnemonic = "IN"; line.operands = fmt::format("{}, 0x{:02X}", (op & 1) ? "AX" : "AL", fetch8()); }
        else if (op == 0xE6 || op == 0xE7) { line.mnemonic = "OUT"; line.operands = fmt::format("0x{:02X}, {}", fetch8(), (op & 1) ? "AX" : "AL"); }
        else if (op == 0xE8) { line.mnemonic = "CALL"; line.operands = fmt::format("0x{:04X}", (addr + fetch16()) & 0xFFFF); }
        else if (op == 0xE9) { line.mnemonic = "JMP"; line.operands = fmt::format("0x{:04X}", (addr + fetch16()) & 0xFFFF); }
        else if (op == 0xEA) { uint16_t off = fetch16(); uint16_t seg = fetch16(); line.mnemonic = "JMP FAR"; line.operands = fmt::format("0x{:04X}:0x{:04X}", seg, off); }
        else if (op == 0xEB) { line.mnemonic = "JMP"; line.operands = fmt::format("0x{:04X}", (addr + fetch_s8()) & 0xFFFF); }
        else if (op == 0xEC || op == 0xED) { line.mnemonic = "IN"; line.operands = fmt::format("{}, DX", (op & 1) ? "AX" : "AL"); }
        else if (op == 0xEE || op == 0xEF) { line.mnemonic = "OUT"; line.operands = fmt::format("DX, {}", (op & 1) ? "AX" : "AL"); }
        else if (op == 0xF0) { line.mnemonic = "LOCK"; }
        else if (op == 0xF2) { line.mnemonic = "REPNE"; }
        else if (op == 0xF3) { line.mnemonic = "REP"; }
        else if (op == 0xF4) { line.mnemonic = "HLT"; }
        else if (op == 0xF5) { line.mnemonic = "CMC"; }
        else if (op == 0xF8) { line.mnemonic = "CLC"; }
        else if (op == 0xF9) { line.mnemonic = "STC"; }
        else if (op == 0xFA) { line.mnemonic = "CLI"; }
        else if (op == 0xFB) { line.mnemonic = "STI"; }
        else if (op == 0xFC) { line.mnemonic = "CLD"; }
        else if (op == 0xFD) { line.mnemonic = "STD"; }
        else if (op == 0xF6 || op == 0xF7) {
            bool word = op & 1; int reg; std::string rm = parse_modrm(mem, addr, line.raw_bytes, word, reg);
            const char* grp3[] = {"TEST", "TEST", "NOT", "NEG", "MUL", "IMUL", "DIV", "IDIV"};
            line.mnemonic = grp3[reg];
            line.operands = (reg <= 1) ? fmt::format("{}, 0x{:0X}", rm, word ? fetch16() : fetch8()) : rm;
        }
        else if (op == 0xFE || op == 0xFF) {
            bool word = op & 1; int reg; std::string rm = parse_modrm(mem, addr, line.raw_bytes, word, reg);
            const char* grp4[] = {"INC", "DEC", "CALL", "CALL FAR", "JMP", "JMP FAR", "PUSH", "???"};
            line.mnemonic = grp4[reg]; line.operands = rm;
        }
        else {
            line.mnemonic = "DB";
            line.operands = fmt::format("0x{:02X}", op);
        }
        
        lines.push_back(line);
    }
    
    return lines;
}

} // namespace emu8086::core
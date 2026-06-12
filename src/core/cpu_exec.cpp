#include "cpu_exec.h"

namespace emu8086::core {

Executor::Executor(CPU& cpu) : cpu(cpu) {}

void Executor::reset() {
    last_opcode = 0;
    cycle_count = 0;
}

// --- Fetch Helpers ---
uint8_t Executor::fetch8() {
    uint32_t addr = cpu.cs_ip();
    cpu.regs.IP++;
    return cpu.mem.read8(addr);
}

uint16_t Executor::fetch16() {
    uint8_t lo = fetch8();
    uint8_t hi = fetch8();
    return static_cast<uint16_t>((hi << 8) | lo);
}

int8_t Executor::fetch_s8() {
    return static_cast<int8_t>(fetch8());
}

int16_t Executor::fetch_s16() {
    return static_cast<int16_t>(fetch16());
}

// --- Register Fetch Helpers ---
uint16_t* Executor::get_reg16(uint8_t idx) {
    switch (idx & 7) {
        case 0: return &cpu.regs.AX; case 1: return &cpu.regs.CX;
        case 2: return &cpu.regs.DX; case 3: return &cpu.regs.BX;
        case 4: return &cpu.regs.SP; case 5: return &cpu.regs.BP;
        case 6: return &cpu.regs.SI; case 7: return &cpu.regs.DI;
        default: return &cpu.regs.AX;
    }
}

uint8_t* Executor::get_reg8(uint8_t idx) {
    switch (idx & 7) {
        case 0: return &cpu.regs.AL(); case 1: return &cpu.regs.CL();
        case 2: return &cpu.regs.DL(); case 3: return &cpu.regs.BL();
        case 4: return &cpu.regs.AH(); case 5: return &cpu.regs.CH();
        case 6: return &cpu.regs.DH(); case 7: return &cpu.regs.BH();
        default: return &cpu.regs.AL();
    }
}

uint16_t* Executor::get_seg_reg(uint8_t idx) {
    switch (idx & 3) {
        case 0: return &cpu.regs.ES; case 1: return &cpu.regs.CS;
        case 2: return &cpu.regs.SS; case 3: return &cpu.regs.DS;
        default: return &cpu.regs.DS;
    }
}

// --- ModRM Decoding & Access ---
ModRMResult Executor::decode_modrm(uint8_t modrm, bool word, uint32_t seg_override) {
    ModRMResult res{};
    uint8_t mod = (modrm >> 6) & 3;
    res.reg_idx = (modrm >> 3) & 7;
    uint8_t rm = modrm & 7;

    res.is_mem = (mod != 3);
    if (!res.is_mem) {
        if (word) res.reg_ptr = get_reg16(rm);
        else res.reg8_ptr = get_reg8(rm);
    } else {
        uint16_t offset = 0;
        uint16_t default_seg = cpu.regs.DS;
        switch (rm) {
            case 0: offset = cpu.regs.BX + cpu.regs.SI; break;
            case 1: offset = cpu.regs.BX + cpu.regs.DI; break;
            case 2: offset = cpu.regs.BP + cpu.regs.SI; default_seg = cpu.regs.SS; break;
            case 3: offset = cpu.regs.BP + cpu.regs.DI; default_seg = cpu.regs.SS; break;
            case 4: offset = cpu.regs.SI; break;
            case 5: offset = cpu.regs.DI; break;
            case 6:
                if (mod == 0) offset = fetch16();
                else { offset = cpu.regs.BP; default_seg = cpu.regs.SS; }
                break;
            case 7: offset = cpu.regs.BX; break;
        }
        if (mod == 1) offset += fetch_s8();
        else if (mod == 2) offset += fetch16();

        res.offset = offset;
        uint16_t seg = (seg_override != 0xFFFFFFFF) ? seg_override : default_seg;
        res.mem_addr = Memory::segment_offset(seg, offset);
    }
    return res;
}

uint32_t Executor::read_rm(const ModRMResult& rm, bool word) {
    if (rm.is_mem) return word ? cpu.mem.read16(rm.mem_addr) : cpu.mem.read8(rm.mem_addr);
    else return word ? *rm.reg_ptr : *rm.reg8_ptr;
}

void Executor::write_rm(const ModRMResult& rm, bool word, uint32_t val) {
    if (rm.is_mem) {
        if (word) cpu.mem.write16(rm.mem_addr, static_cast<uint16_t>(val));
        else cpu.mem.write8(rm.mem_addr, static_cast<uint8_t>(val));
    } else {
        if (word) *rm.reg_ptr = static_cast<uint16_t>(val);
        else *rm.reg8_ptr = static_cast<uint8_t>(val);
    }
}

// --- ALU Operations ---
void Executor::alu_op(uint8_t op_type, bool word, uint32_t v1, uint32_t v2, uint32_t& out_res, bool& write_back) {
    write_back = true;
    switch (op_type) {
        case 0: // ADD
            out_res = v1 + v2;
            cpu.update_flags_add(v1, v2, out_res, word);
            break;
        case 1: // OR
            out_res = v1 | v2;
            cpu.update_flags_logical(static_cast<uint16_t>(out_res), word);
            break;
        case 2: // ADC
            out_res = v1 + v2 + (cpu.regs.flags.CF ? 1 : 0);
            cpu.update_flags_add(v1, v2, out_res, word);
            break;
        case 3: // SBB
            out_res = v1 - v2 - (cpu.regs.flags.CF ? 1 : 0);
            cpu.update_flags_sub(v1, v2, out_res, word);
            break;
        case 4: // AND
            out_res = v1 & v2;
            cpu.update_flags_logical(static_cast<uint16_t>(out_res), word);
            break;
        case 5: // SUB
            out_res = v1 - v2;
            cpu.update_flags_sub(v1, v2, out_res, word);
            break;
        case 6: // XOR
            out_res = v1 ^ v2;
            cpu.update_flags_logical(static_cast<uint16_t>(out_res), word);
            break;
        case 7: // CMP
            out_res = v1 - v2;
            cpu.update_flags_sub(v1, v2, out_res, word);
            write_back = false;
            break;
    }
}

// --- Condition Check for Jcc ---
bool Executor::check_jcc(uint8_t cond) {
    switch (cond) {
        case 0x0: return cpu.regs.flags.OF; // JO
        case 0x1: return !cpu.regs.flags.OF; // JNO
        case 0x2: return cpu.regs.flags.CF; // JB/JC/JNAE
        case 0x3: return !cpu.regs.flags.CF; // JAE/JNB/JNC
        case 0x4: return cpu.regs.flags.ZF; // JE/JZ
        case 0x5: return !cpu.regs.flags.ZF; // JNE/JNZ
        case 0x6: return cpu.regs.flags.CF || cpu.regs.flags.ZF; // JBE/JNA
        case 0x7: return !cpu.regs.flags.CF && !cpu.regs.flags.ZF; // JA/JNBE
        case 0x8: return cpu.regs.flags.SF; // JS
        case 0x9: return !cpu.regs.flags.SF; // JNS
        case 0xA: return cpu.regs.flags.PF; // JP/JPE
        case 0xB: return !cpu.regs.flags.PF; // JNP/JPO
        case 0xC: return cpu.regs.flags.SF != cpu.regs.flags.OF; // JL/JNGE
        case 0xD: return cpu.regs.flags.SF == cpu.regs.flags.OF; // JGE/JNL
        case 0xE: return cpu.regs.flags.ZF || (cpu.regs.flags.SF != cpu.regs.flags.OF); // JLE/JNG
        case 0xF: return !cpu.regs.flags.ZF && (cpu.regs.flags.SF == cpu.regs.flags.OF); // JG/JNLE
    }
    return false;
}

// --- Main Dispatch Cycle ---
ExecResult Executor::step() {
    bool prefix = true;
    uint32_t seg_override = 0xFFFFFFFF;
    bool rep = false, repz = false, repnz = false;

    while (prefix) {
        last_opcode = fetch8();
        switch (last_opcode) {
            case 0x26: seg_override = cpu.regs.ES; break;
            case 0x2E: seg_override = cpu.regs.CS; break;
            case 0x36: seg_override = cpu.regs.SS; break;
            case 0x3E: seg_override = cpu.regs.DS; break;
            case 0xF0: break; // LOCK prefix (no-op in basic emulator)
            case 0xF3: rep = true; repz = true; break;
            case 0xF2: rep = true; repnz = true; break;
            default: prefix = false; break;
        }
    }
    cycle_count++;
    uint8_t op = last_opcode;

    // --- Arithmetic & Logic (0x00 - 0x3F) ---
    if (op < 0x40 && (op & 0x07) < 0x06) {
        uint8_t op_type = (op >> 3) & 7;
        bool dir = (op >> 1) & 1;
        bool word = op & 1;
        
        if ((op & 0x06) == 0x04) { // AL/AX immediate accumulator short forms
            uint32_t v1 = word ? cpu.regs.AX : cpu.regs.AL();
            uint32_t v2 = word ? fetch16() : fetch8();
            uint32_t res; bool wb;
            alu_op(op_type, word, v1, v2, res, wb);
            if (wb) { if (word) cpu.regs.AX = static_cast<uint16_t>(res); else cpu.regs.AL() = static_cast<uint8_t>(res); }
        } else { // ModRM forms
            uint8_t modrm = fetch8();
            auto rm = decode_modrm(modrm, word, seg_override);
            uint32_t v_rm = read_rm(rm, word);
            uint32_t v_reg = word ? *get_reg16(rm.reg_idx) : *get_reg8(rm.reg_idx);
            
            uint32_t v1 = dir ? v_reg : v_rm;
            uint32_t v2 = dir ? v_rm : v_reg;
            uint32_t res; bool wb;
            alu_op(op_type, word, v1, v2, res, wb);
            
            if (wb) {
                if (dir) { if (word) *get_reg16(rm.reg_idx) = static_cast<uint16_t>(res); else *get_reg8(rm.reg_idx) = static_cast<uint8_t>(res); }
                else { write_rm(rm, word, res); }
            }
        }
        return ExecResult::OK;
    }

    switch (op) {
        case 0x06: case 0x0E: case 0x16: case 0x1E: { // PUSH seg
            cpu.regs.SP -= 2;
            cpu.mem.write16(cpu.ss_sp(), *get_seg_reg((op >> 3) & 3));
            break;
        }
        case 0x07: case 0x0F: case 0x17: case 0x1F: { // POP seg
            *get_seg_reg((op >> 3) & 3) = cpu.mem.read16(cpu.ss_sp());
            cpu.regs.SP += 2;
            break;
        }
        // --- Arithmetic & Reg Access ---
        case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: { // INC reg16
            uint16_t& reg = *get_reg16(op & 7);
            bool old_cf = cpu.regs.flags.CF;
            uint32_t res = reg + 1;
            cpu.update_flags_add(reg, 1, res, true);
            cpu.regs.flags.CF = old_cf; // INC does not affect CF
            reg = static_cast<uint16_t>(res);
            break;
        }
        case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F: { // DEC reg16
            uint16_t& reg = *get_reg16(op & 7);
            bool old_cf = cpu.regs.flags.CF;
            uint32_t res = reg - 1;
            cpu.update_flags_sub(reg, 1, res, true);
            cpu.regs.flags.CF = old_cf;
            reg = static_cast<uint16_t>(res);
            break;
        }
        case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: { // PUSH reg16
            cpu.regs.SP -= 2;
            cpu.mem.write16(cpu.ss_sp(), *get_reg16(op & 7));
            break;
        }
        case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F: { // POP reg16
            uint16_t val = cpu.mem.read16(cpu.ss_sp());
            cpu.regs.SP += 2;
            *get_reg16(op & 7) = val;
            break;
        }
        case 0x68: { // PUSH imm16
            uint16_t imm = fetch16();
            cpu.regs.SP -= 2; cpu.mem.write16(cpu.ss_sp(), imm);
            break;
        }
        case 0x6A: { // PUSH imm8
            uint16_t imm = static_cast<uint16_t>(fetch_s8());
            cpu.regs.SP -= 2; cpu.mem.write16(cpu.ss_sp(), imm);
            break;
        }
        case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: // Jcc
        case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F: {
            int8_t offset = fetch_s8();
            if (check_jcc(op & 0xF)) cpu.regs.IP += offset;
            break;
        }
        case 0x80: case 0x81: case 0x82: case 0x83: { // ALU r/m, imm
            bool word = (op == 0x81 || op == 0x83);
            bool sign_ext = (op == 0x83);
            uint8_t modrm = fetch8();
            auto rm = decode_modrm(modrm, word, seg_override);
            uint32_t v1 = read_rm(rm, word);
            uint32_t v2 = word ? (sign_ext ? static_cast<uint16_t>(fetch_s8()) : fetch16()) : fetch8();
            uint32_t res; bool wb;
            alu_op(rm.reg_idx, word, v1, v2, res, wb);
            if (wb) write_rm(rm, word, res);
            break;
        }
        case 0x84: case 0x85: { // TEST r/m, reg
            bool word = op & 1;
            uint8_t modrm = fetch8();
            auto rm = decode_modrm(modrm, word, seg_override);
            uint32_t v1 = read_rm(rm, word);
            uint32_t v2 = word ? *get_reg16(rm.reg_idx) : *get_reg8(rm.reg_idx);
            cpu.update_flags_logical(static_cast<uint16_t>(v1 & v2), word);
            break;
        }
        case 0x86: case 0x87: { // XCHG r/m, reg
            bool word = op & 1;
            uint8_t modrm = fetch8();
            auto rm = decode_modrm(modrm, word, seg_override);
            uint32_t v1 = read_rm(rm, word);
            uint32_t v2 = word ? *get_reg16(rm.reg_idx) : *get_reg8(rm.reg_idx);
            write_rm(rm, word, v2);
            if (word) *get_reg16(rm.reg_idx) = static_cast<uint16_t>(v1);
            else *get_reg8(rm.reg_idx) = static_cast<uint8_t>(v1);
            break;
        }
        case 0x88: case 0x89: { // MOV r/m, reg
            bool word = op & 1;
            uint8_t modrm = fetch8();
            auto rm = decode_modrm(modrm, word, seg_override);
            uint32_t v = word ? *get_reg16(rm.reg_idx) : *get_reg8(rm.reg_idx);
            write_rm(rm, word, v);
            break;
        }
        case 0x8A: case 0x8B: { // MOV reg, r/m
            bool word = op & 1;
            uint8_t modrm = fetch8();
            auto rm = decode_modrm(modrm, word, seg_override);
            uint32_t v = read_rm(rm, word);
            if (word) *get_reg16(rm.reg_idx) = static_cast<uint16_t>(v);
            else *get_reg8(rm.reg_idx) = static_cast<uint8_t>(v);
            break;
        }
        case 0x8C: { // MOV r/m, seg
            uint8_t modrm = fetch8();
            auto rm = decode_modrm(modrm, true, seg_override);
            write_rm(rm, true, *get_seg_reg(rm.reg_idx & 3));
            break;
        }
        case 0x8D: { // LEA reg16, mem
            uint8_t modrm = fetch8();
            auto rm = decode_modrm(modrm, true, seg_override);
            if (!rm.is_mem) return ExecResult::ILLEGAL_OPCODE;
            *get_reg16(rm.reg_idx) = rm.offset;
            break;
        }
        case 0x8E: { // MOV seg, r/m
            uint8_t modrm = fetch8();
            auto rm = decode_modrm(modrm, true, seg_override);
            *get_seg_reg(rm.reg_idx & 3) = static_cast<uint16_t>(read_rm(rm, true));
            break;
        }
        case 0x8F: { // POP r/m
            uint8_t modrm = fetch8();
            auto rm = decode_modrm(modrm, true, seg_override);
            uint16_t val = cpu.mem.read16(cpu.ss_sp());
            cpu.regs.SP += 2;
            write_rm(rm, true, val);
            break;
        }
        case 0x90: break; // NOP
        case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97: { // XCHG AX, reg16
            uint16_t temp = cpu.regs.AX;
            cpu.regs.AX = *get_reg16(op & 7);
            *get_reg16(op & 7) = temp;
            break;
        }
        case 0x98: // CBW
            cpu.regs.AH() = (cpu.regs.AL() & 0x80) ? 0xFF : 0x00; break;
        case 0x99: // CWD
            cpu.regs.DX = (cpu.regs.AX & 0x8000) ? 0xFFFF : 0x0000; break;
        case 0x9A: { // CALL far
            uint16_t offset = fetch16(), seg = fetch16();
            cpu.regs.SP -= 2; cpu.mem.write16(cpu.ss_sp(), cpu.regs.CS);
            cpu.regs.SP -= 2; cpu.mem.write16(cpu.ss_sp(), cpu.regs.IP);
            cpu.regs.CS = seg; cpu.regs.IP = offset;
            break;
        }
        case 0x9C: cpu.regs.SP -= 2; cpu.mem.write16(cpu.ss_sp(), cpu.regs.flags.to_word()); break; // PUSHF
        case 0x9D: cpu.regs.flags.from_word(cpu.mem.read16(cpu.ss_sp())); cpu.regs.SP += 2; break;  // POPF
        case 0x9E: // SAHF
            cpu.regs.flags.from_word((cpu.regs.flags.to_word() & 0xFF00) | cpu.regs.AH());
            break;
        case 0x9F: // LAHF
            cpu.regs.AH() = cpu.regs.flags.to_word() & 0xFF;
            break;
        case 0xA0: case 0xA1: { // MOV AL/AX, moffs
            bool word = op & 1;
            uint32_t addr = Memory::segment_offset((seg_override != 0xFFFFFFFF) ? seg_override : cpu.regs.DS, fetch16());
            if (word) cpu.regs.AX = cpu.mem.read16(addr); else cpu.regs.AL() = cpu.mem.read8(addr);
            break;
        }
        case 0xA2: case 0xA3: { // MOV moffs, AL/AX
            bool word = op & 1;
            uint32_t addr = Memory::segment_offset((seg_override != 0xFFFFFFFF) ? seg_override : cpu.regs.DS, fetch16());
            if (word) cpu.mem.write16(addr, cpu.regs.AX); else cpu.mem.write8(addr, cpu.regs.AL());
            break;
        }
        // --- String Operations ---
        case 0xA4: case 0xA5: case 0xA6: case 0xA7: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAE: case 0xAF: {
            bool word = op & 1;
            int step = word ? 2 : 1;
            if (cpu.regs.flags.DF) step = -step;
            bool is_cmp = (op == 0xA6 || op == 0xA7 || op == 0xAE || op == 0xAF);
            
            // Single iteration if no REP, or loop if REP
            do {
                if (rep && cpu.regs.CX == 0) break;

                uint32_t si_addr = Memory::segment_offset((seg_override != 0xFFFFFFFF) ? seg_override : cpu.regs.DS, cpu.regs.SI);
                uint32_t di_addr = Memory::segment_offset(cpu.regs.ES, cpu.regs.DI);
                
                if (op == 0xA4 || op == 0xA5) { // MOVS
                    uint32_t val = word ? cpu.mem.read16(si_addr) : cpu.mem.read8(si_addr);
                    if (word) cpu.mem.write16(di_addr, static_cast<uint16_t>(val)); else cpu.mem.write8(di_addr, static_cast<uint8_t>(val));
                    cpu.regs.SI += step; cpu.regs.DI += step;
                } else if (op == 0xAA || op == 0xAB) { // STOS
                    if (word) cpu.mem.write16(di_addr, cpu.regs.AX); else cpu.mem.write8(di_addr, cpu.regs.AL());
                    cpu.regs.DI += step;
                } else if (op == 0xAC || op == 0xAD) { // LODS
                    if (word) cpu.regs.AX = cpu.mem.read16(si_addr); else cpu.regs.AL() = cpu.mem.read8(si_addr);
                    cpu.regs.SI += step;
                } else if (op == 0xA6 || op == 0xA7) { // CMPS
                    uint32_t src = word ? cpu.mem.read16(si_addr) : cpu.mem.read8(si_addr);
                    uint32_t dst = word ? cpu.mem.read16(di_addr) : cpu.mem.read8(di_addr);
                    uint32_t res; bool wb; alu_op(5, word, src, dst, res, wb); // Use SUB (5) for CMPS
                    cpu.regs.SI += step; cpu.regs.DI += step;
                } else if (op == 0xAE || op == 0xAF) { // SCAS
                    uint32_t dst = word ? cpu.mem.read16(di_addr) : cpu.mem.read8(di_addr);
                    uint32_t src = word ? cpu.regs.AX : cpu.regs.AL();
                    uint32_t res; bool wb; alu_op(5, word, src, dst, res, wb); // Use SUB (5) for SCAS
                    cpu.regs.DI += step;
                }

                if (rep) {
                    cpu.regs.CX--;
                    if (is_cmp && repz && !cpu.regs.flags.ZF) break;
                    if (is_cmp && repnz && cpu.regs.flags.ZF) break;
                }
            } while (rep && cpu.regs.CX != 0);
            break;
        }
        case 0xA8: case 0xA9: { // TEST AL/AX, imm
            bool word = op & 1;
            uint32_t v1 = word ? cpu.regs.AX : cpu.regs.AL();
            uint32_t v2 = word ? fetch16() : fetch8();
            cpu.update_flags_logical(static_cast<uint16_t>(v1 & v2), word);
            break;
        }
        case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7: // MOV reg8, imm8
            *get_reg8(op & 7) = fetch8(); break;
        case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBE: case 0xBF: // MOV reg16, imm16
            *get_reg16(op & 7) = fetch16(); break;
        case 0xC2: { uint16_t imm = fetch16(); cpu.regs.IP = cpu.mem.read16(cpu.ss_sp()); cpu.regs.SP += 2 + imm; break; } // RET imm16
        case 0xC3: { cpu.regs.IP = cpu.mem.read16(cpu.ss_sp()); cpu.regs.SP += 2; break; } // RET
        case 0xC4: case 0xC5: { // LES, LDS
            uint8_t modrm = fetch8(); auto rm = decode_modrm(modrm, true, seg_override);
            if (!rm.is_mem) return ExecResult::ILLEGAL_OPCODE;
            uint16_t off = cpu.mem.read16(rm.mem_addr), seg = cpu.mem.read16(rm.mem_addr + 2);
            *get_reg16(rm.reg_idx) = off; if (op == 0xC4) cpu.regs.ES = seg; else cpu.regs.DS = seg;
            break;
        }
        case 0xC6: case 0xC7: { // MOV r/m, imm
            bool word = op & 1; uint8_t modrm = fetch8(); auto rm = decode_modrm(modrm, word, seg_override);
            write_rm(rm, word, word ? fetch16() : fetch8());
            break;
        }
        case 0xCA: { uint16_t imm = fetch16(); cpu.regs.IP = cpu.mem.read16(cpu.ss_sp()); cpu.regs.SP += 2; cpu.regs.CS = cpu.mem.read16(cpu.ss_sp()); cpu.regs.SP += 2 + imm; break; } // RETF imm16
        case 0xCB: { cpu.regs.IP = cpu.mem.read16(cpu.ss_sp()); cpu.regs.SP += 2; cpu.regs.CS = cpu.mem.read16(cpu.ss_sp()); cpu.regs.SP += 2; break; } // RETF
        case 0xCC: last_interrupt_num = 3; return ExecResult::BREAKPOINT; // INT 3
        case 0xCD: last_interrupt_num = fetch8(); return ExecResult::INTERRUPT_PENDING; // INT imm
        case 0xCE: if (cpu.regs.flags.OF) { last_interrupt_num = 4; return ExecResult::INTERRUPT_PENDING; } break; // INTO
        case 0xCF: // IRET
            cpu.regs.IP = cpu.mem.read16(cpu.ss_sp()); cpu.regs.SP += 2;
            cpu.regs.CS = cpu.mem.read16(cpu.ss_sp()); cpu.regs.SP += 2;
            cpu.regs.flags.from_word(cpu.mem.read16(cpu.ss_sp())); cpu.regs.SP += 2;
            break;
        case 0xD0: case 0xD1: case 0xD2: case 0xD3: { // Shift/Rotate
            bool word = op & 1, use_cl = op & 2; uint8_t modrm = fetch8(); auto rm = decode_modrm(modrm, word, seg_override);
            uint32_t v = read_rm(rm, word); uint8_t count = (use_cl ? cpu.regs.CL() : 1);
            if (count == 0) break;
            uint8_t op_type = rm.reg_idx; uint32_t msb = word ? 0x8000 : 0x80, mask = word ? 0xFFFF : 0xFF;
            bool original_msb = (v & msb) != 0;
            for (int i=0; i<count; i++) {
                switch (op_type) {
                    case 0: cpu.regs.flags.CF = (v & msb) != 0; v = ((v << 1) | (cpu.regs.flags.CF ? 1 : 0)) & mask; break; // ROL
                    case 1: cpu.regs.flags.CF = (v & 1) != 0; v = ((v >> 1) | (cpu.regs.flags.CF ? msb : 0)) & mask; break; // ROR
                    case 2: { bool o_cf = cpu.regs.flags.CF; cpu.regs.flags.CF = (v & msb) != 0; v = ((v << 1) | (o_cf ? 1 : 0)) & mask; break; } // RCL
                    case 3: { bool o_cf = cpu.regs.flags.CF; cpu.regs.flags.CF = (v & 1) != 0; v = ((v >> 1) | (o_cf ? msb : 0)) & mask; break; } // RCR
                    case 4: case 6: cpu.regs.flags.CF = (v & msb) != 0; v = (v << 1) & mask; break; // SHL/SAL
                    case 5: cpu.regs.flags.CF = (v & 1) != 0; v = (v >> 1) & mask; break; // SHR
                    case 7: { bool sign = (v & msb) != 0; cpu.regs.flags.CF = (v & 1) != 0; v = ((v >> 1) | (sign ? msb : 0)) & mask; break; } // SAR
                }
            }
            if (op_type >= 4) {
                cpu.regs.flags.ZF = (v == 0);
                cpu.regs.flags.SF = (v & msb) != 0;
                cpu.regs.flags.PF = cpu.calc_parity(static_cast<uint8_t>(v & 0xFF));
            }
            if (count == 1) {
                if (op_type == 0 || op_type == 2 || op_type == 4 || op_type == 6) cpu.regs.flags.OF = cpu.regs.flags.CF ^ ((v & msb) != 0);
                else if (op_type == 1 || op_type == 3) cpu.regs.flags.OF = ((v & msb) != 0) ^ (((v << 1) & msb) != 0);
                else if (op_type == 5) cpu.regs.flags.OF = original_msb;
                else if (op_type == 7) cpu.regs.flags.OF = false;
            }
            write_rm(rm, word, v);
            break;
        }
        case 0xD4: { uint8_t base = fetch8(); if (base == 0) return ExecResult::DIVISION_BY_ZERO; cpu.regs.AH() = cpu.regs.AL() / base; cpu.regs.AL() %= base; cpu.update_flags_logical(cpu.regs.AL(), false); break; } // AAM
        case 0xD5: { uint8_t base = fetch8(); cpu.regs.AL() = cpu.regs.AL() + (cpu.regs.AH() * base); cpu.regs.AH() = 0; cpu.update_flags_logical(cpu.regs.AL(), false); break; } // AAD
        case 0xD7: // XLAT
            cpu.regs.AL() = cpu.mem.read8(Memory::segment_offset((seg_override != 0xFFFFFFFF) ? seg_override : cpu.regs.DS, cpu.regs.BX + cpu.regs.AL()));
            break;
        case 0x27: { // DAA
            uint8_t old_al = cpu.regs.AL(); bool old_cf = cpu.regs.flags.CF;
            if ((cpu.regs.AL() & 0x0F) > 9 || cpu.regs.flags.AF) { cpu.regs.AL() += 6; cpu.regs.flags.AF = true; } else cpu.regs.flags.AF = false;
            if (old_al > 0x99 || old_cf) { cpu.regs.AL() += 0x60; cpu.regs.flags.CF = true; } else cpu.regs.flags.CF = false;
            uint8_t res = cpu.regs.AL();
            cpu.regs.flags.ZF = (res == 0);
            cpu.regs.flags.SF = (res & 0x80) != 0;
            cpu.regs.flags.PF = cpu.calc_parity(res);
            break;
        }
        case 0x2F: { // DAS
            uint8_t old_al = cpu.regs.AL(); bool old_cf = cpu.regs.flags.CF;
            if ((cpu.regs.AL() & 0x0F) > 9 || cpu.regs.flags.AF) { cpu.regs.AL() -= 6; cpu.regs.flags.AF = true; } else cpu.regs.flags.AF = false;
            if (old_al > 0x99 || old_cf) { cpu.regs.AL() -= 0x60; cpu.regs.flags.CF = true; } else cpu.regs.flags.CF = false;
            uint8_t res = cpu.regs.AL();
            cpu.regs.flags.ZF = (res == 0);
            cpu.regs.flags.SF = (res & 0x80) != 0;
            cpu.regs.flags.PF = cpu.calc_parity(res);
            break;
        }
        case 0x37: { // AAA
            if ((cpu.regs.AL() & 0x0F) > 9 || cpu.regs.flags.AF) { 
                cpu.regs.AL() += 6; 
                cpu.regs.AH() += 1; 
                cpu.regs.flags.AF = cpu.regs.flags.CF = true; 
            } else {
                cpu.regs.flags.AF = cpu.regs.flags.CF = false;
            }
            cpu.regs.AL() &= 0x0F; 
            break;
        }
        case 0x3F: { // AAS
            if ((cpu.regs.AL() & 0x0F) > 9 || cpu.regs.flags.AF) { 
                cpu.regs.AL() -= 6; 
                cpu.regs.AH() -= 1; 
                cpu.regs.flags.AF = cpu.regs.flags.CF = true; 
            } else {
                cpu.regs.flags.AF = cpu.regs.flags.CF = false;
            }
            cpu.regs.AL() &= 0x0F; 
            break;
        }
        case 0xE0: { int8_t offset = fetch_s8(); if (--cpu.regs.CX != 0 && !cpu.regs.flags.ZF) cpu.regs.IP += offset; break; } // LOOPNE
        case 0xE1: { int8_t offset = fetch_s8(); if (--cpu.regs.CX != 0 && cpu.regs.flags.ZF) cpu.regs.IP += offset; break; }  // LOOPE
        case 0xE2: { int8_t offset = fetch_s8(); if (--cpu.regs.CX != 0) cpu.regs.IP += offset; break; }                         // LOOP
        case 0xE3: { int8_t offset = fetch_s8(); if (cpu.regs.CX == 0) cpu.regs.IP += offset; break; }                           // JCXZ
        case 0xE4: fetch8(); cpu.regs.AL() = 0xFF; break; // IN AL, imm8
        case 0xE5: fetch8(); cpu.regs.AX = 0xFFFF; break; // IN AX, imm8
        case 0xE6: case 0xE7: fetch8(); break;            // OUT imm8
        case 0xEC: cpu.regs.AL() = 0xFF; break;           // IN AL, DX
        case 0xED: cpu.regs.AX = 0xFFFF; break;           // IN AX, DX
        case 0xEE: case 0xEF: break;                      // OUT DX
        case 0xE8: { int16_t offset = fetch_s16(); cpu.regs.SP -= 2; cpu.mem.write16(cpu.ss_sp(), cpu.regs.IP); cpu.regs.IP += offset; break; } // CALL rel16
        case 0xE9: { int16_t offset = fetch_s16(); cpu.regs.IP += offset; break; } // JMP rel16
        case 0xEA: { uint16_t offset = fetch16(), seg = fetch16(); cpu.regs.CS = seg; cpu.regs.IP = offset; break; } // JMP far
        case 0xEB: { int8_t offset = fetch_s8(); cpu.regs.IP += offset; break; } // JMP rel8
        case 0xF4: return ExecResult::HALT;
        case 0xF5: cpu.regs.flags.CF = !cpu.regs.flags.CF; break; // CMC
        case 0xF6: case 0xF7: { // Group 3: TEST, NOT, NEG, MUL, IMUL, DIV, IDIV
            bool word = op & 1; uint8_t modrm = fetch8(); auto rm = decode_modrm(modrm, word, seg_override);
            uint32_t v = read_rm(rm, word);
            switch (rm.reg_idx) {
                case 0: case 1: cpu.update_flags_logical(static_cast<uint16_t>(v & (word ? fetch16() : fetch8())), word); break; // TEST imm
                case 2: write_rm(rm, word, ~v); break; // NOT
                case 3: { uint32_t res = 0 - v; cpu.update_flags_sub(0, v, res, word); cpu.regs.flags.CF = (v != 0); write_rm(rm, word, res); break; } // NEG
                case 4: { // MUL
                    if (word) { uint32_t res = static_cast<uint32_t>(cpu.regs.AX) * v; cpu.regs.AX = res & 0xFFFF; cpu.regs.DX = res >> 16; cpu.regs.flags.CF = cpu.regs.flags.OF = (cpu.regs.DX != 0); }
                    else { uint16_t res = static_cast<uint16_t>(cpu.regs.AL()) * v; cpu.regs.AX = res; cpu.regs.flags.CF = cpu.regs.flags.OF = (cpu.regs.AH() != 0); } break;
                }
                case 5: { // IMUL
                    if (word) { int32_t res = static_cast<int32_t>(static_cast<int16_t>(cpu.regs.AX)) * static_cast<int32_t>(static_cast<int16_t>(v)); cpu.regs.AX = res & 0xFFFF; cpu.regs.DX = res >> 16; cpu.regs.flags.CF = cpu.regs.flags.OF = (res != static_cast<int32_t>(static_cast<int16_t>(res & 0xFFFF))); }
                    else { int16_t res = static_cast<int16_t>(static_cast<int8_t>(cpu.regs.AL())) * static_cast<int16_t>(static_cast<int8_t>(v)); cpu.regs.AX = res; cpu.regs.flags.CF = cpu.regs.flags.OF = (res != static_cast<int16_t>(static_cast<int8_t>(res & 0xFF))); } break;
                }
                case 6: { // DIV
                    if (v == 0) return ExecResult::DIVISION_BY_ZERO;
                    if (word) { uint32_t num = (static_cast<uint32_t>(cpu.regs.DX) << 16) | cpu.regs.AX; if (num / v > 0xFFFF) return ExecResult::DIVISION_BY_ZERO; cpu.regs.AX = num / v; cpu.regs.DX = num % v; }
                    else { uint16_t num = cpu.regs.AX; if (num / v > 0xFF) return ExecResult::DIVISION_BY_ZERO; cpu.regs.AL() = num / v; cpu.regs.AH() = num % v; } break;
                }
                case 7: { // IDIV
                    if (v == 0) return ExecResult::DIVISION_BY_ZERO;
                if (word) { 
                    int64_t num = static_cast<int32_t>((static_cast<uint32_t>(cpu.regs.DX) << 16) | cpu.regs.AX); 
                    int64_t den = static_cast<int16_t>(v); 
                    int64_t res = num / den; 
                    if (res > 32767 || res < -32768) return ExecResult::DIVISION_BY_ZERO; 
                    cpu.regs.AX = static_cast<uint16_t>(res); cpu.regs.DX = static_cast<uint16_t>(num % den); 
                } else { 
                    int32_t num = static_cast<int16_t>(cpu.regs.AX); 
                    int32_t den = static_cast<int8_t>(v); 
                    int32_t res = num / den; 
                    if (res > 127 || res < -128) return ExecResult::DIVISION_BY_ZERO; 
                    cpu.regs.AL() = static_cast<uint8_t>(res); cpu.regs.AH() = static_cast<uint8_t>(num % den); 
                } 
                break;
                }
            }
            break;
        }
        case 0xF8: cpu.regs.flags.CF = false; break; // CLC
        case 0xF9: cpu.regs.flags.CF = true; break;  // STC
        case 0xFA: cpu.regs.flags.IF = false; break; // CLI
        case 0xFB: cpu.regs.flags.IF = true; break;  // STI
        case 0xFC: cpu.regs.flags.DF = false; break; // CLD
        case 0xFD: cpu.regs.flags.DF = true; break;  // STD
        case 0xFE: case 0xFF: { // Group 4 & 5
            bool word = op & 1; uint8_t modrm = fetch8(); auto rm = decode_modrm(modrm, word, seg_override);
            switch (rm.reg_idx) {
                case 0: { uint32_t v = read_rm(rm, word); bool o_cf = cpu.regs.flags.CF; uint32_t res = v + 1; cpu.update_flags_add(v, 1, res, word); cpu.regs.flags.CF = o_cf; write_rm(rm, word, res); break; } // INC
                case 1: { uint32_t v = read_rm(rm, word); bool o_cf = cpu.regs.flags.CF; uint32_t res = v - 1; cpu.update_flags_sub(v, 1, res, word); cpu.regs.flags.CF = o_cf; write_rm(rm, word, res); break; } // DEC
                case 2: if (word) { uint32_t v = read_rm(rm, word); cpu.regs.SP -= 2; cpu.mem.write16(cpu.ss_sp(), cpu.regs.IP); cpu.regs.IP = static_cast<uint16_t>(v); } break; // CALL near
                case 3: if (word && rm.is_mem) { uint32_t v = read_rm(rm, word); uint16_t seg = cpu.mem.read16(rm.mem_addr + 2); cpu.regs.SP -= 2; cpu.mem.write16(cpu.ss_sp(), cpu.regs.CS); cpu.regs.SP -= 2; cpu.mem.write16(cpu.ss_sp(), cpu.regs.IP); cpu.regs.CS = seg; cpu.regs.IP = static_cast<uint16_t>(v); } break; // CALL far
                case 4: if (word) { uint32_t v = read_rm(rm, word); cpu.regs.IP = static_cast<uint16_t>(v); } break; // JMP near
                case 5: if (word && rm.is_mem) { uint32_t v = read_rm(rm, word); cpu.regs.CS = cpu.mem.read16(rm.mem_addr + 2); cpu.regs.IP = static_cast<uint16_t>(v); } break; // JMP far
                case 6: if (word) { 
                    cpu.regs.SP -= 2; 
                    uint16_t val = (rm.is_mem) ? static_cast<uint16_t>(read_rm(rm, true)) : *rm.reg_ptr; 
                    cpu.mem.write16(cpu.ss_sp(), val); 
                } break; // PUSH
            }
            break;
        }
        default: return ExecResult::ILLEGAL_OPCODE;
    }

    return ExecResult::OK;
}

} // namespace emu8086::core
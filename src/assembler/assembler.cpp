#include "assembler.h"
#include <sstream>
#include <cctype>
#include <algorithm>

namespace emu8086::assembler {

struct ParsedLine {
    int line_num;
    std::string label;
    std::string mnemonic;
    std::vector<std::string> operands;
};

struct Operand {
    enum Type { NONE, REG8, REG16, SEG, MEM, IMM } type{NONE};
    int val{0};
    int mod{0}, rm{0}, disp{0};
    bool word{true};
};

// --- Lookup Tables ---
static const std::map<std::string, int> alu_ops = {
    {"ADD", 0}, {"OR", 1}, {"ADC", 2}, {"SBB", 3}, {"AND", 4}, {"SUB", 5}, {"XOR", 6}, {"CMP", 7}
};

static const std::map<std::string, uint8_t> jcc_ops = {
    {"JO", 0x70}, {"JNO", 0x71}, {"JB", 0x72}, {"JC", 0x72}, {"JNAE", 0x72},
    {"JAE", 0x73}, {"JNB", 0x73}, {"JNC", 0x73}, {"JE", 0x74}, {"JZ", 0x74},
    {"JNE", 0x75}, {"JNZ", 0x75}, {"JBE", 0x76}, {"JNA", 0x76}, {"JA", 0x77}, {"JNBE", 0x77},
    {"JS", 0x78}, {"JNS", 0x79}, {"JP", 0x7A}, {"JPE", 0x7A}, {"JNP", 0x7B}, {"JPO", 0x7B},
    {"JL", 0x7C}, {"JNGE", 0x7C}, {"JGE", 0x7D}, {"JNL", 0x7D}, {"JLE", 0x7E}, {"JNG", 0x7E},
    {"JG", 0x7F}, {"JNLE", 0x7F}, {"JCXZ", 0xE3}, {"LOOP", 0xE2}, {"LOOPE", 0xE1}, {"LOOPNE", 0xE0}
};

static const std::map<std::string, uint8_t> one_byte_ops = {
    {"NOP", 0x90}, {"HLT", 0xF4}, {"CLC", 0xF8}, {"STC", 0xF9}, {"CMC", 0xF5},
    {"CLD", 0xFC}, {"STD", 0xFD}, {"CLI", 0xFA}, {"STI", 0xFB},
    {"CBW", 0x98}, {"CWD", 0x99}, {"PUSHF", 0x9C}, {"POPF", 0x9D},
    {"IRET", 0xCF}, {"DAA", 0x27}, {"DAS", 0x2F}, {"AAA", 0x37}, {"AAS", 0x3F},
    {"MOVSB", 0xA4}, {"MOVSW", 0xA5}, {"STOSB", 0xAA}, {"STOSW", 0xAB},
    {"LODSB", 0xAC}, {"LODSW", 0xAD}, {"CMPSB", 0xA6}, {"CMPSW", 0xA7},
    {"SCASB", 0xAE}, {"SCASW", 0xAF},
    {"REP", 0xF3}, {"REPE", 0xF3}, {"REPZ", 0xF3}, {"REPNE", 0xF2}, {"REPNZ", 0xF2}
};

static const std::map<std::string, int> shift_ops = {
    {"ROL", 0}, {"ROR", 1}, {"RCL", 2}, {"RCR", 3},
    {"SHL", 4}, {"SAL", 4}, {"SHR", 5}, {"SAR", 7}
};

static const std::map<std::string, int> grp3_ops = {
    {"NOT", 2}, {"NEG", 3}, {"MUL", 4}, {"IMUL", 5}, {"DIV", 6}, {"IDIV", 7}
};

// --- Helpers ---
std::string to_upper(std::string s) {
    for (char& c : s) c = std::toupper(c);
    return s;
}

int get_reg8(const std::string& r) {
    if (r=="AL") return 0; if (r=="CL") return 1; if (r=="DL") return 2; if (r=="BL") return 3;
    if (r=="AH") return 4; if (r=="CH") return 5; if (r=="DH") return 6; if (r=="BH") return 7;
    return -1;
}

int get_reg16(const std::string& r) {
    if (r=="AX") return 0; if (r=="CX") return 1; if (r=="DX") return 2; if (r=="BX") return 3;
    if (r=="SP") return 4; if (r=="BP") return 5; if (r=="SI") return 6; if (r=="DI") return 7;
    return -1;
}

void emit8(std::vector<uint8_t>& code, uint8_t v) { code.push_back(v); }
void emit16(std::vector<uint8_t>& code, uint16_t v) { code.push_back(v & 0xFF); code.push_back(v >> 8); }
void emit_modrm(std::vector<uint8_t>& code, int mod, int reg, int rm, int disp) {
    code.push_back((mod << 6) | (reg << 3) | rm);
    if (mod == 1) emit8(code, disp);
    else if (mod == 2 || (mod == 0 && rm == 6)) emit16(code, disp);
}

// --- Parsing & Evaluation ---
int evaluate_expression(std::string e, const std::map<std::string, uint16_t>& symbols, bool& ok) {
    if (e.empty()) return 0;
    size_t plus = e.find_last_of('+');
    size_t minus = e.find_last_of('-');
    if (plus != std::string::npos && (minus == std::string::npos || plus > minus)) {
        return evaluate_expression(e.substr(0, plus), symbols, ok) + evaluate_expression(e.substr(plus+1), symbols, ok);
    }
    if (minus != std::string::npos && (plus == std::string::npos || minus > plus)) {
        return evaluate_expression(e.substr(0, minus), symbols, ok) - evaluate_expression(e.substr(minus+1), symbols, ok);
    }
    if (symbols.count(e)) return symbols.at(e);
    try {
        if (e.length() > 2 && e[0] == '0' && (e[1] == 'X' || e[1] == 'x')) return std::stoi(e, nullptr, 16);
        if (e.length() > 2 && e[0] == '0' && (e[1] == 'B' || e[1] == 'b')) return std::stoi(e.substr(2), nullptr, 2);
        if (e.length() > 2 && e[0] == '0' && (e[1] == 'O' || e[1] == 'o')) return std::stoi(e.substr(2), nullptr, 8);
        if (e.back() == 'H') return std::stoi(e.substr(0, e.length()-1), nullptr, 16);
        if (e.back() == 'B') return std::stoi(e.substr(0, e.length()-1), nullptr, 2);
        if (std::isdigit(e[0]) || e[0] == '-') return std::stoi(e, nullptr, 0);
    } catch (...) {}
    ok = false;
    return 0;
}

void parse_memory(std::string e, int& mod, int& rm, int& disp, const std::map<std::string, uint16_t>& symbols, bool& ok) {
    e = e.substr(1, e.length()-2); // strip []
    if (e.find("BX") == std::string::npos && e.find("BP") == std::string::npos &&
        e.find("SI") == std::string::npos && e.find("DI") == std::string::npos) {
        mod = 0; rm = 6; disp = evaluate_expression(e, symbols, ok); return;
    }
    
    if (e.find("BX") != std::string::npos && e.find("SI") != std::string::npos) rm = 0;
    else if (e.find("BX") != std::string::npos && e.find("DI") != std::string::npos) rm = 1;
    else if (e.find("BP") != std::string::npos && e.find("SI") != std::string::npos) rm = 2;
    else if (e.find("BP") != std::string::npos && e.find("DI") != std::string::npos) rm = 3;
    else if (e.find("SI") != std::string::npos) rm = 4;
    else if (e.find("DI") != std::string::npos) rm = 5;
    else if (e.find("BP") != std::string::npos) rm = 6;
    else if (e.find("BX") != std::string::npos) rm = 7;
    
    std::string d_expr = e;
    size_t p;
    for (auto r : {"BX", "BP", "SI", "DI"}) {
        while ((p = d_expr.find(r)) != std::string::npos) d_expr.replace(p, 2, "0");
    }
    disp = evaluate_expression(d_expr, symbols, ok);
    
    if (disp == 0 && rm != 6) mod = 0;
    else if (disp >= -128 && disp <= 127) mod = 1;
    else mod = 2;
}

Operand parse_operand(std::string s, const std::map<std::string, uint16_t>& symbols, bool& ok) {
    Operand op;
    s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
    if (s.find("BYTEPTR") == 0) { op.word = false; s = s.substr(7); }
    if (s.find("WORDPTR") == 0) { op.word = true; s = s.substr(7); }

    int reg;
    if ((reg = get_reg8(s)) != -1) { op.type = Operand::REG8; op.val = reg; return op; }
    if ((reg = get_reg16(s)) != -1) { op.type = Operand::REG16; op.val = reg; return op; }
    if (s == "CS") { op.type = Operand::SEG; op.val = 1; return op; }
    if (s == "DS") { op.type = Operand::SEG; op.val = 3; return op; }
    if (s == "ES") { op.type = Operand::SEG; op.val = 0; return op; }
    if (s == "SS") { op.type = Operand::SEG; op.val = 2; return op; }

    if (!s.empty() && s[0] == '[') {
        op.type = Operand::MEM;
        parse_memory(s, op.mod, op.rm, op.disp, symbols, ok);
        return op;
    }

    op.type = Operand::IMM;
    op.val = evaluate_expression(s, symbols, ok);
    op.word = (op.val < -128 || op.val > 255);
    return op;
}

std::vector<ParsedLine> parse_source(const std::string& source) {
    std::vector<ParsedLine> result;
    std::istringstream stream(source);
    std::string line;
    int line_num = 0;
    while (std::getline(stream, line)) {
        line_num++;
        size_t comment_pos = line.find(';');
        if (comment_pos != std::string::npos) line = line.substr(0, comment_pos);
        
        std::vector<std::string> tokens;
        bool in_quotes = false;
        std::string token;
        for (char c : line) {
            if (c == '"' || c == '\'') in_quotes = !in_quotes;
            if (!in_quotes && (c == ' ' || c == '\t' || c == '\r' || c == ',')) {
                if (!token.empty()) { tokens.push_back(token); token.clear(); }
                if (c == ',') tokens.push_back(",");
            } else token += c;
        }
        if (!token.empty()) tokens.push_back(token);
        
        for (auto& t : tokens) {
            if (t.front() != '"' && t.front() != '\'') t = to_upper(t);
        }
        
        if (tokens.empty()) continue;
        
        ParsedLine pl;
        pl.line_num = line_num;
        size_t idx = 0;
        
        if (tokens[0].back() == ':') {
            pl.label = tokens[0].substr(0, tokens[0].length() - 1);
            idx++;
        } else if (tokens.size() > 1 && tokens[1] == ":") {
            pl.label = tokens[0];
            idx += 2;
        }
        
        if (idx < tokens.size()) pl.mnemonic = tokens[idx++];
        
        std::string current_op;
        for (; idx < tokens.size(); idx++) {
            if (tokens[idx] == ",") {
                if (!current_op.empty()) { pl.operands.push_back(current_op); current_op.clear(); }
            } else current_op += tokens[idx];
        }
        if (!current_op.empty()) pl.operands.push_back(current_op);
        
        result.push_back(pl);
    }
    return result;
}

// --- Main Instruction Encoder ---
void encode_instruction(const std::string& mnemonic, const std::vector<std::string>& ops, const std::map<std::string, uint16_t>& symbols, std::vector<uint8_t>& code, uint16_t LC, AssemblyResult& res, int line_num, bool is_pass2) {
    if (mnemonic == "DB" || mnemonic == "DW") {
        for (auto& op : ops) {
            if (op.front() == '"' || op.front() == '\'') {
                for (size_t i = 1; i < op.length() - 1; i++) {
                    emit8(code, op[i]);
                    if (mnemonic == "DW") emit8(code, 0);
                }
            } else {
                bool ok = true;
                int val = evaluate_expression(op, symbols, ok);
                if (is_pass2 && !ok) { res.success = false; res.errors.push_back({line_num, "Unresolved symbol: " + op}); }
                emit8(code, val & 0xFF);
                if (mnemonic == "DW") emit8(code, (val >> 8) & 0xFF);
            }
        }
        return;
    }
    if (one_byte_ops.count(mnemonic)) { emit8(code, one_byte_ops.at(mnemonic)); return; }
    
    bool ok = true;
    Operand op1 = ops.size() > 0 ? parse_operand(ops[0], symbols, ok) : Operand();
    Operand op2 = ops.size() > 1 ? parse_operand(ops[1], symbols, ok) : Operand();
    if (is_pass2 && !ok) { res.success = false; res.errors.push_back({line_num, "Invalid operand in " + mnemonic}); }

    if (alu_ops.count(mnemonic)) {
        int opc = alu_ops.at(mnemonic);
        bool word = (op1.type == Operand::REG16 || op2.type == Operand::REG16 || (op1.type == Operand::MEM && op1.word));
        if (op1.type == Operand::REG8 && op2.type == Operand::REG8) { emit8(code, (opc << 3) | 0x00); emit_modrm(code, 3, op1.val, op2.val, 0); }
        else if (op1.type == Operand::REG16 && op2.type == Operand::REG16) { emit8(code, (opc << 3) | 0x01); emit_modrm(code, 3, op1.val, op2.val, 0); }
        else if (op1.type == Operand::MEM && op2.type == Operand::REG8) { emit8(code, (opc << 3) | 0x00); emit_modrm(code, op1.mod, op2.val, op1.rm, op1.disp); }
        else if (op1.type == Operand::MEM && op2.type == Operand::REG16) { emit8(code, (opc << 3) | 0x01); emit_modrm(code, op1.mod, op2.val, op1.rm, op1.disp); }
        else if (op1.type == Operand::REG8 && op2.type == Operand::MEM) { emit8(code, (opc << 3) | 0x02); emit_modrm(code, op2.mod, op1.val, op2.rm, op2.disp); }
        else if (op1.type == Operand::REG16 && op2.type == Operand::MEM) { emit8(code, (opc << 3) | 0x03); emit_modrm(code, op2.mod, op1.val, op2.rm, op2.disp); }
        else if (op1.type == Operand::REG8 && op1.val == 0 && op2.type == Operand::IMM) { emit8(code, (opc << 3) | 0x04); emit8(code, op2.val); }
        else if (op1.type == Operand::REG16 && op1.val == 0 && op2.type == Operand::IMM) { emit8(code, (opc << 3) | 0x05); emit16(code, op2.val); }
        else if ((op1.type == Operand::REG8 || op1.type == Operand::MEM) && op2.type == Operand::IMM) {
            if (op1.type == Operand::REG8 || !word) {
                emit8(code, 0x80);
                if (op1.type == Operand::REG8) emit_modrm(code, 3, opc, op1.val, 0); else emit_modrm(code, op1.mod, opc, op1.rm, op1.disp);
                emit8(code, op2.val);
            } else {
                if (op2.val >= -128 && op2.val <= 127) {
                    emit8(code, 0x83);
                    if (op1.type == Operand::REG16) emit_modrm(code, 3, opc, op1.val, 0); else emit_modrm(code, op1.mod, opc, op1.rm, op1.disp);
                    emit8(code, op2.val);
                } else {
                    emit8(code, 0x81);
                    if (op1.type == Operand::REG16) emit_modrm(code, 3, opc, op1.val, 0); else emit_modrm(code, op1.mod, opc, op1.rm, op1.disp);
                    emit16(code, op2.val);
                }
            }
        }
        return;
    }
    
    if (mnemonic == "MOV") {
        bool word = (op1.type == Operand::REG16 || op2.type == Operand::REG16 || op1.word);
        if (op1.type == Operand::REG8 && op2.type == Operand::REG8) { emit8(code, 0x8A); emit_modrm(code, 3, op1.val, op2.val, 0); }
        else if (op1.type == Operand::REG16 && op2.type == Operand::REG16) { emit8(code, 0x8B); emit_modrm(code, 3, op1.val, op2.val, 0); }
        else if (op1.type == Operand::REG8 && op2.type == Operand::MEM) { emit8(code, 0x8A); emit_modrm(code, op2.mod, op1.val, op2.rm, op2.disp); }
        else if (op1.type == Operand::REG16 && op2.type == Operand::MEM) { emit8(code, 0x8B); emit_modrm(code, op2.mod, op1.val, op2.rm, op2.disp); }
        else if (op1.type == Operand::MEM && op2.type == Operand::REG8) { emit8(code, 0x88); emit_modrm(code, op1.mod, op2.val, op1.rm, op1.disp); }
        else if (op1.type == Operand::MEM && op2.type == Operand::REG16) { emit8(code, 0x89); emit_modrm(code, op1.mod, op2.val, op1.rm, op1.disp); }
        else if (op1.type == Operand::REG8 && op2.type == Operand::IMM) { emit8(code, 0xB0 | op1.val); emit8(code, op2.val); }
        else if (op1.type == Operand::REG16 && op2.type == Operand::IMM) { emit8(code, 0xB8 | op1.val); emit16(code, op2.val); }
        else if (op1.type == Operand::MEM && op2.type == Operand::IMM) {
            emit8(code, 0xC6 | (word ? 1 : 0));
            emit_modrm(code, op1.mod, 0, op1.rm, op1.disp);
            if (word) emit16(code, op2.val); else emit8(code, op2.val);
        } else if (op1.type == Operand::SEG && (op2.type == Operand::REG16 || op2.type == Operand::MEM)) {
            emit8(code, 0x8E);
            if (op2.type == Operand::REG16) emit_modrm(code, 3, op1.val, op2.val, 0); else emit_modrm(code, op2.mod, op1.val, op2.rm, op2.disp);
        } else if ((op1.type == Operand::REG16 || op1.type == Operand::MEM) && op2.type == Operand::SEG) {
            emit8(code, 0x8C);
            if (op1.type == Operand::REG16) emit_modrm(code, 3, op2.val, op1.val, 0); else emit_modrm(code, op1.mod, op2.val, op1.rm, op1.disp);
        }
        return;
    }

    if (mnemonic == "INC" || mnemonic == "DEC") {
        int opc = (mnemonic == "INC") ? 0 : 1;
        if (op1.type == Operand::REG16) emit8(code, 0x40 | (opc << 3) | op1.val);
        else {
            bool word = (op1.type == Operand::REG16 || op1.word);
            emit8(code, 0xFE | (word ? 1 : 0));
            if (op1.type == Operand::REG8) emit_modrm(code, 3, opc, op1.val, 0); else emit_modrm(code, op1.mod, opc, op1.rm, op1.disp);
        }
        return;
    }

    if (mnemonic == "PUSH" || mnemonic == "POP") {
        int opc = (mnemonic == "PUSH") ? 6 : 0;
        if (op1.type == Operand::REG16) emit8(code, (mnemonic == "PUSH" ? 0x50 : 0x58) | op1.val);
        else if (op1.type == Operand::SEG) emit8(code, (mnemonic == "PUSH" ? 0x06 : 0x07) | (op1.val << 3));
        else if (op1.type == Operand::IMM && mnemonic == "PUSH") {
            if (op1.val >= -128 && op1.val <= 127) { emit8(code, 0x6A); emit8(code, op1.val); } else { emit8(code, 0x68); emit16(code, op1.val); }
        } else if (op1.type == Operand::MEM) {
            emit8(code, mnemonic == "PUSH" ? 0xFF : 0x8F);
            emit_modrm(code, op1.mod, opc, op1.rm, op1.disp);
        }
        return;
    }
    
    if (jcc_ops.count(mnemonic)) {
        int rel = op1.val - (LC + 2);
        if (is_pass2 && (rel < -128 || rel > 127) && ok) { res.success = false; res.errors.push_back({line_num, "Jump target out of range"}); }
        emit8(code, jcc_ops.at(mnemonic)); emit8(code, rel);
        return;
    }

    if (mnemonic == "JMP" || mnemonic == "CALL") {
        if (op1.type == Operand::IMM) {
            int rel = op1.val - (LC + (mnemonic == "JMP" && op1.val - (LC+2) >= -128 && op1.val - (LC+2) <= 127 ? 2 : 3));
            if (mnemonic == "JMP" && rel >= -128 && rel <= 127) { emit8(code, 0xEB); emit8(code, rel); }
            else { emit8(code, mnemonic == "JMP" ? 0xE9 : 0xE8); emit16(code, rel); }
        } else {
            emit8(code, 0xFF);
            int opc = mnemonic == "JMP" ? 4 : 2;
            if (op1.type == Operand::REG16) emit_modrm(code, 3, opc, op1.val, 0); else emit_modrm(code, op1.mod, opc, op1.rm, op1.disp);
        }
        return;
    }

    if (mnemonic == "RET") { if (ops.empty()) emit8(code, 0xC3); else { emit8(code, 0xC2); emit16(code, op1.val); } return; }
    if (mnemonic == "RETF") { if (ops.empty()) emit8(code, 0xCB); else { emit8(code, 0xCA); emit16(code, op1.val); } return; }
    if (mnemonic == "INT") { if (op1.val == 3) emit8(code, 0xCC); else { emit8(code, 0xCD); emit8(code, op1.val); } return; }
    
    if (is_pass2) { res.success = false; res.errors.push_back({line_num, "Unhandled or unknown mnemonic: " + mnemonic}); }
}

// --- Core Assemble Function ---
AssemblyResult Assembler::assemble(const std::string& source, uint16_t origin) {
    AssemblyResult res;
    auto lines = parse_source(source);

    // 3 passes: allows resolving forward jumps shrinking from near (3b) to short (2b) cleanly.
    int passes = 3; 
    for (int p = 0; p < passes; p++) {
        uint16_t LC = origin;
        bool is_pass2 = (p == passes - 1);
        if (is_pass2) {
            res.machine_code.clear(); res.errors.clear(); res.line_to_offset.clear(); res.offset_to_line.clear();
        }

        for (auto& line : lines) {
            if (!line.label.empty() && !is_pass2) res.symbols[line.label] = LC;
            if (line.mnemonic.empty()) continue;
            
            if (line.mnemonic == "ORG") {
                bool ok = true;
                uint16_t new_LC = evaluate_expression(line.operands[0], res.symbols, ok);
                if (is_pass2 && new_LC > LC) res.machine_code.resize(res.machine_code.size() + (new_LC - LC), 0);
                LC = new_LC;
                continue;
            } else if (line.mnemonic == "EQU") {
                if (!is_pass2) {
                    bool ok = true;
                    res.symbols[line.label] = evaluate_expression(line.operands[0], res.symbols, ok);
                }
                continue;
            }

            if (is_pass2) {
                res.line_to_offset[line.line_num] = LC;
                res.offset_to_line[LC] = line.line_num;
            }

            std::vector<uint8_t> code;
            encode_instruction(line.mnemonic, line.operands, res.symbols, code, LC, res, line.line_num, is_pass2);
            
            if (is_pass2) res.machine_code.insert(res.machine_code.end(), code.begin(), code.end());
            LC += code.size();
        }
    }
    return res;
}

} // namespace emu8086::assembler
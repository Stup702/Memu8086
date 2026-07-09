#include "test_runner.h"
#include "core/emulator.h"
#include "assembler/assembler.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <iostream>
#include <fmt/core.h>

namespace emu_test {

TestRunner::TestRunner() {}

std::vector<Expectation> TestRunner::parse_expectations(const std::string& source) {
    std::vector<Expectation> expectations;
    std::istringstream stream(source);
    std::string line;
    
    // Regex for: ; EXPECT AX=1234, ; EXPECT CF=1, ; EXPECT MEM[1234]=AB
    std::regex reg_expect(R"(;\s*EXPECT\s+([A-Z]+)\s*=\s*([0-9A-Fa-f]+h?))");
    std::regex mem_expect(R"(;\s*EXPECT\s+MEM\[([0-9A-Fa-f]+h?)\]\s*=\s*([0-9A-Fa-f]+h?))");
    std::regex err_expect(R"(;\s*EXPECT\s+ASM_ERROR)");

    auto parse_hex = [](std::string val) -> uint32_t {
        if (val.empty()) return 0;
        if (val.back() == 'h' || val.back() == 'H') {
            val.pop_back();
            return std::stoul(val, nullptr, 16);
        }
        return std::stoul(val, nullptr, 10);
    };

    while (std::getline(stream, line)) {
        std::smatch match;
        if (std::regex_search(line, match, reg_expect)) {
            std::string name = match[1];
            uint32_t val = parse_hex(match[2]);
            
            static const std::vector<std::string> flags = {"CF", "PF", "AF", "ZF", "SF", "TF", "IF", "DF", "OF"};
            bool is_flag = std::find(flags.begin(), flags.end(), name) != flags.end();
            
            expectations.push_back({
                is_flag ? Expectation::Type::FLAG : Expectation::Type::REGISTER,
                name,
                0,
                static_cast<uint16_t>(val)
            });
        } else if (std::regex_search(line, match, mem_expect)) {
            uint32_t addr = parse_hex(match[1]);
            uint32_t val = parse_hex(match[2]);
            expectations.push_back({
                Expectation::Type::MEMORY,
                "",
                addr,
                static_cast<uint16_t>(val)
            });
        } else if (std::regex_search(line, match, err_expect)) {
            expectations.push_back({
                Expectation::Type::ASM_ERROR,
                "",
                0,
                0
            });
        }
    }
    return expectations;
}

TestResult TestRunner::run_file(const std::string& file_path) {
    TestResult result;
    result.test_name = file_path;
    result.success = true;

    std::ifstream file(file_path);
    if (!file.is_open()) {
        result.success = false;
        result.errors.push_back("Could not open file.");
        return result;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    auto expectations = parse_expectations(source);
    if (expectations.empty()) {
        result.success = false;
        result.errors.push_back("No expectations found in file. Use '; EXPECT REG=VAL' format.");
        return result;
    }

    bool expect_asm_error = false;
    for (const auto& exp : expectations) {
        if (exp.type == Expectation::Type::ASM_ERROR) expect_asm_error = true;
    }

    emu8086::assembler::Assembler asmb;
    auto asm_res = asmb.assemble(source, 0x0100);
    
    if (expect_asm_error) {
        if (asm_res.success) {
            result.success = false;
            result.errors.push_back("Expected assembly to fail, but it succeeded.");
        }
        return result;
    }

    if (!asm_res.success) {
        result.success = false;
        result.errors.push_back("Assembly failed:");
        for (const auto& err : asm_res.errors) {
            result.errors.push_back(fmt::format("  Line {}: {}", err.line, err.message));
        }
        return result;
    }

    emu8086::core::Emulator emu;
    emu.load_com(asm_res.machine_code, 0x0100);

    const uint32_t MAX_CYCLES = 1000000;
    uint32_t cycles = 0;

    while (emu.state() != emu8086::core::EmulatorState::HALTED && 
           emu.state() != emu8086::core::EmulatorState::ERROR && 
           cycles < MAX_CYCLES) {
        emu.step();
        cycles++;
    }

    result.cycles_executed = cycles;

    if (emu.state() == emu8086::core::EmulatorState::ERROR) {
        result.success = false;
        result.errors.push_back("Emulator entered ERROR state.");
    } else if (cycles >= MAX_CYCLES) {
        result.success = false;
        result.errors.push_back("Maximum cycle count exceeded (infinite loop?).");
    }

    auto snap = emu.snapshot();
    auto mem = emu.memory_view();

    auto get_reg = [&](const std::string& name) -> uint16_t {
        if (name == "AX") return snap.regs.AX;
        if (name == "BX") return snap.regs.BX;
        if (name == "CX") return snap.regs.CX;
        if (name == "DX") return snap.regs.DX;
        if (name == "SI") return snap.regs.SI;
        if (name == "DI") return snap.regs.DI;
        if (name == "SP") return snap.regs.SP;
        if (name == "BP") return snap.regs.BP;
        if (name == "CS") return snap.regs.CS;
        if (name == "DS") return snap.regs.DS;
        if (name == "ES") return snap.regs.ES;
        if (name == "SS") return snap.regs.SS;
        if (name == "IP") return snap.regs.IP;
        
        // 8-bit registers
        if (name == "AL") return snap.regs.AX & 0xFF;
        if (name == "AH") return snap.regs.AX >> 8;
        if (name == "BL") return snap.regs.BX & 0xFF;
        if (name == "BH") return snap.regs.BX >> 8;
        if (name == "CL") return snap.regs.CX & 0xFF;
        if (name == "CH") return snap.regs.CX >> 8;
        if (name == "DL") return snap.regs.DX & 0xFF;
        if (name == "DH") return snap.regs.DX >> 8;
        
        return 0;
    };

    auto get_flag = [&](const std::string& name) -> bool {
        if (name == "CF") return snap.flags.CF;
        if (name == "PF") return snap.flags.PF;
        if (name == "AF") return snap.flags.AF;
        if (name == "ZF") return snap.flags.ZF;
        if (name == "SF") return snap.flags.SF;
        if (name == "TF") return snap.flags.TF;
        if (name == "IF") return snap.flags.IF;
        if (name == "DF") return snap.flags.DF;
        if (name == "OF") return snap.flags.OF;
        return false;
    };

    for (const auto& exp : expectations) {
        if (exp.type == Expectation::Type::REGISTER) {
            uint16_t actual = get_reg(exp.name);
            if (actual != exp.expected_value) {
                result.success = false;
                result.errors.push_back(fmt::format("Mismatch {}: expected {:04X}h, got {:04X}h", exp.name, exp.expected_value, actual));
            }
        } else if (exp.type == Expectation::Type::FLAG) {
            bool actual = get_flag(exp.name);
            bool expected = exp.expected_value != 0;
            if (actual != expected) {
                result.success = false;
                result.errors.push_back(fmt::format("Mismatch {}: expected {}, got {}", exp.name, expected, actual));
            }
        } else if (exp.type == Expectation::Type::MEMORY) {
            if (exp.address >= mem.size()) {
                result.success = false;
                result.errors.push_back(fmt::format("Memory address {:05X}h out of bounds", exp.address));
                continue;
            }
            uint8_t actual = mem[exp.address];
            if (actual != (exp.expected_value & 0xFF)) {
                result.success = false;
                result.errors.push_back(fmt::format("Mismatch MEM[{:05X}h]: expected {:02X}h, got {:02X}h", exp.address, exp.expected_value & 0xFF, actual));
            }
        }
    }

    return result;
}

} // namespace emu_test

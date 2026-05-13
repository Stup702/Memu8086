#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace emu8086::assembler {

struct AssemblerError {
    int line;
    std::string message;
};

struct AssemblyResult {
    bool success{true};
    std::vector<uint8_t> machine_code;
    std::vector<AssemblerError> errors;
    std::map<int, uint16_t> line_to_offset;
    std::map<uint16_t, int> offset_to_line;
    std::map<std::string, uint16_t> symbols;
};

class Assembler {
public:
    AssemblyResult assemble(const std::string& source, uint16_t origin = 0x0000);
};

} // namespace emu8086::assembler
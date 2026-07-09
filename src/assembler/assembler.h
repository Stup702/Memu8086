// memu8086
// Copyright (C) 2026 Animesh Barua Mugdha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

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
    std::vector<uint8_t> code_bytes;
    std::vector<uint8_t> data_bytes;
    std::vector<AssemblerError> errors;
    std::map<int, uint16_t> line_to_offset;
    std::map<uint16_t, int> offset_to_line;
    std::map<std::string, uint16_t> symbols;

    uint16_t data_segment_offset = 0;
    uint16_t code_segment_offset = 0x100;
    uint16_t stack_size = 0x100;
    bool has_model_directive = false;

    std::map<std::string, int> sym_lengths;
    std::map<std::string, int> sym_sizes;
    std::map<std::string, int> sym_types;
};

class Assembler {
public:
    AssemblyResult assemble(const std::string& source, uint16_t origin = 0x0000);
};

} // namespace emu8086::assembler
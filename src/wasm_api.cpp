#include <emscripten/bind.h>
#include "core/cpu.h"
#include "core/emulator.h"
#include "assembler/assembler.h"
#include <string>
#include <vector>

using namespace emscripten;
using namespace emu8086;

class WasmEmulator {
private:
    core::Emulator emulator;

public:
    WasmEmulator() {}

    bool assemble(const std::string& source) {
        return emulator.load_from_assembly(source);
    }

    void reset() {
        emulator.stop();
        emulator.load_from_assembly(""); // clear
    }

    void step() {
        emulator.step();
    }

    void run() {
        // For WASM we probably just want to run until halted or loop,
        // but since `Emulator::start()` uses std::thread which may not work in WASM without pthread,
        // we might just step in a loop.
        while (emulator.state() != core::EmulatorState::HALTED && 
               emulator.state() != core::EmulatorState::ERROR &&
               emulator.state() != core::EmulatorState::IDLE) {
            emulator.step();
        }
    }
    
    bool is_halted() const {
        return emulator.state() == core::EmulatorState::HALTED;
    }

    int get_current_line() const {
        uint16_t ip = emulator.snapshot().regs.IP;
        const auto& asm_res = emulator.last_assembly();
        auto it = asm_res.offset_to_line.find(ip);
        if (it != asm_res.offset_to_line.end()) {
            return it->second;
        }
        int closest_line = -1;
        uint16_t closest_offset = 0;
        for (const auto& pair : asm_res.offset_to_line) {
            if (pair.first <= ip && pair.first >= closest_offset) {
                closest_offset = pair.first;
                closest_line = pair.second;
            }
        }
        return closest_line;
    }

    std::string get_output() {
        return emulator.consume_output();
    }

    void send_input(int char_code) {
        emulator.send_key(static_cast<char>(char_code));
    }

    val get_registers() const {
        core::EmulatorSnapshot snap = emulator.snapshot();
        val regs = val::object();
        regs.set("AX", snap.regs.AX);
        regs.set("BX", snap.regs.BX);
        regs.set("CX", snap.regs.CX);
        regs.set("DX", snap.regs.DX);
        regs.set("SI", snap.regs.SI);
        regs.set("DI", snap.regs.DI);
        regs.set("SP", snap.regs.SP);
        regs.set("BP", snap.regs.BP);
        regs.set("IP", snap.regs.IP);
        regs.set("CS", snap.regs.CS);
        regs.set("DS", snap.regs.DS);
        regs.set("ES", snap.regs.ES);
        regs.set("SS", snap.regs.SS);
        
        val flags = val::object();
        flags.set("CF", snap.regs.flags.CF);
        flags.set("PF", snap.regs.flags.PF);
        flags.set("AF", snap.regs.flags.AF);
        flags.set("ZF", snap.regs.flags.ZF);
        flags.set("SF", snap.regs.flags.SF);
        flags.set("TF", snap.regs.flags.TF);
        flags.set("IF", snap.regs.flags.IF);
        flags.set("DF", snap.regs.flags.DF);
        flags.set("OF", snap.regs.flags.OF);
        
        regs.set("flags", flags);
        return regs;
    }

    val get_memory(uint32_t start, uint32_t size) const {
        const auto& mem = emulator.memory_view();
        if (start + size > mem.size()) size = mem.size() - start;
        return val(typed_memory_view(size, mem.data() + start));
    }
};

EMSCRIPTEN_BINDINGS(memu8086_module) {
    class_<WasmEmulator>("WasmEmulator")
        .constructor<>()
        .function("assemble", &WasmEmulator::assemble)
        .function("reset", &WasmEmulator::reset)
        .function("step", &WasmEmulator::step)
        .function("run", &WasmEmulator::run)
        .function("is_halted", &WasmEmulator::is_halted)
        .function("get_current_line", &WasmEmulator::get_current_line)
        .function("get_output", &WasmEmulator::get_output)
        .function("send_input", &WasmEmulator::send_input)
        .function("get_registers", &WasmEmulator::get_registers)
        .function("get_memory", &WasmEmulator::get_memory);
}

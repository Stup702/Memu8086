#include <cassert>
#include "../core/cpu.h"
#include "../core/emulator.h"
#include "../assembler/assembler.h"

using namespace emu8086;

void test_cpu_flags() {
    core::CPU cpu;
    cpu.reset();
    cpu.update_flags_add(0x00FF, 0x0001, 0x0100, false);
    assert(cpu.regs.flags.CF == true);
}

void test_assembler_mov() {
    assembler::Assembler asm_obj;
    auto res = asm_obj.assemble("mov ax, 1234h");
    assert(res.success);
    assert(res.machine_code.size() == 3);
    assert(res.machine_code[0] == 0xB8);
    assert(res.machine_code[1] == 0x34);
    assert(res.machine_code[2] == 0x12);
}

void test_memory_rw() {
    core::Memory mem;
    mem.reset();
    mem.write16(0x1000, 0xABCD);
    assert(mem.read16(0x1000) == 0xABCD);
    assert(mem.read8(0x1000) == 0xCD);
    assert(mem.read8(0x1001) == 0xAB);
}

void test_int21_print() {
    core::Emulator emu;
    assembler::Assembler asm_obj;
    auto res = asm_obj.assemble("mov ah, 02h\nmov dl, 41h\nint 21h\nhlt\n");
    assert(res.success);
    emu.load_com(res.machine_code);
}

int main() {
    test_cpu_flags();
    test_assembler_mov();
    test_memory_rw();
    test_int21_print();
    return 0;
}
#include <gbcpu.h>
#include <stdio.h>

const OpInstr opcodes[] = {
    [0x00] = {nop, implied, 1, 4, "NOP"},
    [0x31] = {ld_16, implied, 3, 12, "LD "},
    [0xC3] = {jp, immediate_ext, 3, 12, "JP "},
    [0xF3] = {di, implied, 1, 4, "DI "},
};

void cpu_initialize(GBCPU *cpu) {
    cpu->reg.AF = 0x0000;
    cpu->reg.BC = 0x0000;
    cpu->reg.DE = 0x0000;
    cpu->reg.HL = 0x0000;
    cpu->reg.SP = 0x0000;
    cpu->reg.PC = 0x0000;
}

void cpu_reset(GBCPU *cpu) {
    cpu->reg.AF = 0x01B0;
    cpu->reg.BC = 0x0013;
    cpu->reg.DE = 0x00D8;
    cpu->reg.HL = 0x014D;
    cpu->reg.SP = 0xFFFE;
    cpu->reg.PC = 0x0100;

    cpu->addr_abs = 0x0000;
}

uint8_t cpu_read(GBCPU *cpu) {
    uint8_t data = cpu->memory[cpu->reg.PC];
    cpu->reg.PC++;
    return data;
}   

void cpu_clock(GBCPU *cpu) {
    uint16_t addr = cpu->reg.PC;
    uint8_t opcode = cpu_read(cpu);
    printf("--> 0x%02X\n", opcode);
    OpInstr instr = opcodes[opcode];
    instr.addr_mode(cpu);
    instr.instruction(cpu);

    printf("[$%04X][0x%02X] %s 0x%04X\n", addr, opcode, instr.name, cpu->addr_abs);
}

uint8_t bus_read(GBCPU *cpu, uint16_t addr) {
    return cpu->memory[addr];
}

void implied(GBCPU * cpu) {
    (void)cpu;
}

void implied_sp(GBCPU * cpu) {
    (void)cpu;
}

void immediate_ext(GBCPU *cpu) {
    cpu->addr_abs = cpu->reg.PC;
}

void di(GBCPU *cpu) {
    (void)cpu;
    // disable interrupts after
}

void jp(GBCPU *cpu) {
    uint8_t lo = bus_read(cpu, cpu->addr_abs);
    uint8_t hi = bus_read(cpu, cpu->addr_abs + 1);
    cpu->reg.PC = (hi << 8) | lo;
}

void ld_16(GBCPU *cpu) {
    uint8_t lo = cpu_read(cpu);
    uint8_t hi = cpu_read(cpu);
    uint16_t value = (hi << 8) | lo;
    value += 1;

}

void nop(GBCPU *cpu) {
    (void)cpu;
}

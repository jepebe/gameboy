#include <stdlib.h>

#include "acutest.h"
#include "gbcpu.h"
#include "tools.h"

void test_inc_16_and_dec_16() {
    uint16_t values[] = {0x0000, 0x0001, 0x000F, 0x0010, 0x001F, 0x007F, 0x0080, 0x00FF,
                         0x0100, 0x0F00, 0x1F00, 0x1000, 0x7FFF, 0x8000, 0xFFFF};
    GBCPU cpu;
    cpu_initialize(&cpu);
    cpu_reset(&cpu);

    cpu.memory[0x0100] = 0x33;  //opcode for inc_16 SP
    cpu.memory[0x0101] = 0x3B;  //opcode for dec_16 SP
    cpu.memory[0x0102] = 0x3B;  //opcode for dec_16 SP
    cpu.memory[0x0103] = 0x23;  //opcode for inc_16 HL
    cpu.memory[0x0104] = 0x2B;  //opcode for dec_16 HL
    cpu.memory[0x0105] = 0x2B;  //opcode for dec_16 HL
    cpu.memory[0x0106] = 0x13;  //opcode for inc_16 DE
    cpu.memory[0x0107] = 0x1B;  //opcode for dec_16 DE
    cpu.memory[0x0108] = 0x1B;  //opcode for dec_16 DE
    cpu.memory[0x0109] = 0x03;  //opcode for inc_16 BC
    cpu.memory[0x010A] = 0x0B;  //opcode for dec_16 BC
    cpu.memory[0x010B] = 0x0B;  //opcode for dec_16 BC

    cpu.reg.F = 0xF0;  // should also test 0x00
    for (size_t i = 0; i < 15; ++i) {
        cpu.reg.PC = 0x0100;
        uint16_t val = values[i];
        uint16_t inc_val = values[i] + 1;
        uint16_t dec_val = values[i] - 1;

        cpu.reg.SP = val;
        cpu_clock(&cpu, false, false);
        TEST_CHECK(cpu.reg.SP == inc_val);
        TEST_MSG("INC SP=0x%04X != 0x%04X\n", cpu.reg.SP, inc_val);
        cpu_clock(&cpu, false, false);
        TEST_CHECK(cpu.reg.SP == val);
        TEST_MSG("DEC SP=0x%04X != 0x%04X\n", cpu.reg.SP, val);
        cpu_clock(&cpu, false, false);
        TEST_CHECK(cpu.reg.SP == dec_val);
        TEST_MSG("DEC SP=0x%04X != 0x%04X\n", cpu.reg.SP, dec_val);

        cpu.reg.HL = val;
        cpu_clock(&cpu, false, false);
        TEST_CHECK(cpu.reg.HL == inc_val);
        TEST_MSG("INC HL=0x%04X != 0x%04X\n", cpu.reg.HL, inc_val);
        cpu_clock(&cpu, false, false);
        TEST_CHECK(cpu.reg.HL == val);
        TEST_MSG("DEC HL=0x%04X != 0x%04X\n", cpu.reg.HL, val);
        cpu_clock(&cpu, false, false);
        TEST_CHECK(cpu.reg.HL == dec_val);
        TEST_MSG("DEC HL=0x%04X != 0x%04X\n", cpu.reg.HL, dec_val);

        cpu.reg.DE = val;
        cpu_clock(&cpu, false, false);
        TEST_CHECK(cpu.reg.DE == inc_val);
        TEST_MSG("INC DE=0x%04X != 0x%04X\n", cpu.reg.DE, inc_val);
        cpu_clock(&cpu, false, false);
        TEST_CHECK(cpu.reg.DE == val);
        TEST_MSG("DEC DE=0x%04X != 0x%04X\n", cpu.reg.DE, val);
        cpu_clock(&cpu, false, false);
        TEST_CHECK(cpu.reg.DE == dec_val);
        TEST_MSG("DEC DE=0x%04X != 0x%04X\n", cpu.reg.DE, dec_val);

        cpu.reg.BC = val;
        cpu_clock(&cpu, false, false);
        TEST_CHECK(cpu.reg.BC == inc_val);
        TEST_MSG("INC BC=0x%04X != 0x%04X\n", cpu.reg.BC, inc_val);
        cpu_clock(&cpu, false, false);
        TEST_CHECK(cpu.reg.BC == val);
        TEST_MSG("DEC BC=0x%04X != 0x%04X\n", cpu.reg.BC, val);
        cpu_clock(&cpu, false, false);
        TEST_CHECK(cpu.reg.BC == dec_val);
        TEST_MSG("DEC BC=0x%04X != 0x%04X\n", cpu.reg.BC, dec_val);
    }
    TEST_CHECK(cpu.reg.F == 0xF0);
}

void test_add() {
    GBCPU cpu;
    cpu_initialize(&cpu);
    cpu.reg.F = 0x00;

    // simulate adding 0x7FFF with 0x0001

    uint8_t res = add(&cpu, 0xFF, 0x01, false);

    TEST_CHECK(res == 0x00);
    TEST_CHECK(cpu.reg.flags.c == 1);
    TEST_CHECK(cpu.reg.flags.h == 1);

    res = add(&cpu, 0x7F, 0x00, cpu.reg.flags.c);

    TEST_CHECK(res == 0x80);
    TEST_CHECK(cpu.reg.flags.c == 0);
    TEST_CHECK(cpu.reg.flags.h == 1);

    // switch order on last operation
    res = add(&cpu, 0x00, 0x7F, 1);

    TEST_CHECK(res == 0x80);
    TEST_CHECK(cpu.reg.flags.c == 0);
    TEST_CHECK(cpu.reg.flags.h == 1);
}

void test_add_sp_r8() {
    GBCPU cpu;
    cpu_initialize(&cpu);
    cpu_reset(&cpu);

    cpu.reg.SP = 0x0000;
    cpu.memory[0x0100] = 0xE8;
    cpu.memory[0x0101] = 0xFF;

    cpu_clock(&cpu, false, false);
    TEST_CHECK(cpu.reg.SP == 0xFFFF);
    TEST_MSG("Expected 0x%04X found 0x%04X", 0xFFFF, cpu.reg.SP);

    TEST_CHECK(cpu.reg.flags.c == 0);
    TEST_CHECK(cpu.reg.flags.h == 0);
}

void test_print_flags() {
    GBCPU cpu;
    cpu_initialize(&cpu);
    cpu_reset(&cpu);

    cpu.memory[0x0100] = 0x33;
    cpu.memory[0x0101] = 0x3B;
    cpu.memory[0x0102] = 0x3B;
    cpu.memory[0x0103] = 0x23;

    cpu_clock(&cpu, true, false);

    char* expected =
        "A: 01 F: B0 B: 00 C: 13 D: 00 E: D8 H: 01 L: 4D "
        "SP: FFFE PC: 00:0100 (33 3B 3B 23)";

    TEST_CHECK(strcmp(expected, cpu.debug) == 0);
    TEST_MSG("%s != %s", expected, cpu.debug);
}

TEST_LIST = {
    {"INC 16 and DEC 16", test_inc_16_and_dec_16},
    {"ADD Add with carry", test_add },
    {"ADD SP,r8", test_add_sp_r8 },
    {"Print Flags", test_print_flags},
    {NULL, NULL} /* zeroed record marking the end of the list */
};

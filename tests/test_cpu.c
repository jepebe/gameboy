#include <stdlib.h>

#include "acutest.h"
#include "gbcpu.h"
#include "cartridge.h"
#include "tools.h"

const uint8_t nintendo[] = {
    0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D,
    0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99,
    0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E
};

void test_blargg_binary() {
    GBCPU cpu;
    cpu_initialize(&cpu);
    cpu_reset(&cpu);

    bool success = read_binary("roms/cpu_instrs.gb", cpu.memory);
    TEST_CHECK(success);

    const Cartridge* cartridge = cartridge_allocate(cpu.memory);

    for(size_t i = 0; i < 48; ++i) {
        TEST_CHECK(cartridge->nintendo[i] == nintendo[i ]);
        TEST_MSG("0x%02X != 0x%02X", cartridge->nintendo[i], nintendo[i]);
    }

    TEST_CHECK(strcmp(cartridge->title, "CPU_INSTRS") == 0);
    TEST_MSG("strcmp() -> %d != 0", strcmp(cartridge->title, "CPU_INSTRS"));

    //TEST_CHECK(cartridge->gb_type == 0x80);
    //TEST_MSG("0x%02X != 0x%02X", cartridge->gb_type, 0x80);

    TEST_CHECK(cartridge->lic_code_high == 0x00);
    TEST_CHECK(cartridge->lic_code_low == 0x00);

    TEST_CHECK(cartridge->gb_sgb_indicator == 0x00);

    TEST_CHECK(cartridge->cartridge_type == ROM_MBC1);
    TEST_MSG("0x%02X != 0x%02X", cartridge->cartridge_type, ROM_MBC1);

    TEST_CHECK(cartridge->rom_size == ROM_64_KB);
    TEST_CHECK(cartridge->ram_size == RAM_NONE);

    TEST_CHECK(cartridge->destination_code == JAPANESE);
    TEST_MSG("0x%02X != 0x%02X", cartridge->destination_code, JAPANESE);

    TEST_CHECK(cartridge->lic_code_old == 0x00);
    TEST_MSG("0x%02X != 0x%02X", cartridge->lic_code_old, 0x00);

    TEST_CHECK(cartridge->mask_rom_version == 0x00);
    TEST_MSG("0x%02X != 0x%02X", cartridge->mask_rom_version, 0x00);

    uint8_t checksum = cartridge_header_checksum(cpu.memory);
    TEST_CHECK(cartridge->complement_check == checksum);
    TEST_MSG("0x%02X != 0x%02X", cartridge->complement_check, checksum);

    TEST_CHECK(cartridge->checksum == 0xF530);
    TEST_MSG("0x%04X != 0x%04X", cartridge->checksum, 0xF530);

    free((void*)cartridge);
}

void test_cpu_registers() {
    cpu_registers reg;
    
    reg.AF = 0xAA55;
    TEST_CHECK(reg.A == 0xAA);
    TEST_CHECK(reg.F == 0x55);
    TEST_MSG("%d != %d", reg.A, 0xAA);
    TEST_MSG("%d != %d", reg.F, 0x55);

    TEST_CHECK(reg.flags.areg == 0xAA);
    TEST_CHECK(reg.flags.z == 0);
    TEST_CHECK(reg.flags.n == 1);
    TEST_CHECK(reg.flags.h == 0);
    TEST_CHECK(reg.flags.c == 1);
    
    reg.F = 0xF0;
    TEST_CHECK(reg.flags.z == 1);
    TEST_CHECK(reg.flags.n == 1);
    TEST_CHECK(reg.flags.h == 1);
    TEST_CHECK(reg.flags.c == 1);

    reg.F = 0xAA;
    TEST_CHECK(reg.flags.z == 1);
    TEST_CHECK(reg.flags.n == 0);
    TEST_CHECK(reg.flags.h == 1);
    TEST_CHECK(reg.flags.c == 0);

    reg.BC = 0x6699;
    TEST_CHECK(reg.B == 0x66);
    TEST_CHECK(reg.C == 0x99);
    
    reg.DE = 0x66AA;
    TEST_CHECK(reg.D == 0x66);
    TEST_CHECK(reg.E == 0xAA);

    reg.HL = 0x5599;
    TEST_CHECK(reg.H == 0x55);
    TEST_CHECK(reg.L == 0x99);

    reg.SP = 0xFFFF;
    reg.SP += 1;
    TEST_CHECK(reg.SP == 0x00);

    reg.HL = 0x0000;
    reg.L = 0xFF;
    TEST_CHECK(reg.L == 0xFF);
    TEST_CHECK(reg.H == 0x00);
    reg.L += 1;
    TEST_CHECK(reg.L == 0x00);
    TEST_CHECK(reg.H == 0x00);
}

void test_reset() {
    GBCPU cpu;
    cpu_initialize(&cpu);
    cpu_reset(&cpu);

    TEST_CHECK(cpu.reg.AF == 0x01B0);
    TEST_CHECK(cpu.reg.BC == 0x0013);
    TEST_CHECK(cpu.reg.DE == 0x00D8);
    TEST_CHECK(cpu.reg.HL == 0x014D);
    TEST_CHECK(cpu.reg.SP == 0xFFFE);
    TEST_CHECK(cpu.reg.PC == 0x0100);
}

TEST_LIST = {
   { "Blargg CPU binary", test_blargg_binary },
   { "CPU Registers", test_cpu_registers },
   { "CPU Reset", test_reset },
   { NULL, NULL }     /* zeroed record marking the end of the list */
};

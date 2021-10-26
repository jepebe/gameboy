#include <stdlib.h>

#include "acutest.h"
#include "cartridge.h"
#include "gbcpu.h"
#include "tools.h"

bool run_cpu(GBCPU *cpu, FILE *log, size_t last_instruction) {
    char line[256];
    // uint8_t mem_val_d = cpu->memory[0xFF0F];
    // uint8_t mem_val_e = cpu->memory[0xDF7E];
    // uint8_t mem_val_c = cpu->memory[0xDF7C];

    while (!cpu->crashed && cpu->instruction_count <= last_instruction) {
        cpu_clock(cpu, false, false);

        // if (mem_val_d != cpu->memory[0xFF0F]) {
        // printf("%s $FF0F=0x%02X\n", &cpu->disassembly[0], mem_val_d);
        // mem_val_d = cpu->memory[0xFF0F];
        // }

        // mem_val_d = cpu->memory[0xDF7D];
        //     mem_val_e = cpu->memory[0xDF7E];
        //     mem_val_c = cpu->memory[0xDF7C];
        // printf("%s $DF7D=0x%02X $DF7E=0x%02X $DF7C=0x%02X\n", &cpu->disassembly[0], mem_val_d, mem_val_e, mem_val_c);
        // printf("%s $0104=0x%02X\n", &cpu->disassembly[0], mem_val_d);
        // }

        if (!cpu->crashed) {
            if (serial_buffer_eol(&cpu->buffer)) {
                printf("%s", &cpu->buffer.buffer[0]);

                // if (strcmp("Passed\n", &cpu->buffer.buffer[0]) == 0) {
                //     printf("Test completed after %zu instructions\n", cpu->instruction_count);
                //     serial_buffer_clear(&cpu->buffer);
                //     return true;
                // }

                serial_buffer_clear(&cpu->buffer);
            }
            if (log && !fgets(line, sizeof(line), log)) {
                // bool success = TEST_CHECK(false);
                // TEST_MSG("Log is empty!");
                // if (!success)
                // return false;
            }

            // line[strcspn(line, "\n")] = '\0';  // strip newline
            // bool success = TEST_CHECK(strcmp(line, cpu->debug) == 0);
            // TEST_MSG("%8zu opcode=0x%02X", cpu->instruction_count - 1, cpu->opcode);
            // TEST_MSG("\033[0;32mExpected: %s\033[0m", line);
            // TEST_MSG("\033[0;31mFound   : %s\033[0m", cpu->debug);
            // if (!success)
            //     return false;
        }
    }
    return true;
}

void run_disassembly(GBCPU *cpu, size_t last_line) {
    size_t first_line = last_line - 10;
    while (!cpu->crashed && cpu->instruction_count < last_line) {
        cpu_clock(cpu, false, cpu->instruction_count >= first_line);

        char *color = cpu->instruction_count == last_line ? "\033[0;31m" : "";
        char *end_color = cpu->instruction_count == last_line ? "\033[0m" : "";

        if (cpu->instruction_count >= first_line) {
            printf("%s%s%s\n", color, &cpu->disassembly[0], end_color);
        }
    }
}

void run_and_test_rom(char *rom, char *log, size_t last_instruction) {
    GBCPU cpu;
    cpu_initialize(&cpu);
    cpu_reset(&cpu);
    read_binary(rom, cpu.memory);

    cpu.memory[0xFF44] = 0x90; // LY

    FILE *file;
    if (log != NULL) {
        file = fopen(log, "r");
    } else {
        file = NULL;
    }

    bool success = run_cpu(&cpu, file, last_instruction);
    success = !cpu.crashed && success;

    if (cpu.buffer.pos > 0) {
        printf("%s", &cpu.buffer.buffer[0]);
    }

    if (!success) {
        // print assembly for some lines before the error
        size_t instruction_number = cpu.instruction_count;
        cpu_reset(&cpu);
        cpu.memory[0xFF44] = 0x90; // LY
        read_binary(rom, cpu.memory);
        run_disassembly(&cpu, instruction_number);
    }

    TEST_CHECK(success);
}

void run_rom(char *rom) {
    GBCPU cpu;
    cpu_initialize(&cpu);
    cpu_reset(&cpu);
    read_binary(rom, cpu.memory);

    cpu.memory[0xFF44] = 0x90; // LY

    while (!cpu.crashed) {
        cpu_clock(&cpu, false, false);

        if (!cpu.crashed) {
            if (serial_buffer_eol(&cpu.buffer)) {
                printf("%s", &cpu.buffer.buffer[0]);

                if (strcmp("Passed\n", &cpu.buffer.buffer[0]) == 0) {
                    printf("Test completed after %zu instructions\n", cpu.instruction_count);
                    serial_buffer_clear(&cpu.buffer);
                    break;
                }

                serial_buffer_clear(&cpu.buffer);
            }
        }
    }

    if (cpu.buffer.pos > 0) {
        printf("%s", &cpu.buffer.buffer[0]);
    }

    if (cpu.crashed) {
        printf("CPU crashed\n");
    }
}

void test_blargg_cpu_instrs() {
    run_rom("../tests/roms/cpu_instrs.gb");
}

void test_blargg_special() {
    char *rom = "../tests/roms/01-special.gb";
    char *log = "../tests/logs/01-special.txt";
    run_and_test_rom(rom, log, 1258894);
}

void test_blargg_interrupts() {
    char *rom = "../tests/roms/02-interrupts.gb";
    char *log = "../tests/logs/02-interrupts.txt";
    run_and_test_rom(rom, log, 186112);
}

void test_blargg_op_sp_hl() {
    char *rom = "../tests/roms/03-op sp,hl.gb";
    char *log = "../tests/logs/03-op sp,hl.txt";
    run_and_test_rom(rom, log, 1068421);
}

void test_blargg_op_r_imm() {
    char *rom = "../tests/roms/04-op r,imm.gb";
    char *log = "../tests/logs/04-op r,imm.txt";
    run_and_test_rom(rom, log, 1262765);
}

void test_blargg_op_rp() {
    char *rom = "../tests/roms/05-op rp.gb";
    char *log = "../tests/logs/05-op rp.txt";
    run_and_test_rom(rom, log, 1763387);
}

void test_blargg_ld_r_r() {
    char *rom = "../tests/roms/06-ld r,r.gb";
    char *log = "../tests/logs/06-ld r,r.txt";
    run_and_test_rom(rom, log, 243272);
}

void test_blargg_jr_jp_call_ret_rst() {
    char *rom = "../tests/roms/07-jr,jp,call,ret,rst.gb";
    char *log = "../tests/logs/07-jr,jp,call,ret,rst.txt";
    run_and_test_rom(rom, log, 287415);
}

void test_blargg_misc_instrs() {
    char *rom = "../tests/roms/08-misc instrs.gb";
    char *log = "../tests/logs/08-misc instrs.txt";
    run_and_test_rom(rom, log, 223891);
}

void test_blargg_op_r_r() {
    char *rom = "../tests/roms/09-op r,r.gb";
    char *log = "../tests/logs/09-op r,r.txt";
    run_and_test_rom(rom, log, 4420381);
}

void test_blargg_bit_ops() {
    char *rom = "../tests/roms/10-bit ops.gb";
    char *log = "../tests/logs/10-bit ops.txt";
    run_and_test_rom(rom, log, 6714722);
}

void test_blargg_op_a_hl() {
    char *rom = "../tests/roms/11-op a,(hl).gb";
    char *log = "../tests/logs/11-op a,(hl).txt";
    run_and_test_rom(rom, log, 7429761);
}

void test_bootstrap_rom() {
    char *rom = "../tests/roms/DMG_ROM.bin";
    // char* log = "../tests/logs/DMG_ROM.txt";

    GBCPU cpu;
    cpu_initialize(&cpu);
    read_binary(rom, cpu.memory);

    uint8_t logo[] = {0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00,
                      0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89,
                      0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB,
                      0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F,
                      0xBB, 0xB9, 0x33, 0x3E};

    memcpy(&cpu.memory[0x0104], logo, sizeof(logo));

    cpu.memory[0x014D] = cartridge_header_checksum(&cpu.memory[0]);

    cpu.memory[0xFF44] = 0x90; // LY

    while (!cpu.crashed && cpu.reg.PC != 0x0100) {
        cpu_clock(&cpu, false, false);

        if (!cpu.crashed) {
            if (cpu.disassembly[0]) {
                printf("%s\n", &cpu.disassembly[0]);
            }

            if (serial_buffer_eol(&cpu.buffer)) {
                printf("%s", &cpu.buffer.buffer[0]);

                serial_buffer_clear(&cpu.buffer);
            }
        }
    }

    TEST_CHECK(!cpu.crashed);
    TEST_CHECK(cpu.reg.AF == 0x01B0);
    TEST_CHECK(cpu.reg.BC == 0x0013);
    TEST_CHECK(cpu.reg.DE == 0x00D8);
    TEST_CHECK(cpu.reg.HL == 0x014D);
    TEST_CHECK(cpu.reg.SP == 0xFFFE);
    TEST_CHECK(cpu.reg.PC == 0x0100);
}

TEST_LIST = {
    {"Bootstrap ROM", test_bootstrap_rom},
    // {"Blargg CPU instructions", test_blargg_cpu_instrs},
    {"Blargg Special", test_blargg_special},                       // complete
    {"Blargg Interrupts", test_blargg_interrupts},                 // wrong value in $FF0F
    {"Blargg op SP,HL", test_blargg_op_sp_hl},                     // complete?
    {"Blargg op R,IMM", test_blargg_op_r_imm},                     // complete?
    {"Blargg op RP", test_blargg_op_rp},                           // complete?
    {"Blargg ld R,R", test_blargg_ld_r_r},                         // complete?
    {"Blargg JR JP CALL RET RST", test_blargg_jr_jp_call_ret_rst}, // complete? fails w/log
    {"Blargg misc instructions", test_blargg_misc_instrs},         // complete?
    {"Blargg op r,r", test_blargg_op_r_r},                         // complete?
    {"Blargg bit ops", test_blargg_bit_ops},                       // complete?
    {"Blargg op a,(hl)", test_blargg_op_a_hl},                     // complete
    {NULL, NULL}                                                   /* zeroed record marking the end of the list */
};

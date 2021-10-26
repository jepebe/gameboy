#include "gbcpu.h"

#include <stdio.h>
#include <string.h>

void cpu_initialize(GBCPU *cpu) {
    cpu->reg.AF = 0x0000;
    cpu->reg.BC = 0x0000;
    cpu->reg.DE = 0x0000;
    cpu->reg.HL = 0x0000;
    cpu->reg.SP = 0x0000;
    cpu->reg.PC = 0x0000;

    cpu->ime = false;
    cpu->opcode = 0x00;
    cpu->instruction_count = 0;

    cpu->src.ptr = NULL;
    cpu->src.addr = 0x000;
    cpu->src.reg = false;

    cpu->crashed = false;
    serial_buffer_clear(&cpu->buffer);
}

void cpu_reset(GBCPU *cpu) {
    cpu->reg.AF = 0x01B0;
    cpu->reg.BC = 0x0013;
    cpu->reg.DE = 0x00D8;
    cpu->reg.HL = 0x014D;
    cpu->reg.SP = 0xFFFE;
    cpu->reg.PC = 0x0100;

    memset(cpu->memory, '\0', 0x10000);

    cpu->ime = false;
    cpu->opcode = 0x00;
    cpu->instruction_count = 0;

    cpu->src.ptr = NULL;
    cpu->src.addr = cpu->reg.PC;
    cpu->src.reg = false;

    cpu->dst.ptr = NULL;
    cpu->dst.addr = cpu->reg.PC;
    cpu->dst.reg = false;

    cpu->crashed = false;
    serial_buffer_clear(&cpu->buffer);
}

uint8_t cpu_read(GBCPU *cpu) {
    uint8_t data = cpu->memory[cpu->reg.PC];
    cpu->reg.PC++;
    return data;
}

void cpu_push(GBCPU *cpu, uint8_t value) {
    cpu->memory[--cpu->reg.SP] = value;
}

uint8_t cpu_pop(GBCPU *cpu) {
    return cpu->memory[cpu->reg.SP++];
}

static void disassemble(GBCPU *cpu, OpInstr *instr, uint16_t addr) {
    sprintf(cpu->disassembly, "%8zu ", cpu->instruction_count);
    sprintf(cpu->disassembly + 9, "$%04X ", addr);
    sprintf(cpu->disassembly + 15, "[0x%02X ", cpu->memory[addr]);

    if (instr->length > 1) {
        sprintf(cpu->disassembly + 21, "0x%02X ", cpu->memory[addr + 1]);
    } else {
        sprintf(cpu->disassembly + 21, "     ");
    }

    if (instr->length > 2) {
        sprintf(cpu->disassembly + 26, "0x%02X] ", cpu->memory[addr + 2]);
    } else {
        sprintf(cpu->disassembly + 26, "    ] ");
    }

    char op[20];
    sprintf(op, "%s ", instr->name);

    if (instr->write_mode.repr[0] != '\0') {
        sprintf(op + strlen(op), "%s", instr->write_mode.repr);
    }
    if (instr->write_mode.repr[0] != '\0' && instr->read_mode.repr[0] != '\0') {
        sprintf(op + strlen(op), ", ");
    }
    if (instr->read_mode.repr[0] != '\0') {
        sprintf(op + strlen(op), "%s", instr->read_mode.repr);
    }

    sprintf(cpu->disassembly + 32, "%-18s ", op);
    sprintf(cpu->disassembly + 50, "A=%02X ", cpu->reg.A);
    sprintf(cpu->disassembly + 55, "BC=%04X ", cpu->reg.BC);
    sprintf(cpu->disassembly + 63, "DE=%04X ", cpu->reg.DE);
    sprintf(cpu->disassembly + 71, "HL=%04X ", cpu->reg.HL);
    sprintf(cpu->disassembly + 79, "SP=%04X ", cpu->reg.SP);

    sprintf(cpu->disassembly + 87, "%s", cpu->reg.flags.z ? "z" : ".");
    sprintf(cpu->disassembly + 88, "%s", cpu->reg.flags.n ? "n" : ".");
    sprintf(cpu->disassembly + 89, "%s", cpu->reg.flags.h ? "h" : ".");
    sprintf(cpu->disassembly + 90, "%s", cpu->reg.flags.c ? "c" : ".");
    sprintf(cpu->disassembly + 91, " F=%02X ", cpu->reg.F);
}

static void print_flags(GBCPU *cpu, uint16_t addr) {
    sprintf(cpu->debug, "A: %02X ", cpu->reg.A);
    sprintf(cpu->debug + 6, "F: %02X ", cpu->reg.F);
    sprintf(cpu->debug + 12, "B: %02X ", cpu->reg.B);
    sprintf(cpu->debug + 18, "C: %02X ", cpu->reg.C);
    sprintf(cpu->debug + 24, "D: %02X ", cpu->reg.D);
    sprintf(cpu->debug + 30, "E: %02X ", cpu->reg.E);
    sprintf(cpu->debug + 36, "H: %02X ", cpu->reg.H);
    sprintf(cpu->debug + 42, "L: %02X ", cpu->reg.L);
    sprintf(cpu->debug + 48, "SP: %04X ", cpu->reg.SP);
    sprintf(cpu->debug + 57, "PC: 00:%04X ", addr);
    sprintf(cpu->debug + 69, "(%02X ", cpu->memory[addr + 0]);
    sprintf(cpu->debug + 73, "%02X ", cpu->memory[addr + 1]);
    sprintf(cpu->debug + 76, "%02X ", cpu->memory[addr + 2]);
    sprintf(cpu->debug + 79, "%02X)", cpu->memory[addr + 3]);
}

void cpu_clock(GBCPU *cpu, bool debug, bool disassembly) {
    uint16_t addr = cpu->reg.PC;
    uint8_t opcode = cpu_read(cpu);
    cpu->opcode = opcode;

    OpInstr instr;

    if (opcode == 0xCB) {
        opcode = cpu_read(cpu);
        cpu->opcode = opcode;

        if (prefix_opcodes[opcode].instruction == 0x00) {
            printf("\n%8zu CB missing opcode = 0x%02X\n", cpu->instruction_count, opcode);
            cpu->crashed = true;
            cpu->reg.PC = addr;
            return;
        }
        instr = prefix_opcodes[opcode];
    } else {
        if (opcodes[opcode].instruction == 0x00) {
            printf("\n%8zu missing opcode = 0x%02X ", cpu->instruction_count, opcode);
            cpu->crashed = true;
            cpu->reg.PC = addr;
            return;
        }
        instr = opcodes[opcode];
    }

    if (disassembly) {
        disassemble(cpu, &instr, addr);
    } else {
        cpu->disassembly[0] = '\0';
    }
    if (debug) {
        print_flags(cpu, addr);
    } else {
        cpu->debug[0] = '\0';
    }

    instr.read_mode.addr_mode_func(cpu, &cpu->src);
    instr.write_mode.addr_mode_func(cpu, &cpu->dst);
    instr.instruction(cpu);

    if (cpu->memory[0xFF02] == 0x81) {
        serial_buffer_push(&cpu->buffer, cpu->memory[0xFF01]);
        // printf("%c", );
        // if(cpu->memory[0xFF01] == 'P') {
        //     cpu->crashed = true;
        // }
        cpu->memory[0xFF02] = 0x01;
    }

    if (addr == cpu->reg.PC) {
        // check for infinite loop
        if (!disassembly) {
            disassemble(cpu, &instr, addr);
        }
        cpu->crashed = true;
        fprintf(stderr, "\033[0;31m");
        fprintf(stderr, "Infinite loop, aborting!\n");
        fprintf(stderr, "%s\n", &cpu->disassembly[0]);
        fprintf(stderr, "\033[0m");
    }

    cpu->instruction_count++;
}

static void cpu_print_read_src(GBCPU *cpu, char *label) {
    printf("%s", &cpu->disassembly[0]);
    uint8_t value = cpu->memory[cpu->src.addr];
    printf("[8 src] Reading from %s $%04X --> 0x%02X\n", label, cpu->src.addr, value);
}

static void cpu_print_read_dst(GBCPU *cpu, char *label) {
    printf("%s", &cpu->disassembly[0]);
    uint8_t value = cpu->memory[cpu->dst.addr];
    printf("[8 dst] Reading from %s $%04X --> 0x%02X\n", label, cpu->dst.addr, value);
}

static void cpu_print_read_src_16(GBCPU *cpu, char *label) {
    printf("%s", &cpu->disassembly[0]);
    uint8_t value = cpu->memory[cpu->src.addr];
    printf("[16 src] Reading from %s $%04X --> 0x%02X\n", label, cpu->src.addr, value);
}

static void cpu_print_read_dst_16(GBCPU *cpu, char *label) {
    printf("%s", &cpu->disassembly[0]);
    uint8_t value = cpu->memory[cpu->dst.addr];
    printf("[16 dst] Reading from %s $%04X --> 0x%02X\n", label, cpu->dst.addr, value);
}

uint8_t cpu_read_from_src(GBCPU *cpu) {
    if (!cpu->src.reg) {
        if (cpu->dst.addr >= 0xE000 && cpu->src.addr < 0xFDFF) {
            cpu_print_read_src(cpu, "Echo RAM");
        } else if (cpu->src.addr == 0xFF0F) {
            cpu_print_read_src(cpu, "Interrupt Flag");
        } else if (cpu->src.addr == 0xDF7D) {
            // cpu_print_read_src(cpu, "Memory");
        } else if (cpu->src.addr == 0xDF7E) {
            // cpu_print_read_src(cpu, "Memory");
        } else if (cpu->dst.addr >= 0xFF10 && cpu->dst.addr <= 0xFF26) {
            cpu_print_read_src(cpu, "Sound Registers");
        } else if (cpu->dst.addr == 0xFF40) {
            cpu_print_read_src(cpu, "LCDC LCD Control");
        } else if (cpu->dst.addr == 0xFF41) {
            cpu_print_read_src(cpu, "STAT LCD Status");
        } else if (cpu->dst.addr == 0xFF42) {
            cpu_print_read_src(cpu, "SCY Scroll Y");
        } else if (cpu->dst.addr == 0xFF43) {
            cpu_print_read_src(cpu, "SCX Scroll X");
        } else if (cpu->dst.addr == 0xFF44) {
            cpu_print_read_src(cpu, "LY Y-Coordinate");
        } else if (cpu->dst.addr == 0xFF45) {
            cpu_print_read_src(cpu, "LYC LY Compare");
        } else if (cpu->dst.addr == 0xFF46) {
            cpu_print_read_src(cpu, "DMA Tramsfer and Start Address");
        } else if (cpu->dst.addr == 0xFF47) {
            cpu_print_read_src(cpu, "BGP Window and Palette Data");
        }
        //     if (cpu->src.addr >= 0x0000 && cpu->src.addr < 0x4000) {
        //         //printf("Reading from ROM Bank 0 $%04X\n", cpu->src.addr);
        //     } else if (cpu->src.addr >= 0x4000 && cpu->src.addr < 0x8000) {
        //         //printf("Reading from ROM Bank 1 $%04X\n", cpu->src.addr);
        //     } else if (cpu->src.addr >= 0x8000 && cpu->src.addr < 0xA000) {
        //         //printf("Reading to VRAM $%04X\n", cpu->src.addr);
        //     } else if (cpu->src.addr >= 0xFF00 && cpu->src.addr <= 0xFF7F) {
        //         if(cpu->src.addr == 0xFF01) {
        //             printf("%8zu Reading from serial transfer data    $%04X\n", cpu->instruction_count, cpu->src.addr);
        //         } else if(cpu->src.addr == 0xFF02) {
        //             printf("Reading from serial transfer control $%04X\n", cpu->src.addr);
        //         } else if(cpu->src.addr == 0xFF44) {
        //             //printf("Reading from LY 0x%02X\n", value);
        //         } else {
        //             printf("Reading from I/O Regs $%04X\n", cpu->src.addr);
        //         }
        //     } else if (cpu->src.addr >= 0xFF80 && cpu->src.addr <= 0xFFFE){
        //         // printf("Reading from HRAM $%04X\n", cpu->src.addr);
        //     } else if (cpu->src.addr == 0xFFFF){
        //         printf("Reading Interrupt Enable Register $%04X\n", cpu->src.addr);
        //     }
    }
    return *(uint8_t *)(cpu->src.ptr);
}

uint16_t cpu_read_from_src_16(GBCPU *cpu) {
    if (!cpu->src.reg) {
        if (cpu->dst.addr >= 0xE000 && cpu->dst.addr < 0xFDFF) {
            cpu_print_read_src_16(cpu, "Echo RAM");
        } else if (cpu->src.addr == 0xFF0F) {
            cpu_print_read_src_16(cpu, "Interrupt Flag");
        } else if (cpu->src.addr == 0xDF7D) {
            // cpu_print_read_src_16(cpu, "Memory");
        } else if (cpu->src.addr == 0xDF7E) {
            // cpu_print_read_src_16(cpu, "Memory");
        }
        //     if (cpu->src.addr >= 0x0000 && cpu->src.addr < 0x4000) {
        //         //printf("Reading from ROM Bank 0 $%04X\n", cpu->src.addr);
        //     } else if (cpu->src.addr >= 0x4000 && cpu->src.addr < 0x8000) {
        //         //printf("Reading from ROM Bank 1 $%04X\n", cpu->src.addr);
        //     } else if (cpu->src.addr >= 0x8000 && cpu->src.addr < 0xA000) {
        //         //printf("Reading to VRAM $%04X\n", cpu->src.addr);
        //     } else if (cpu->src.addr >= 0xFF00 && cpu->src.addr <= 0xFF7F) {
        //         if(cpu->src.addr == 0xFF01) {
        //             printf("Reading from serial transfer data    $%04X\n", cpu->src.addr);
        //         } else if(cpu->src.addr == 0xFF02) {
        //             printf("Reading from serial transfer control $%04X\n", cpu->src.addr);
        //         } else if(cpu->src.addr == 0xFF44) {
        //             //printf("Reading from LY 0x%02X\n", value);
        //         } else {
        //             printf("Reading from I/O Regs $%04X\n", cpu->src.addr);
        //         }
        //     } else if (cpu->src.addr >= 0xFF80 && cpu->src.addr <= 0xFFFE){
        //         // printf("Reading from HRAM $%04X\n", cpu->src.addr);
        //     } else if (cpu->src.addr == 0xFFFF){
        //         printf("Reading Interrupt Enable Register $%04X\n", cpu->src.addr);
        //     }
    }
    return *(uint16_t *)(cpu->src.ptr);
}

uint8_t cpu_read_from_dst(GBCPU *cpu) {
    if (!cpu->dst.reg) {
        if (cpu->dst.addr >= 0xE000 && cpu->dst.addr < 0xFDFF) {
            cpu_print_read_dst(cpu, "Echo RAM");
        } else if (cpu->src.addr == 0xFF0F) {
            cpu_print_read_dst(cpu, "Interrupt Flag");
        } else if (cpu->dst.addr >= 0xFF10 && cpu->dst.addr <= 0xFF26) {
            cpu_print_read_dst(cpu, "Sound Registers");
        } else if (cpu->dst.addr == 0xFF40) {
            cpu_print_read_dst(cpu, "LCDC LCD Control");
        } else if (cpu->dst.addr == 0xFF41) {
            cpu_print_read_dst(cpu, "STAT LCD Status");
        } else if (cpu->dst.addr == 0xFF42) {
            cpu_print_read_dst(cpu, "SCY Scroll Y");
        } else if (cpu->dst.addr == 0xFF43) {
            cpu_print_read_dst(cpu, "SCX Scroll X");
        } else if (cpu->dst.addr == 0xFF44) {
            cpu_print_read_dst(cpu, "LY Y-Coordinate");
        } else if (cpu->dst.addr == 0xFF45) {
            cpu_print_read_dst(cpu, "LYC LY Compare");
        } else if (cpu->dst.addr == 0xFF46) {
            cpu_print_read_dst(cpu, "DMA Tramsfer and Start Address");
        } else if (cpu->dst.addr == 0xFF47) {
            cpu_print_read_dst(cpu, "BGP Window and Palette Data");
        }
    }
    return *(uint8_t *)(cpu->dst.ptr);
}

uint16_t cpu_read_from_dst_16(GBCPU *cpu) {
    if (!cpu->dst.reg) {
        if (cpu->dst.addr >= 0xE000 && cpu->dst.addr < 0xFDFF) {
            cpu_print_read_dst_16(cpu, "Echo RAM");
        } else if (cpu->dst.addr == 0xDF7D) {
            // cpu_print_read_dst_16(cpu, "Memory");
        } else if (cpu->dst.addr == 0xDF7E) {
            // cpu_print_read_dst_16(cpu, "Memory");
        } else if (cpu->src.addr == 0xFF0F) {
            cpu_print_read_dst_16(cpu, "Interrupt Flag");
        }
    }

    return *(uint16_t *)(cpu->dst.ptr);
}

static void cpu_print_write(GBCPU *cpu, char *label, uint8_t value) {
    printf("%s", &cpu->disassembly[0]);
    printf("[8 dst] Writing to %s $%04X <-- 0x%02X\n", label, cpu->dst.addr, value);
}

static void cpu_print_write_16(GBCPU *cpu, char *label, uint16_t value) {
    printf("%s", &cpu->disassembly[0]);
    printf("[16 dst] Writing to %s $%04X <-- 0x%04X\n", label, cpu->dst.addr, value);
}
void cpu_write_to_dst(GBCPU *cpu, uint8_t value) {
    if (cpu->dst.reg && cpu->dst.addr > 0x0000) {
        printf("%s", &cpu->disassembly[0]);
        printf("Reg writing to memory!");
    }

    if (!cpu->dst.reg) {
        if (cpu->dst.addr >= 0x0000 && cpu->dst.addr < 0x4000) {
            cpu_print_write(cpu, "ROM Bank 0", value);
        } else if (cpu->dst.addr >= 0x4000 && cpu->dst.addr < 0x8000) {
            cpu_print_write(cpu, "ROM Bank 1", value);
        } else if (cpu->dst.addr >= 0x8000 && cpu->dst.addr <= 0x9FFF) {
            // cpu_print_write(cpu, "VRAM", value);            }
        } else if (cpu->dst.addr >= 0xA000 && cpu->dst.addr <= 0xBFFF) {
            cpu_print_write(cpu, "External RAM", value);
        } else if (cpu->dst.addr >= 0xC000 && cpu->dst.addr <= 0xDFFF) {
            // cpu_print_write(cpu, "WRAM", value);
        } else if (cpu->dst.addr >= 0xE000 && cpu->dst.addr < 0xFDFF) {
            cpu_print_write(cpu, "Echo RAM", value);

            //     } else if (cpu->dst.addr >= 0x8000 && cpu->dst.addr < 0xA000) {
            //         //printf("Writing to VRAM $%04X\n", cpu->dst.addr);
            //     } else if (cpu->dst.addr >= 0xFF00 && cpu->dst.addr <= 0xFF7F) {
            //         if(cpu->dst.addr == 0xFF01) {
            //             printf("Writing to serial transfer data    0x%02X\n", value);
            //         } else if(cpu->dst.addr == 0xFF02) {
            //             printf("Writing to serial transfer control 0x%02X\n", value);
            //         } else if(cpu->dst.addr == 0xFF44) {
            //             //printf("Writing to LY 0x%02X\n", value);
            //         } else {
            //             printf("Writing to I/O Regs $%04X\n", cpu->dst.addr);
            //         }
            //     } else if (cpu->dst.addr >= 0xFF80 && cpu->dst.addr <= 0xFFFE){
            //         // printf("Writing to HRAM $%04X\n", cpu->dst.addr);

        } else if (cpu->dst.addr >= 0xFFE0 && cpu->dst.addr <= 0xFF9F) {
            cpu_print_write(cpu, "Object Attribute Memory", value);

        } else if (cpu->dst.addr >= 0xFF00 && cpu->dst.addr <= 0xFF7F) {
            if (cpu->dst.addr == 0xFF00) {
                cpu_print_write(cpu, "P1 Joypad", value);
            } else if (cpu->dst.addr == 0xFF01) {
                // cpu_print_write(cpu, "SB Serial Transfer Data", value);
            } else if (cpu->dst.addr == 0xFF02) {
                // cpu_print_write(cpu, "SC Serial Transfer Control", value);
            } else if (cpu->dst.addr == 0xFF04) {
                // ff03 is LSB of DIV
                cpu_print_write(cpu, "DIV Divider Register", value);
            } else if (cpu->dst.addr == 0xFF05) {
                cpu_print_write(cpu, "TIMA Timer Counter", value);
            } else if (cpu->dst.addr == 0xFF06) {
                cpu_print_write(cpu, "TMA Timer Modulo", value);
            } else if (cpu->dst.addr == 0xFF07) {
                cpu_print_write(cpu, "TAC Timer Control", value);
                uint32_t timer_speed = value & 0x03;
                uint8_t timer_enabled = (value >> 2) & 0x01;

                switch (timer_speed) {
                case 0:
                    timer_speed = 4096;
                    break;
                case 1:
                    timer_speed = 262144;
                    break;
                case 2:
                    timer_speed = 65536;
                    break;
                case 3:
                    timer_speed = 16384;
                    break;
                }

                printf("Timer %s to %d\n", (timer_enabled) ? "enabled" : "disabled", timer_speed);

            } else if (cpu->dst.addr == 0xFF0F) {
                cpu_print_write(cpu, "Interrupt Flag", value);

                uint8_t flags = cpu->memory[0xFFFF];
                if (cpu->ime && ((value & flags) == value)) {
                    printf("$FF0F reset!\n");
                    cpu->memory[0xFF0F] = 0x00;
                    cpu->ime = false;
                    cpu_push(cpu, (cpu->reg.PC >> 8) & 0x00FF);
                    cpu_push(cpu, cpu->reg.PC & 0x00FF);
                    uint16_t addr;
                    switch (value) {
                    case 0x01:
                        addr = 0x0040;
                        break;
                    case 0x02:
                        addr = 0x0048;
                        break;
                    case 0x04:
                        addr = 0x0050;
                        break;
                    case 0x08:
                        addr = 0x0058;
                        break;
                    case 0x10:
                        addr = 0x0060;
                        break;
                    default:
                        printf("Unkown interrupt state 0x%02X\n", value);
                        addr = cpu->reg.PC;
                        cpu->crashed = true;
                    }

                    cpu->reg.PC = addr;
                    return;
                }
            } else if (cpu->dst.addr == 0xFF50) {
                cpu_print_write(cpu, "Boot ROM Control", value);
            } else if (cpu->dst.addr >= 0xFF10 && cpu->dst.addr <= 0xFF26) {
                cpu_print_write(cpu, "Sound Registers", value);
            } else if (cpu->dst.addr == 0xFF40) {
                cpu_print_write(cpu, "LCDC LCD Control", value);
            } else if (cpu->dst.addr == 0xFF41) {
                cpu_print_write(cpu, "STAT LCD Status", value);
            } else if (cpu->dst.addr == 0xFF42) {
                cpu_print_write(cpu, "SCY Scroll Y", value);
            } else if (cpu->dst.addr == 0xFF43) {
                cpu_print_write(cpu, "SCX Scroll X", value);
            } else if (cpu->dst.addr == 0xFF44) {
                cpu_print_write(cpu, "LY Y-Coordinate", value);
            } else if (cpu->dst.addr == 0xFF45) {
                cpu_print_write(cpu, "LYC LY Compare", value);
            } else if (cpu->dst.addr == 0xFF46) {
                cpu_print_write(cpu, "DMA Transfer and Start Address", value);
            } else if (cpu->dst.addr == 0xFF47) {
                cpu_print_write(cpu, "BGP Window and Palette Data", value);
            } else {
                cpu_print_write(cpu, "Hardware IO Registers", value);
            }
        } else if (cpu->dst.addr >= 0xFF80 && cpu->dst.addr <= 0xFFFE) {
            // cpu_print_write(cpu, "HRAM", value);
        } else if (cpu->dst.addr == 0xFFFF) {
            cpu_print_write(cpu, "Interrupt Enable", value);
        } else {
            cpu_print_write(cpu, "Unclassified", value);
        }
    }

    *((uint8_t *)cpu->dst.ptr) = value;
}

void cpu_write_to_dst_16(GBCPU *cpu, uint16_t value) {
    if (cpu->dst.reg && cpu->dst.addr > 0x0000) {
        printf("%s", &cpu->disassembly[0]);
        printf("[16] Reg writing to memory!");
    }

    if (!cpu->dst.reg) {
        // printf("%s", &cpu->disassembly[0]);
        //  printf("16-bit write but not to register $%04X\n", cpu->dst.addr);
        if (cpu->dst.addr >= 0x0000 && cpu->dst.addr < 0x4000) {
            cpu_print_write_16(cpu, "ROM Bank 0", value);
        } else if (cpu->dst.addr >= 0x4000 && cpu->dst.addr < 0x8000) {
            cpu_print_write_16(cpu, "ROM Bank 1", value);
        } else if (cpu->dst.addr >= 0xC000 && cpu->dst.addr <= 0xDFFF) {
            // cpu_print_write_16(cpu, "WRAM", value);
        } else if (cpu->dst.addr >= 0xE000 && cpu->dst.addr <= 0xFDFF) {
            cpu_print_write_16(cpu, "Echo RAM", value);
        } else if (cpu->dst.addr >= 0xFFE0 && cpu->dst.addr <= 0xFF9F) {
            cpu_print_write_16(cpu, "Object Attribute Memory", value);
        } else if (cpu->dst.addr >= 0xFF00 && cpu->dst.addr <= 0xFF7F) {
            if (cpu->dst.addr == 0xFF0F) {
                cpu_print_write_16(cpu, "Interrupt Flag", value);
            } else {
                cpu_print_write_16(cpu, "Hardware I/O Registers", value);
            }
        } else if (cpu->dst.addr == 0xFFFF) {
            cpu_print_write_16(cpu, "Interrupt Flag", value);
        } else {
            cpu_print_write_16(cpu, "Unclassified", value);
        }
    }

    *((uint8_t *)cpu->dst.ptr) = value & 0x00FF;
    *((uint8_t *)cpu->dst.ptr + 1) = (value >> 8) & 0x00FF;
}

void implied(GBCPU *cpu, DataAccess *data_access) {
    (void)cpu;
    data_access->ptr = NULL;
    data_access->addr = 0x0000;
    data_access->reg = false; // maybe?
}

void immediate(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = cpu->reg.PC;
    data_access->ptr = (char *)&cpu->memory[cpu->reg.PC];
    data_access->reg = false;
    cpu->reg.PC += 1;
}

void immediate_ptr(GBCPU *cpu, DataAccess *data_access) {
    uint8_t value = cpu->memory[cpu->reg.PC];
    cpu->reg.PC += 1;
    data_access->addr = 0xFF00 + value;
    data_access->ptr = (char *)&cpu->memory[0xFF00 + value];
    data_access->reg = false;
}

void immediate_ext(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = cpu->reg.PC;
    data_access->ptr = (char *)&cpu->memory[cpu->reg.PC];
    data_access->reg = false;
    cpu->reg.PC += 2;
}

void immediate_ext_ptr(GBCPU *cpu, DataAccess *data_access) {
    uint8_t lo = cpu->memory[cpu->reg.PC];
    uint8_t hi = cpu->memory[cpu->reg.PC + 1];
    cpu->reg.PC += 2;
    uint16_t addr = (hi << 8) | lo;

    data_access->addr = addr;
    data_access->ptr = (char *)&cpu->memory[addr];
    data_access->reg = false;
}

void reg_a(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = 0x0000;
    data_access->ptr = (char *)&cpu->reg.A;
    data_access->reg = true;
}

void reg_b(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = 0x0000;
    data_access->ptr = (char *)&cpu->reg.B;
    data_access->reg = true;
}

void reg_c(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = 0x0000;
    data_access->ptr = (char *)&cpu->reg.C;
    data_access->reg = true;
}

void reg_c_ptr(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = 0xFF00 + cpu->reg.C;
    data_access->ptr = (char *)&cpu->memory[0xFF00 + cpu->reg.C];
    data_access->reg = false;
}

void reg_d(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = 0x0000;
    data_access->ptr = (char *)&cpu->reg.D;
    data_access->reg = true;
}

void reg_e(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = 0x0000;
    data_access->ptr = (char *)&cpu->reg.E;
    data_access->reg = true;
}

void reg_h(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = 0x0000;
    data_access->ptr = (char *)&cpu->reg.H;
    data_access->reg = true;
}

void reg_l(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = 0x0000;
    data_access->ptr = (char *)&cpu->reg.L;
    data_access->reg = true;
}

void reg_af(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = 0x0000;
    data_access->ptr = (char *)&cpu->reg.AF;
    data_access->reg = true;
}

void reg_bc(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = 0x0000;
    data_access->ptr = (char *)&cpu->reg.BC;
    data_access->reg = true;
}

void reg_bc_ptr(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = cpu->reg.BC;
    data_access->ptr = (char *)&cpu->memory[cpu->reg.BC];
    data_access->reg = false;
}

void reg_de(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = 0x0000;
    data_access->ptr = (char *)&cpu->reg.DE;
    data_access->reg = true;
}

void reg_de_ptr(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = cpu->reg.DE;
    data_access->ptr = (char *)&cpu->memory[cpu->reg.DE];
    data_access->reg = false;
}

void reg_sp(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = 0x0000;
    data_access->ptr = (char *)&cpu->reg.SP;
    data_access->reg = true;
}

void reg_hl(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = 0x0000;
    data_access->ptr = (char *)&cpu->reg.HL;
    data_access->reg = true;
}

void reg_hl_ptr(GBCPU *cpu, DataAccess *data_access) {
    data_access->addr = cpu->reg.HL;
    data_access->ptr = (char *)&cpu->memory[cpu->reg.HL];
    data_access->reg = false;
}

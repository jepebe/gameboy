#include <stdio.h>

#include "gbcpu.h"

void sbc(GBCPU *cpu) {
    // Subtract n from dst with carry
    uint8_t a = cpu_read_from_dst(cpu);
    uint8_t b = cpu_read_from_src(cpu);
    uint16_t result = a + ~(b - 1) - cpu->reg.flags.c;

    cpu->reg.flags.z = ((result & 0xFF) == 0);
    cpu->reg.flags.n = 1;
    cpu->reg.flags.h = (a & 0x0f) < ((b & 0x0f) + cpu->reg.flags.c);
    cpu->reg.flags.c = a < (b + cpu->reg.flags.c);

    cpu->reg.A = result & 0xFF;
}

void sub(GBCPU *cpu) {
    // Subtract n from A
    uint8_t a = cpu->reg.A;
    uint8_t b = cpu_read_from_src(cpu);
    uint16_t result = a + ~(b - 1);

    cpu->reg.flags.z = ((result & 0xFF) == 0);
    cpu->reg.flags.n = 1;
    cpu->reg.flags.h = (a & 0x0f) < (b & 0x0f);
    cpu->reg.flags.c = a < b;

    cpu->reg.A = result & 0xFF;
}

uint8_t add(GBCPU *cpu, uint8_t a, uint8_t b, bool carry) {
    // add a to b with carry, update flags
    uint16_t result = a + b + carry;
    cpu->reg.flags.z = ((result & 0xFF) == 0);
    cpu->reg.flags.n = 0;
    cpu->reg.flags.h = (((a & 0xF) + (b & 0xF) + carry) & 0x10) == 0x10;
    cpu->reg.flags.c = result > 0xFF;
    return result & 0xFF;
}

void add_8(GBCPU *cpu) {
    // Add src to dst
    uint8_t a = cpu_read_from_dst(cpu);
    uint8_t b = cpu_read_from_src(cpu);
    uint8_t result = add(cpu, a, b, 0);
    cpu_write_to_dst(cpu, result);
}

void add_16(GBCPU *cpu) {
    // Add src to dst
    uint8_t z = cpu->reg.flags.z;
    uint16_t a = cpu_read_from_dst_16(cpu);
    uint16_t b = cpu_read_from_src_16(cpu);

    uint8_t lo = add(cpu, a & 0xFF, b & 0xFF, 0);
    uint8_t hi = add(cpu, (a >> 8) & 0xFF, (b >> 8) & 0xFF, cpu->reg.flags.c);
    cpu->reg.flags.z = z;
    cpu->reg.flags.n = 0;
    cpu_write_to_dst_16(cpu, (hi << 8) | lo);
}

uint16_t add_rel(GBCPU *cpu, uint16_t a, uint8_t b) {
    uint8_t lo = add(cpu, a & 0xFF, b, 0);
    uint8_t hi = (a >> 8) & 0xFF;

    if ((b & 0x80) == 0x80) {
        hi -= !cpu->reg.flags.c;
    } else {
        hi += cpu->reg.flags.c;
    }

    return (hi << 8) | lo;
}

void adc(GBCPU *cpu) {
    // add with carry
    uint8_t a = cpu_read_from_dst(cpu);
    uint8_t b = cpu_read_from_src(cpu);
    uint8_t result = add(cpu, a, b, cpu->reg.flags.c);

    cpu_write_to_dst(cpu, result & 0xFF);
}

void add_sp(GBCPU *cpu) {
    // Add immediate signed r8 to SP
    uint8_t r8 = cpu_read_from_src(cpu);
    uint16_t sp = cpu_read_from_dst_16(cpu);

    sp = add_rel(cpu, sp, r8);

    cpu->reg.flags.z = 0;
    cpu->reg.flags.n = 0;

    cpu_write_to_dst_16(cpu, sp);
}

void cp(GBCPU *cpu) {
    // Compare A with read value
    // USE SBC
    uint8_t value = cpu_read_from_src(cpu);
    cpu->reg.flags.n = 1;
    cpu->reg.flags.z = cpu->reg.A == value;
    cpu->reg.flags.c = cpu->reg.A < value;
    cpu->reg.flags.h = (cpu->reg.A & 0x0f) < (value & 0x0f);
}

void cpl(GBCPU *cpu) {
    cpu->reg.A = ~cpu->reg.A;
    cpu->reg.flags.h = 1;
    cpu->reg.flags.n = 1;
}

void ccf(GBCPU *cpu) {
    cpu->reg.flags.h = 0;
    cpu->reg.flags.n = 0;
    cpu->reg.flags.c = ~cpu->reg.flags.c;
}

void scf(GBCPU *cpu) {
    cpu->reg.flags.h = 0;
    cpu->reg.flags.n = 0;
    cpu->reg.flags.c = 1;
}

void daa(GBCPU *cpu) {
    uint16_t a = cpu->reg.A;
    bool carry = false;

    if (cpu->reg.flags.h) {
        if (cpu->reg.flags.n) {
            a += 0x0A;
        } else {
            a += 0x06;
        }
    } else if (!cpu->reg.flags.n && ((a & 0x0F) > 0x09)) {
        a += 0x06;
    }

    if (cpu->reg.flags.c) {
        if (cpu->reg.flags.n && cpu->reg.flags.h) {
            a += 0x90;
        } else if (cpu->reg.flags.n) {
            a += 0xA0;
        } else {
            a += 0x60;
        }
        carry = true;
    } else if (cpu->reg.flags.n && cpu->reg.flags.h) {
        a += 0xF0;
    } else if (cpu->reg.flags.n) {
        a += 0x00;
    } else if ((a & 0xFFF0) > 0x90) {
        a += 0x60;
        carry = true;
    }

    cpu->reg.flags.z = ((a & 0x00FF) == 0);
    cpu->reg.flags.h = 0;
    cpu->reg.flags.c = carry;

    cpu->reg.A = a & 0x00FF;
}

void di(GBCPU *cpu) {
    // disable interrupts
    cpu->ime = false;
}

void ei(GBCPU *cpu) {
    // enable interrupts
    cpu->ime = true;
}

void jp(GBCPU *cpu) {
    cpu->reg.PC = cpu_read_from_src_16(cpu);
}

void jp_c(GBCPU *cpu) {
    if (cpu->reg.flags.c) {
        jp(cpu);
        // cycles 16
    } else {
        // cycles 12
    }
}

void jp_z(GBCPU *cpu) {
    if (cpu->reg.flags.z) {
        jp(cpu);
        // cycles 16
    } else {
        // cycles 12
    }
}

void jp_nc(GBCPU *cpu) {
    if (!(cpu->reg.flags.c)) {
        jp(cpu);
        // cycles 16
    } else {
        // cycles 12
    }
}

void jp_nz(GBCPU *cpu) {
    if (!(cpu->reg.flags.z)) {
        jp(cpu);
        // cycles 16
    } else {
        // cycles 12
    }
}

void jr(GBCPU *cpu) {
    int8_t rel = (int8_t)cpu_read_from_src(cpu);
    cpu->reg.PC += rel;
}

void jr_c(GBCPU *cpu) {
    if (cpu->reg.flags.c) {
        jr(cpu);
        // cycles 12
    } else {
        // cycles 7
    }
}

void jr_nc(GBCPU *cpu) {
    if (!(cpu->reg.flags.c)) {
        jr(cpu);
        // cycles 12
    } else {
        // cycles 7
    }
}

void jr_nz(GBCPU *cpu) {
    if (!(cpu->reg.flags.z)) {
        jr(cpu);
        // cycles 12
    } else {
        // cycles 7
    }
}

void jr_z(GBCPU *cpu) {
    if (cpu->reg.flags.z) {
        jr(cpu);
        // cycles 12
    } else {
        // cycles 7
    }
}

void ld_16(GBCPU *cpu) {
    uint16_t value = cpu_read_from_src_16(cpu);
    cpu_write_to_dst_16(cpu, value);
}

void ld_8(GBCPU *cpu) {
    uint8_t val = cpu_read_from_src(cpu);
    cpu_write_to_dst(cpu, val);
}

void ldi(GBCPU *cpu) {
    uint8_t val = cpu_read_from_src(cpu);
    cpu_write_to_dst(cpu, val);
    cpu->reg.HL++;
}

void ldd(GBCPU *cpu) {
    uint8_t val = cpu_read_from_src(cpu);
    cpu_write_to_dst(cpu, val);
    cpu->reg.HL--;
}

void ldhl(GBCPU *cpu) {
    uint16_t sp = cpu_read_from_dst_16(cpu);
    int8_t rel = cpu_read_from_src(cpu);

    sp = add_rel(cpu, sp, rel);
    cpu->reg.HL = sp;
    cpu->reg.flags.z = 0;
    cpu->reg.flags.n = 0;
}

void nop(GBCPU *cpu) {
    (void)cpu;
}

void inc_16(GBCPU *cpu) {
    uint16_t val = cpu_read_from_dst_16(cpu);
    cpu_write_to_dst_16(cpu, ++val);
}

void inc_8(GBCPU *cpu) {
    bool carry = cpu->reg.flags.c;
    uint8_t val = cpu_read_from_dst(cpu);
    val = add(cpu, val, 1, false);
    cpu->reg.flags.c = carry;
    cpu_write_to_dst(cpu, val);
}

void dec_8(GBCPU *cpu) {
    uint8_t val = cpu_read_from_dst(cpu); //*((uint8_t*)(cpu->dst));
    val--;

    cpu->reg.flags.z = val == 0;
    cpu->reg.flags.n = 1;
    cpu->reg.flags.h = ((val + 1) & 0x0f) < (val & 0x0f);
    cpu_write_to_dst(cpu, val);
}

void dec_16(GBCPU *cpu) {
    uint16_t val = cpu_read_from_dst_16(cpu);
    cpu_write_to_dst_16(cpu, --val);
}

void or (GBCPU * cpu) {
    uint8_t value = cpu_read_from_src(cpu);
    value = cpu->reg.A | value;
    cpu->reg.A = value;
    cpu->reg.F = 0x00;
    cpu->reg.flags.z = (value == 0);
}

void and (GBCPU * cpu) {
    uint8_t value = cpu_read_from_src(cpu);
    value = cpu->reg.A & value;
    cpu->reg.A = value;
    cpu->reg.F = 0x00;
    cpu->reg.flags.z = (value == 0);
    cpu->reg.flags.h = 1;
}

void xor (GBCPU * cpu) {
    uint8_t value = cpu_read_from_src(cpu);
    value = cpu->reg.A ^ value;
    cpu->reg.A = value;
    cpu->reg.F = 0x00;
    cpu->reg.flags.z = (value == 0);
}

    void rla(GBCPU *cpu) {
    // rotate A left insert carry at bit 0, new carry is msb
    uint8_t value = cpu->reg.A;
    uint8_t carry = cpu->reg.flags.c;
    cpu->reg.F = 0x00;
    cpu->reg.flags.c = (value & 0x80) == 0x80;

    value = (value << 1) | carry;
    cpu->reg.A = value;
}

void rlca(GBCPU *cpu) {
    // rotate A left insert msb at bit 0, new carry is msb
    uint8_t value = cpu->reg.A;
    cpu->reg.F = 0x00;
    cpu->reg.flags.c = (value & 0x80) == 0x80;

    value = (value << 1) | cpu->reg.flags.c;
    cpu->reg.A = value;
}

void rra(GBCPU *cpu) {
    // rotate A right insert carry at bit 7, new carry is lsb
    uint8_t value = cpu->reg.A;
    uint8_t carry = cpu->reg.flags.c;
    cpu->reg.F = 0x00;
    cpu->reg.flags.c = value & 0x01;

    value = (carry << 7) | (value >> 1);
    cpu->reg.A = value;
}

void rrca(GBCPU *cpu) {
    // rotate A right insert lsb at bit 7, new carry is lsb
    uint8_t value = cpu->reg.A;
    cpu->reg.F = 0x00;
    cpu->reg.flags.c = value & 0x01;

    value = (cpu->reg.flags.c << 7) | (value >> 1);
    cpu->reg.A = value;
}

void call(GBCPU *cpu) {
    cpu_push(cpu, (cpu->reg.PC >> 8) & 0x00FF);
    cpu_push(cpu, cpu->reg.PC & 0x00FF);

    cpu->reg.PC = cpu_read_from_src_16(cpu);
}

void call_nz(GBCPU *cpu) {
    if (!(cpu->reg.flags.z)) {
        call(cpu);
    } else {
        // 12
    }
}

void call_z(GBCPU *cpu) {
    if (cpu->reg.flags.z) {
        call(cpu);
    } else {
        // 12
    }
}

void call_nc(GBCPU *cpu) {
    if (!(cpu->reg.flags.c)) {
        call(cpu);
    } else {
        // 12
    }
}

void call_c(GBCPU *cpu) {
    if (cpu->reg.flags.c) {
        call(cpu);
    } else {
        // 12
    }
}

void rst(GBCPU *cpu) {
    // Call Page 0 memory location
    uint8_t t = (cpu->opcode >> 3) & 0x07;
    uint8_t p = 0x08 * t;

    cpu_push(cpu, (cpu->reg.PC >> 8) & 0x00FF);
    cpu_push(cpu, cpu->reg.PC & 0x00FF);

    cpu->reg.PC = (0x00 << 8) | p;
}

void push(GBCPU *cpu) {
    uint16_t value = cpu_read_from_src_16(cpu);

    cpu_push(cpu, (value >> 8) & 0x00FF);
    cpu_push(cpu, value & 0x00FF);
}

void pop(GBCPU *cpu) {
    uint8_t lo = cpu_pop(cpu);
    uint8_t hi = cpu_pop(cpu);

    uint16_t value = (hi << 8) | lo;

    if ((cpu->opcode & 0x30) == 0x30) {
        // AF register set bottom nibble of flags to 0
        value = value & 0xFFF0;
    }
    cpu_write_to_dst_16(cpu, value);
}

void ret(GBCPU *cpu) {
    uint8_t lo = cpu_pop(cpu);
    uint8_t hi = cpu_pop(cpu);

    cpu->reg.PC = (hi << 8) | lo;
}

void reti(GBCPU *cpu) {
    ret(cpu);
    ei(cpu);
}

void ret_c(GBCPU *cpu) {
    if (cpu->reg.flags.c) {
        ret(cpu);
    } else {
        // 8
    }
}

void ret_z(GBCPU *cpu) {
    if (cpu->reg.flags.z) {
        ret(cpu);
    } else {
        // 8
    }
}

void ret_nc(GBCPU *cpu) {
    if (!cpu->reg.flags.c) {
        ret(cpu);
    } else {
        // 8
    }
}

void ret_nz(GBCPU *cpu) {
    if (!cpu->reg.flags.z) {
        ret(cpu);
    } else {
        // 8
    }
}

void stop(GBCPU *cpu) {
    printf("Stopped at $%04X\n", cpu->reg.PC);
}

void bit(GBCPU *cpu) {
    uint8_t bit_num = (cpu->opcode >> 3) & 0x07;
    uint8_t value = cpu_read_from_dst(cpu);
    uint8_t res = value & (0x01 << bit_num);

    cpu->reg.flags.z = (res == 0);
    cpu->reg.flags.n = 0;
    cpu->reg.flags.h = 1;
}

void res(GBCPU *cpu) {
    uint8_t bit_num = (cpu->opcode >> 3) & 0x07;
    uint8_t value = cpu_read_from_dst(cpu);
    uint8_t res = value & ~(0x01 << bit_num);
    cpu_write_to_dst(cpu, res);
}

void set(GBCPU *cpu) {
    uint8_t bit_num = (cpu->opcode >> 3) & 0x07;
    uint8_t value = cpu_read_from_dst(cpu);
    uint8_t res = value | (0x01 << bit_num);
    cpu_write_to_dst(cpu, res);
}

void srl(GBCPU *cpu) {
    uint8_t value = cpu_read_from_dst(cpu);

    cpu->reg.flags.c = value & 0x01;

    value = value >> 1;
    cpu->reg.flags.z = (value == 0);
    cpu->reg.flags.n = 0;
    cpu->reg.flags.h = 0;
    cpu_write_to_dst(cpu, value);
}

void sla(GBCPU *cpu) {
    // shift left, bit 7 -> carry
    uint8_t value = cpu_read_from_dst(cpu);
    cpu->reg.flags.c = (value & 0x80) == 0x80;

    value = value << 1;
    cpu->reg.flags.z = (value == 0);
    cpu->reg.flags.n = 0;
    cpu->reg.flags.h = 0;
    cpu_write_to_dst(cpu, value);
}

void sra(GBCPU *cpu) {
    // shift right, bit 0 -> carry
    uint8_t value = cpu_read_from_dst(cpu);
    uint8_t bit_7 = value & 0x80;
    cpu->reg.flags.c = value & 0x01;

    value = bit_7 | (value >> 1);
    cpu->reg.flags.z = (value == 0);
    cpu->reg.flags.n = 0;
    cpu->reg.flags.h = 0;
    cpu_write_to_dst(cpu, value);
}

void rr(GBCPU *cpu) {
    // rotate right through carry, LSB -> carry
    uint8_t value = cpu_read_from_dst(cpu);
    uint8_t carry = cpu->reg.flags.c;
    cpu->reg.flags.c = value & 0x01;

    value = (carry << 7) | value >> 1;
    cpu->reg.flags.z = (value == 0);
    cpu->reg.flags.n = 0;
    cpu->reg.flags.h = 0;
    cpu_write_to_dst(cpu, value);
}

void rrc(GBCPU *cpu) {
    // rotate right LSB -> carry
    uint8_t value = cpu_read_from_dst(cpu);
    cpu->reg.flags.c = value & 0x01;
    value = (cpu->reg.flags.c << 7) | value >> 1;
    cpu->reg.flags.z = (value == 0);
    cpu->reg.flags.n = 0;
    cpu->reg.flags.h = 0;
    cpu_write_to_dst(cpu, value);
}

void rl(GBCPU *cpu) {
    uint8_t value = cpu_read_from_dst(cpu);
    uint8_t carry = cpu->reg.flags.c;
    cpu->reg.flags.c = (value & 0x80) == 0x80;

    value = (value << 1) | carry;
    cpu->reg.flags.z = (value == 0);
    cpu->reg.flags.n = 0;
    cpu->reg.flags.h = 0;
    cpu_write_to_dst(cpu, value);
}

void rlc(GBCPU *cpu) {
    uint8_t value = cpu_read_from_dst(cpu);
    cpu->reg.flags.c = (value & 0x80) == 0x80;

    value = (value << 1) | cpu->reg.flags.c;
    cpu->reg.flags.z = (value == 0);
    cpu->reg.flags.n = 0;
    cpu->reg.flags.h = 0;
    cpu_write_to_dst(cpu, value);
}

void swap(GBCPU *cpu) {
    uint8_t value = cpu_read_from_dst(cpu);
    value = ((value & 0x0F) << 4 | (value & 0xF0) >> 4);

    cpu->reg.flags.z = (value == 0);
    cpu->reg.flags.n = 0;
    cpu->reg.flags.h = 0;
    cpu->reg.flags.c = 0;
    cpu_write_to_dst(cpu, value);
}

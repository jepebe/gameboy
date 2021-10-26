#pragma once
#include <stdint.h>

#include "tools.h"

typedef struct {
    uint16_t not_used : 4;
    uint16_t c : 1; // carry
    uint16_t h : 1; // half carry
    uint16_t n : 1; // subtract
    uint16_t z : 1; // zero
    uint16_t areg : 8;
} Flags;

typedef struct {
    union {
        uint16_t AF;
        struct __attribute__((__packed__)) {
            uint8_t F;
            uint8_t A;
        };
        Flags flags;
    };
    union __attribute__((__packed__)) {
        uint16_t BC;
        struct {
            uint8_t C;
            uint8_t B;
        };
    };
    union __attribute__((__packed__)) {
        uint16_t DE;
        struct {
            uint8_t E;
            uint8_t D;
        };
    };
    union __attribute__((__packed__)) {
        uint16_t HL;
        struct {
            uint8_t L;
            uint8_t H;
        };
    };
    uint16_t SP;
    uint16_t PC;
} cpu_registers;

typedef struct {
    char *ptr;
    uint16_t addr;
    bool reg;
} DataAccess;

typedef struct {
    uint8_t memory[0x10000];
    cpu_registers reg;
    bool ime;
    size_t instruction_count;
    uint8_t opcode;
    DataAccess src;
    DataAccess dst;
    char debug[100];
    char disassembly[100];
    bool crashed;
    SerialBuffer buffer;
} GBCPU;

typedef void (*Instruction)(GBCPU *cpu);
typedef void (*AddrModeFunc)(GBCPU *cpu, DataAccess *data_access);

typedef struct {
    AddrModeFunc addr_mode_func;
    char *repr;
} AddrMode;

typedef struct {
    Instruction instruction;
    AddrMode read_mode;
    AddrMode write_mode;
    uint8_t length;
    uint8_t cycles;
    char *name;

} OpInstr;

void implied(GBCPU *cpu, DataAccess *data_access);
void immediate(GBCPU *cpu, DataAccess *data_access);
void immediate_ptr(GBCPU *cpu, DataAccess *data_access);
void immediate_ext(GBCPU *cpu, DataAccess *data_access);
void immediate_ext_ptr(GBCPU *cpu, DataAccess *data_access);

void reg_a(GBCPU *cpu, DataAccess *data_access);
void reg_b(GBCPU *cpu, DataAccess *data_access);
void reg_c(GBCPU *cpu, DataAccess *data_access);
void reg_c_ptr(GBCPU *cpu, DataAccess *data_access);
void reg_d(GBCPU *cpu, DataAccess *data_access);
void reg_e(GBCPU *cpu, DataAccess *data_access);
void reg_h(GBCPU *cpu, DataAccess *data_access);
void reg_l(GBCPU *cpu, DataAccess *data_access);
void reg_af(GBCPU *cpu, DataAccess *data_access);
void reg_bc(GBCPU *cpu, DataAccess *data_access);
void reg_bc_ptr(GBCPU *cpu, DataAccess *data_access);
void reg_de(GBCPU *cpu, DataAccess *data_access);
void reg_de_ptr(GBCPU *cpu, DataAccess *data_access);
void reg_hl(GBCPU *cpu, DataAccess *data_access);
void reg_hl_ptr(GBCPU *cpu, DataAccess *data_access);
void reg_sp(GBCPU *cpu, DataAccess *data_access);

#define IMPLIED \
    { .addr_mode_func = implied, .repr = "" }

#define IMMEDIATE \
    { .addr_mode_func = immediate, .repr = "n" }

#define IMMEDIATE_PTR \
    { .addr_mode_func = immediate_ptr, .repr = "($FF00+n)" }

#define IMMEDIATE_EXT \
    { .addr_mode_func = immediate_ext, .repr = "nn" }

#define IMMEDIATE_EXT_PTR \
    { .addr_mode_func = immediate_ext_ptr, .repr = "(nn)" }

#define REG_A \
    { .addr_mode_func = reg_a, .repr = "A" }

#define REG_B \
    { .addr_mode_func = reg_b, .repr = "B" }

#define REG_C \
    { .addr_mode_func = reg_c, .repr = "C" }

#define REG_C_PTR \
    { .addr_mode_func = reg_c_ptr, .repr = "($FF00+C)" }

#define REG_D \
    { .addr_mode_func = reg_d, .repr = "D" }

#define REG_E \
    { .addr_mode_func = reg_e, .repr = "E" }

#define REG_H \
    { .addr_mode_func = reg_h, .repr = "H" }

#define REG_L \
    { .addr_mode_func = reg_l, .repr = "L" }

#define REG_AF \
    { .addr_mode_func = reg_af, .repr = "AF" }

#define REG_BC \
    { .addr_mode_func = reg_bc, .repr = "BC" }

#define REG_BC_PTR \
    { .addr_mode_func = reg_bc_ptr, .repr = "(BC)" }

#define REG_DE \
    { .addr_mode_func = reg_de, .repr = "DE" }

#define REG_DE_PTR \
    { .addr_mode_func = reg_de_ptr, .repr = "(DE)" }

#define REG_HL \
    { .addr_mode_func = reg_hl, .repr = "HL" }

#define REG_HL_PTR \
    { .addr_mode_func = reg_hl_ptr, .repr = "(HL)" }

#define REG_SP \
    { .addr_mode_func = reg_sp, .repr = "SP" }

void cpu_initialize(GBCPU *cpu);
void cpu_reset(GBCPU *cpu);
uint8_t cpu_read(GBCPU *cpu);
void cpu_clock(GBCPU *cpu, bool debug, bool disassembly);
void cpu_push(GBCPU *cpu, uint8_t value);
uint8_t cpu_pop(GBCPU *cpu);

uint8_t cpu_read_from_src(GBCPU *cpu);
uint8_t cpu_read_from_dst(GBCPU *cpu);
uint16_t cpu_read_from_src_16(GBCPU *cpu);
uint16_t cpu_read_from_dst_16(GBCPU *cpu);
void cpu_write_to_dst(GBCPU *cpu, uint8_t value);
void cpu_write_to_dst_16(GBCPU *cpu, uint16_t value);

uint8_t add(GBCPU *cpu, uint8_t a, uint8_t b, bool carry);
void adc(GBCPU *cpu);
void add_8(GBCPU *cpu);
void add_16(GBCPU *cpu);
void add_sp(GBCPU *cpu);
void sub(GBCPU *cpu);
void sbc(GBCPU *cpu);
void cp(GBCPU *cpu);
void cpl(GBCPU *cpu);
void ccf(GBCPU *cpu);
void scf(GBCPU *cpu);
void daa(GBCPU *cpu);
void di(GBCPU *cpu);
void ei(GBCPU *cpu);
void jp(GBCPU *cpu);
void jp_c(GBCPU *cpu);
void jp_nc(GBCPU *cpu);
void jp_nz(GBCPU *cpu);
void jp_z(GBCPU *cpu);
void jr(GBCPU *cpu);
void jr_c(GBCPU *cpu);
void jr_nz(GBCPU *cpu);
void jr_nc(GBCPU *cpu);
void jr_z(GBCPU *cpu);
void ld_8(GBCPU *cpu);
void ld_16(GBCPU *cpu);
void ldd(GBCPU *cpu);
void ldi(GBCPU *cpu);
void ldhl(GBCPU *cpu);
void nop(GBCPU *cpu);
void call(GBCPU *cpu);
void call_nc(GBCPU *cpu);
void call_nz(GBCPU *cpu);
void call_z(GBCPU *cpu);
void call_c(GBCPU *cpu);
void ret(GBCPU *cpu);
void reti(GBCPU *cpu);
void ret_c(GBCPU *cpu);
void ret_z(GBCPU *cpu);
void ret_nc(GBCPU *cpu);
void ret_nz(GBCPU *cpu);
void rst(GBCPU *cpu);
void push(GBCPU *cpu);
void pop(GBCPU *cpu);
void inc_16(GBCPU *cpu);
void inc_8(GBCPU *cpu);
void dec_8(GBCPU *cpu);
void dec_16(GBCPU *cpu);
void or (GBCPU * cpu);
void and (GBCPU * cpu);
void xor (GBCPU * cpu);
void rla(GBCPU *cpu);
void rlca(GBCPU *cpu);
void rra(GBCPU *cpu);
void rrca(GBCPU *cpu);
void stop(GBCPU *cpu);

// CB Prefix
void bit(GBCPU *cpu);
void res(GBCPU *cpu);
void set(GBCPU *cpu);
void srl(GBCPU *cpu);
void sla(GBCPU *cpu);
void sra(GBCPU *cpu);
void rr(GBCPU *cpu);
void rrc(GBCPU *cpu);
void rl(GBCPU *cpu);
void rlc(GBCPU *cpu);
void swap(GBCPU *cpu);

const OpInstr opcodes[256];
const OpInstr prefix_opcodes[256];

#include <stdint.h>

typedef struct {
    uint16_t not_used : 4;
    uint16_t c : 1;
    uint16_t h : 1;
    uint16_t n : 1;
    uint16_t z : 1;
    uint16_t areg : 8;
} Flags;

typedef struct {
    union {
        uint16_t AF;
        struct {
            uint16_t F : 8;
            uint16_t A : 8;
        };
        Flags flags;
    };
    union {
        uint16_t BC;
        struct {
            uint16_t C : 8;
            uint16_t B : 8;
        };
    };
    union {
        uint16_t DE;
        struct {
            uint16_t E : 8;
            uint16_t D : 8;
        };
    };
    union {
        uint16_t HL;
        struct {
            uint16_t L : 8;
            uint16_t H : 8;
        };
    };
    uint16_t SP;
    uint16_t PC;
} cpu_registers;

typedef struct {
    uint8_t memory[0x10000];
    cpu_registers reg;
    uint16_t addr_abs;

} GBCPU;

typedef void (*Instruction)(GBCPU *cpu); 
typedef void (*AddressingMode)(GBCPU *cpu); 
typedef struct {
    Instruction instruction;
    AddressingMode addr_mode;
    uint8_t length;
    uint8_t cycles;
    char* name;

} OpInstr;

void cpu_initialize(GBCPU *cpu);
void cpu_reset(GBCPU *cpu);
uint8_t cpu_read(GBCPU *cpu);
void cpu_clock(GBCPU *cpu);

uint8_t bus_read(GBCPU *cpu, uint16_t addr);

void implied(GBCPU * cpu);
void implied_sp(GBCPU * cpu);
void immediate_ext(GBCPU *cpu);

void di(GBCPU *cpu);
void jp(GBCPU *cpu);
void ld_16(GBCPU *cpu);
void nop(GBCPU *cpu);

const OpInstr opcodes[256];

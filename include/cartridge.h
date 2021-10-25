#pragma once
#include <stdint.h>

typedef enum {
    ROM_ONLY = 0x00,
    ROM_MBC1 = 0x01,
    ROM_MBC1_RAM = 0x02,
    ROM_MBC1_RAM_BATT = 0x03,
    ROM_MBC2 = 0x05,
    ROM_MBC2_BATTERY = 0x06,
    ROM_RAM = 0x08,
    ROM_RAM_BATTERY = 0x09,
    ROM_MMM01 = 0x0B,
    ROM_MMM01_SRAM = 0x0C,
    ROM_MMM01_SRAM_BATT = 0x0D,
    ROM_MBC3_TIMER_BATT = 0x0F,
    ROM_MBC3_TIMER_RAM_BATT = 0x10,
    ROM_MBC30148 = 0x11,
    ROM_MBC3_RAM = 0x12,
    ROM_MBC3_RAM_BATT = 0x13,
    ROM_MBC5 = 0x19,
    ROM_MBC5_RAM = 0x1A,
    ROM_MBC5_RAM_BATT = 0x1B,
    ROM_MBC5_RUMBLE = 0x1C,
    ROM_MBC5_RUMBLE_SRAM = 0x1D,
    ROM_MBC5_RUMBLE_SRAM_BATT = 0x1E,
    POCKET_CAMERA = 0x1F,
    BANDAI_TAMA5 = 0xFD,
    HUDSON_HUC_3 = 0xFE,
    HUDSON_HUC_1 = 0xFF,
} CartridgeType;

typedef enum {
    ROM_32_KB = 0x00,
    ROM_64_KB = 0x01,
    ROM_128_KB = 0x02,
    ROM_256_KB = 0x03,
    ROM_512_KB = 0x04,
    ROM_1024_KB = 0x05,
    ROM_2048_KB = 0x06,
    ROM_4096_KB = 0x07,
    ROM_8192_KB = 0x08,
    ROM_1152_KB = 0x52,
    ROM_1280_KB = 0x53,
    ROM_1536_KB = 0x54,
} RomSize;

typedef enum {
    JAPANESE = 0,
    NON_JAPANESE = 1,
} DestinationCode;

typedef enum {
    RAM_NONE = 0x00,
    RAM_2_KB = 0x01,
    RAM_8_KB = 0x02,
    RAM_32_KB = 0x03,
    RAM_128_KB = 0x04,
    RAM_64_KB = 0x04,
} RamSize;

typedef struct {
    uint8_t start[0x04];
    uint8_t nintendo[0x30];
    char title[0x11];  // 16 characters + null termination
    //uint8_t gb_type;
    uint8_t lic_code_high;
    uint8_t lic_code_low;
    uint8_t gb_sgb_indicator;
    CartridgeType cartridge_type;
    RomSize rom_size;
    RamSize ram_size;
    DestinationCode destination_code;
    uint8_t lic_code_old;
    uint8_t mask_rom_version; // typically 0x00
    uint8_t complement_check;
    uint16_t checksum;
} Cartridge;

const Cartridge* cartridge_allocate(const uint8_t * memory);
uint8_t cartridge_header_checksum(const uint8_t * memory);

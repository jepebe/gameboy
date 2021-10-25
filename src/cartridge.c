#include <cartridge.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


size_t cartridge_ram_size(RamSize ram_size) {
    switch(ram_size) {
            case RAM_NONE:
            return 0;
        case RAM_2_KB:
            return 0x0800;
        case RAM_8_KB:
            return 0x2000;
        case RAM_32_KB:
            return 0x8000;
        case RAM_128_KB:
            return 0x20000;
        case RAM_64_KB:
            return 0x10000;
        default:
            fprintf(stderr, "Unknown RamSize enum value: %d\n", ram_size);
            return 0;
    }
}

size_t cartridge_ram_bank_count(RamSize ram_size) {
    size_t ram = cartridge_ram_size(ram_size);
    if (ram > 0) {
        return ram / 0x2000;
    }
    return 0;
}

size_t cartridge_rom_size(RomSize rom_size) {
    switch(rom_size) {
        case ROM_32_KB:
            return 0x8000;
        case ROM_64_KB:
            return 0x10000;
        case ROM_128_KB:
            return 0x20000;
        case ROM_256_KB:
            return 0x40000;
        case ROM_512_KB:
            return 0x80000;
        case ROM_1024_KB:
            return 0x100000;
        case ROM_2048_KB:
            return 0x200000;
        case ROM_4096_KB:
            return 0x400000;
        case ROM_8192_KB:
            return 0x800000;
        case ROM_1152_KB:
            return 0x120000;
        case ROM_1280_KB:
            return 0x140000;
        case ROM_1536_KB:
            return 0x18000;
        default:
            fprintf(stderr, "Unknown RomSize enum value: %d\n", rom_size);
            return 0;
    }
}

size_t cartridge_rom_bank_count(RomSize rom_size) {
    size_t rom = cartridge_rom_size(rom_size);
    if (rom > 0) {
        return rom / 0x2000;
    }
    return 0;
}

const char * cartridge_type_as_string(CartridgeType type) {
    switch(type) {
        case ROM_ONLY:
            return "ROM_ONLY";
        case ROM_MBC1:
            return "ROM_MBC1";
        case ROM_MBC1_RAM:
            return "ROM_MBC1_RAM";
        case ROM_MBC1_RAM_BATT:
            return "ROM_MBC1_RAM_BATT";
        case ROM_MBC2:
            return "ROM_MBC2";
        case ROM_MBC2_BATTERY:
            return "ROM_MBC2_BATTERY";
        case ROM_RAM:
            return "ROM_RAM";
        case ROM_RAM_BATTERY:
            return "ROM_RAM_BATTERY";
        case ROM_MMM01:
            return "ROM_MMM01";
        case ROM_MMM01_SRAM:
            return "ROM_MMM01_SRAM";
        case ROM_MMM01_SRAM_BATT:
            return "ROM_MMM01_SRAM_BATT";
        case ROM_MBC3_TIMER_BATT:
            return "ROM_MBC3_TIMER_BATT";
        case ROM_MBC3_TIMER_RAM_BATT:
            return "ROM_MBC3_TIMER_RAM_BATT";
        case ROM_MBC30148:
            return "ROM_MBC30148";
        case ROM_MBC3_RAM:
            return "ROM_MBC3_RAM";
        case ROM_MBC3_RAM_BATT:
            return "ROM_MBC3_RAM_BATT";
        case ROM_MBC5:
            return "ROM_MBC5";
        case ROM_MBC5_RAM:
            return "ROM_MBC5_RAM";
        case ROM_MBC5_RAM_BATT:
            return "ROM_MBC5_RAM_BATT";
        case ROM_MBC5_RUMBLE:
            return "ROM_MBC5_RUMBLE";
        case ROM_MBC5_RUMBLE_SRAM:
            return "ROM_MBC5_RUMBLE_SRAM";
        case ROM_MBC5_RUMBLE_SRAM_BATT:
            return "ROM_MBC5_RUMBLE_SRAM_BATT";
        case POCKET_CAMERA:
            return "POCKET_CAMERA";
        case BANDAI_TAMA5:
            return "BANDAI_TAMA5";
        case HUDSON_HUC_3:
            return "HUDSON_HUC_3";
        case HUDSON_HUC_1:
            return "HUDSON_HUC_1";
        default:
            fprintf(stderr, "Unknown CartridgeType enum value: %d\n", type);
            return 0;
    }
}

const Cartridge* cartridge_allocate(const uint8_t * memory) {
    Cartridge* cartridge = (Cartridge*) malloc(sizeof(Cartridge));
    memcpy(cartridge->start, &memory[0x0100], 4);
    memcpy(cartridge->nintendo, &memory[0x0104], 48);
    memcpy(cartridge->title, &memory[0x0134], 16);
    cartridge->title[16] = 0x00;
    //cartridge->gb_type = memory[0x0143];
    cartridge->lic_code_high = memory[0x0144];
    cartridge->lic_code_low = memory[0x0145];
    cartridge->gb_sgb_indicator = memory[0x0146];
    cartridge->cartridge_type = (CartridgeType) memory[0x0147];
    cartridge->rom_size = (RomSize) memory[0x0148];
    cartridge->ram_size = (RamSize) memory[0x0149];
    cartridge->destination_code = (DestinationCode) memory[0x014A];
    cartridge->lic_code_old = memory[0x014B];
    cartridge->mask_rom_version = memory[0x014C];
    cartridge->complement_check = memory[0x014D];
    uint8_t high = memory[0x014E];
    uint8_t low = memory[0x014F];
    cartridge->checksum = (high << 8) | low;
    
    return cartridge;
}

uint8_t cartridge_header_checksum(const uint8_t * memory) {
    uint32_t x = 0;
    for (size_t i = 0x0134; i <= 0x014C; ++i) {
        x = x - memory[i] - 1;
    }
    return x & 0x000000FF;
}

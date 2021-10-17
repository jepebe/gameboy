#include <cartridge.h>
#include <string.h>
#include <stdlib.h>


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

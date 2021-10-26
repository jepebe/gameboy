#include "acutest.h"
#include "cartridge.h"
#include "gbcpu.h"

static void print_rom(char *rom_file) {
    GBCPU cpu;
    cpu_initialize(&cpu);
    read_binary(rom_file, cpu.memory);

    const Cartridge *cartridge = cartridge_allocate(cpu.memory);

    printf("Title: %s\n", cartridge->title);
    printf("Cartridge Type: %s\n", cartridge_type_as_string(cartridge->cartridge_type));
    printf("ROM Size: %zu\n", cartridge_rom_size(cartridge->rom_size));
    printf("RAM Size: %zu\n", cartridge_ram_size(cartridge->ram_size));

    free((void *)cartridge);
}

void test_cartridge_functions() {
    TEST_CHECK(cartridge_ram_size(RAM_32_KB) == 0x8000);
    TEST_CHECK(cartridge_ram_size(9) == 0);

    TEST_CHECK(cartridge_ram_bank_count(RAM_NONE) == 0);
    TEST_CHECK(cartridge_ram_bank_count(RAM_2_KB) == 0);
    TEST_CHECK(cartridge_ram_bank_count(RAM_8_KB) == 1);
    TEST_CHECK(cartridge_ram_bank_count(RAM_32_KB) == 4);
    TEST_CHECK(cartridge_ram_bank_count(RAM_64_KB) == 8);
    TEST_CHECK(cartridge_ram_bank_count(RAM_128_KB) == 16);

    TEST_CHECK(cartridge_ram_bank_count(9) == 0);

    TEST_CHECK(cartridge_rom_size(ROM_32_KB) == 0x8000);
    TEST_CHECK(cartridge_rom_size(13) == 0);

    TEST_CHECK(cartridge_rom_bank_count(ROM_32_KB) == 4);
    TEST_CHECK(cartridge_rom_bank_count(ROM_64_KB) == 8);
    TEST_CHECK(cartridge_rom_bank_count(ROM_128_KB) == 16);
    TEST_CHECK(cartridge_rom_bank_count(ROM_256_KB) == 32);
    TEST_CHECK(cartridge_rom_bank_count(ROM_512_KB) == 64);
    TEST_CHECK(cartridge_rom_bank_count(ROM_1024_KB) == 128);
    TEST_CHECK(cartridge_rom_bank_count(ROM_2048_KB) == 256);
    TEST_CHECK(cartridge_rom_bank_count(11) == 0);
}

void test_game_roms() {
    print_rom("../roms/Dr. Mario (World).gb");
    print_rom("../roms/Kirby's Dream Land (USA, Europe).gb");
    print_rom("../roms/Tetris (World) (Rev A).gb");
}

TEST_LIST = {
    {"Test Cartridge Header Functions", test_cartridge_functions},
    {"Check Game ROMs", test_game_roms},
    {NULL, NULL}};

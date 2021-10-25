#include "acutest.h"
#include "cartridge.h"
#include "gbcpu.h"
#include "tools.h"

void test_serial_buffer() {
    SerialBuffer serial_buffer;
    serial_buffer_clear(&serial_buffer);
    
    TEST_CHECK(serial_buffer.pos == 0);
    TEST_CHECK(serial_buffer_push(&serial_buffer, 'H'));
    
    TEST_CHECK(serial_buffer.pos == 1);
    serial_buffer_push(&serial_buffer, 'i');

    TEST_CHECK(serial_buffer.pos == 2);
    serial_buffer_push(&serial_buffer, '!');

    TEST_CHECK(serial_buffer.pos == 3);
    serial_buffer_push(&serial_buffer, '\n');

    TEST_CHECK(serial_buffer_eol(&serial_buffer));

    TEST_CHECK(strcmp("Hi!\n", &serial_buffer.buffer[0]) == 0);

    serial_buffer_clear(&serial_buffer);

    TEST_CHECK(!serial_buffer_eol(&serial_buffer));

    for(size_t i = 0; i < SERIAL_BUFFER_SIZE - 1; ++i) {
        TEST_CHECK(serial_buffer_push(&serial_buffer, 'x'));
    }

    TEST_CHECK(!serial_buffer_push(&serial_buffer, 'x'));

    TEST_CHECK(serial_buffer.pos == SERIAL_BUFFER_SIZE - 1);
    TEST_CHECK(serial_buffer.buffer[SERIAL_BUFFER_SIZE - 2] == 'x');
    TEST_CHECK(serial_buffer.buffer[SERIAL_BUFFER_SIZE - 1] == '\0');
}

void test_stack() {
    Stack* stack = stack_allocate();
    TEST_CHECK(stack_peek(stack) == 0x00);
    stack_push(stack, 0xA5);
    TEST_CHECK(stack_peek(stack) == 0xA5);
    stack_push(stack, 0x5A);
    TEST_CHECK(stack_peek(stack) == 0x5A);

    TEST_CHECK(stack_pop(stack) == 0x5A);
    TEST_CHECK(stack_pop(stack) == 0xA5);
    TEST_CHECK(stack_pop(stack) == 0x00);

    for(size_t i = 0; i < 0x102; ++i) {
        stack_push(stack, i);
    }

    for(size_t i = 0xFF; i > 0; --i) {
        uint8_t v = stack_pop(stack);
        TEST_CHECK(v == i);
        TEST_MSG("%d != %zu", v, i);
    }
    TEST_CHECK(stack_pop(stack) == 0x00); // Last element
    TEST_CHECK(stack_pop(stack) == 0x00); // Empty stack
    free(stack);
}

static void print_rom(char * rom_file) {
    GBCPU cpu;
    cpu_initialize(&cpu);
    read_binary(rom_file, cpu.memory);

    const Cartridge* cartridge = cartridge_allocate(cpu.memory);

    printf("Cartridge Type: %d\n", cartridge->cartridge_type);
    printf("ROM Size: %d\n", cartridge->rom_size);
    printf("RAM Size: %d\n", cartridge->ram_size);

    free((void*)cartridge);
}

void test_game_roms() {
    print_rom("../roms/Dr. Mario (World).gb");
    print_rom("../roms/Kirby's Dream Land (USA, Europe).gb");
    print_rom("../roms/Tetris (World) (Rev A).gb");
        
}

TEST_LIST = {
   { "Stack", test_stack },
   { "Serial Buffer", test_serial_buffer },
   { "Check Game ROMs", test_game_roms },
   { NULL, NULL } 
};

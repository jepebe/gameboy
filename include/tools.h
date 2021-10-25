#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SERIAL_BUFFER_SIZE 0xFF

typedef struct {
    char buffer[SERIAL_BUFFER_SIZE];
    size_t pos;
    bool eol;
} SerialBuffer;

bool serial_buffer_push(SerialBuffer * serial_buffer, char c);
bool serial_buffer_eol(SerialBuffer * serial_buffer);
void serial_buffer_clear(SerialBuffer * serial_buffer);



#define STACK_SIZE 0x100
typedef struct {
    uint8_t stack[STACK_SIZE];
    size_t pointer;
} Stack;

Stack* stack_allocate();
void stack_push(Stack* stack, uint8_t value);
uint8_t stack_peek(Stack* stack);
uint8_t stack_pop(Stack* stack);

bool read_binary(const char* file, uint8_t* dst);

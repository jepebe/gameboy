#include "tools.h"

#include <stdio.h>
#include <stdlib.h>

bool serial_buffer_push(SerialBuffer *serial_buffer, char c) {
    if (serial_buffer->pos == SERIAL_BUFFER_SIZE - 1) {
        fprintf(stderr, "Serial Buffer full!\n");
        // printf("%s\n", &serial_buffer->buffer[0]);
        return false;
    }
    serial_buffer->buffer[serial_buffer->pos] = c;
    serial_buffer->pos++;
    serial_buffer->eol = (c == '\n');
    return true;
}

bool serial_buffer_eol(SerialBuffer *serial_buffer) {
    return serial_buffer->eol;
}

void serial_buffer_clear(SerialBuffer *serial_buffer) {
    for (size_t i = 0; i < SERIAL_BUFFER_SIZE; ++i) {
        serial_buffer->buffer[i] = 0;
    }
    serial_buffer->eol = false;
    serial_buffer->pos = 0;
}

Stack *stack_allocate() {
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    stack->pointer = 0x100;
    return stack;
}

void stack_push(Stack *stack, uint8_t value) {
    if (stack->pointer == 0) {
        printf("Stack max elements reached!\n");
        return;
    }
    stack->stack[--(stack->pointer)] = value;
}

uint8_t stack_pop(Stack *stack) {
    if (stack->pointer == STACK_SIZE) {
        printf("Stack is empty!\n");
        return 0x00;
    }
    uint8_t value = stack->stack[stack->pointer++];
    return value;
}

uint8_t stack_peek(Stack *stack) {
    if (stack->pointer == STACK_SIZE) {
        printf("Stack is empty!\n");
        return 0x00;
    }
    return stack->stack[stack->pointer];
}

bool read_binary(const char *file, uint8_t *dst) {
    FILE *fd = fopen(file, "rb");

    if (!fd) {
        perror("File opening failed");
        return false;
    }
    // get filesize
    fseek(fd, 0, SEEK_END);
    size_t file_size = ftell(fd);
    rewind(fd);

    size_t max_read = (file_size <= 0x10000) ? file_size : 0x10000;

    size_t read = fread(dst, sizeof(char), max_read, fd);
    fclose(fd);
    return read == file_size;
}

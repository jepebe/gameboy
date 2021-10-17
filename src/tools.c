#include <stdio.h>
#include <stdlib.h>
#include <tools.h>

bool read_binary(const char* file, uint8_t *dst) {
    FILE *fd = fopen(file, "rb");

    if (!fd) {
        perror("File opening failed");
        return false;
    }
    //get filesize 
    fseek(fd, 0, SEEK_END);
    size_t file_size = ftell(fd);
    rewind(fd);

    size_t read = fread(dst, sizeof(char), file_size, fd);

    return read == file_size;
}

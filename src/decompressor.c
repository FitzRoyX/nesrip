#include "decompressor.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

Result* decompressRleKonami(const uint8_t* rom_data, size_t offset, size_t section_size) {
    const uint8_t* data = rom_data + offset;
    Result* result = malloc(sizeof(Result));
    if (!result || !(result->output = malloc(section_size * 2 * sizeof(uint8_t)))) {
        printf("Error: Memory allocation failed.\n");
        free(result);
        return NULL;
    }
    result->size = 0;
    while (*data != 0xFF) { // Process until we hit the terminator
        uint8_t byte = *data++; // Read the current byte
        if (byte <= 0x80) {
            uint8_t repeat_byte = *data++; // Byte to repeat
            result->size += byte;
            memset(result->output + result->size - byte, repeat_byte, byte);
        } else {
            // Copy (byte - 0x80) raw bytes
            for (int i = 0; i < byte - 0x80; i++) {
                result->output[result->size++] = *data++;
            }
        }
    }
    return result;
}

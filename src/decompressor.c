#include "decompressor.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

Result* decompressRleKonami(const uint8_t* compressedData, size_t sectionSize) {
    const uint8_t* data = compressedData;
    Result* result = malloc(sizeof(Result));
    if (!result || !(result->output = malloc(sectionSize * 2 * sizeof(uint8_t)))) {
        printf("Error: Memory allocation failed.\n");
        free(result);
        return NULL;
    }
    result->size = 0;
    while (*data != 0xFF) {
        uint8_t byte = *data++;
        if (byte <= 0x80) {
            uint8_t repeat_byte = *data++;
            result->size += byte;
            memset(result->output + result->size - byte, repeat_byte, byte);
        } else {
            for (int i = 0; i < byte - 0x80; i++) {
                result->output[result->size++] = *data++;
            }
        }
    }
    return result;
}

Result* decompressLzss(const uint8_t* compressedData, size_t sectionSize) {
    const uint8_t* data = compressedData;
    Result* result = malloc(sizeof(Result));
    if (!result || !(result->output = malloc(sectionSize * sizeof(uint8_t)))) {
        printf("Error: Memory allocation failed.\n");
        free(result);
        return NULL;
    }
    result->size = 0;
    // Start of LZSS decompression
    while (result->size < sectionSize) {
        // If the high bit of the first byte is set, it's an LZSS block
        if ((*data) & 0x80) {
            uint8_t length = (*data & 0x7F) + 3;
            uint16_t offset = (*(data + 1) | ((*(data + 2) & 0xF0) << 4));
            // Decompress the LZSS block (copy from previous part of the output)
            data += 3;
            // Copy the referenced data from the previous part of the decompressed output
            for (size_t i = 0; i < length; i++) {
                result->output[result->size] = result->output[result->size - offset - 1];
                result->size++;
            }
        } else {
            // If the high bit of the first byte is not set, it's a literal byte
            result->output[result->size++] = *data++;
        }
    }
    return result;
}

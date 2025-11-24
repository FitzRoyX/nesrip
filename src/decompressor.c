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
    while (result->size < sectionSize) {
        if ((*data) & 0x80) {
            uint8_t length = (*data & 0x7F) + 3;
            uint16_t offset = (*(data + 1) | ((*(data + 2) & 0xF0) << 4));
            data += 3;
            for (size_t i = 0; i < length; i++) {
                result->output[result->size] = result->output[result->size - offset - 1];
                result->size++;
            }
        } else {
            result->output[result->size++] = *data++;
        }
    }
    return result;
}

Result* decompressLz2(const uint8_t* compressedData, size_t sectionSize) {
    const uint8_t* data = compressedData;
    const uint8_t* end  = compressedData + sectionSize;
    Result* result = malloc(sizeof(Result));
    if (!result) {
        printf("Error: Memory allocation failed.\n");
        return NULL;
    }
    size_t capacity = sectionSize * 8;
    if (capacity < 0x1000) {
        capacity = 0x1000;
    }
    result->output = malloc(capacity);
    if (!result->output) {
        printf("Error: Memory allocation failed.\n");
        free(result);
        return NULL;
    }
    result->size = 0;
    while (data < end) {
        uint8_t header = *data++;
        if (header == 0xFF) {
            break;
        }
        uint8_t  cmd    = header >> 5; // top 3 bits
        uint32_t length = header & 0x1F; // low 5 bits
        if (cmd == 0x07) {
            if (data >= end) {
                printf("Error: Unexpected end of data in long-length header.\n");
                break;
            }
            uint8_t header2 = *data++;
            cmd    = (header & 0x1C) >> 2; // CCC */
            length = ((header & 0x03) << 8) | header2; // 10-bit length
        }
        length += 1; //Effective length is always (L + 1)
        if (result->size + length > capacity) {
            size_t newCapacity = capacity * 2;
            while (result->size + length > newCapacity) {
                newCapacity *= 2;
            }
            uint8_t* newBuf = realloc(result->output, newCapacity);
            if (!newBuf) {
                printf("Error: Memory reallocation failed.\n");
                free(result->output);
                free(result);
                return NULL;
            }
            result->output = newBuf;
            capacity = newCapacity;
        }
        switch (cmd) {
            case 0b000: { //Direct copy
                if ((size_t)(end - data) < length) {
                    printf("Error: Unexpected end of data in direct copy.\n");
                    length = (uint32_t)(end - data);
                }
                memcpy(result->output + result->size, data, length);
                result->size += length;
                data        += length;
                break;
            }
            case 0b001: { //Byte fill
                if (data >= end) {
                    printf("Error: Unexpected end of data in byte fill.\n");
                    break;
                }
                uint8_t value = *data++;
                memset(result->output + result->size, value, length);
                result->size += length;
                break;
            }
            case 0b010: { //Word fill
                if (end - data < 2) {
                    printf("Error: Unexpected end of data in word fill.\n");
                    break;
                }
                uint8_t b1 = *data++;
                uint8_t b2 = *data++;
                for (uint32_t i = 0; i < length; i++) {
                    result->output[result->size++] = (i & 1) ? b2 : b1;
                }
                break;
            }
            case 0b011: { //Increasing fill
                if (data >= end) {
                    printf("Error: Unexpected end of data in increasing fill.\n");
                    break;
                }
                uint8_t value = *data++;
                for (uint32_t i = 0; i < length; i++) {
                    result->output[result->size++] = (uint8_t)(value + i);
                }
                break;
            }
            case 0b100: //SMW repeat, overlap-safe
            case 0b101: //SMW repeat, overlap-safe
            case 0b110: { //SMW repeat, overlap-safe
                if (end - data < 2) {
                    printf("Error: Unexpected end of data in repeat command.\n");
                    break;
                }
                uint8_t hi = *data++;
                uint8_t lo = *data++;
                uint16_t addr = ((uint16_t)hi << 8) | lo;
                if ((size_t)addr >= result->size) {
                    printf("Error: Invalid repeat address (%u, out %zu).\n",
                           (unsigned)addr, result->size);
                    break;
                }
                //Overlap-safe copy: behave like the original 65816 loop.
                uint32_t i;
                for (i = 0; i < length; i++) {
                    result->output[result->size] = result->output[addr + i];
                    result->size++;
                }
                break;
            }
            default: {
                //Should never happen (111 is handled via long-length header).
                printf("Error: Unknown compression command %u.\n", cmd);
                data = end;
                break;
            }
        }
    }
    return result;
}

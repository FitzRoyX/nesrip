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

Result* decompressSmwLz2(const uint8_t* compressedData, size_t sectionSize) {
    const uint8_t* data = compressedData;
    const uint8_t* end  = compressedData + sectionSize;

    Result* result = malloc(sizeof(Result));
    if (!result) {
        printf("Error: Memory allocation failed.\n");
        return NULL;
    }

    /* LC_LZ2 rarely blows up more than ~8×, but grow as needed. */
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

        /* 0xFF = end of compressed stream */
        if (header == 0xFF) {
            break;
        }

        uint8_t  cmd    = header >> 5;       /* top 3 bits */
        uint32_t length = header & 0x1F;     /* low 5 bits */

        /* Long-length form: 111CCCLL LLLLLLLL */
        if (cmd == 0x07) {
            if (data >= end) {
                printf("Error: Unexpected end of data in long-length header.\n");
                break;
            }
            uint8_t header2 = *data++;
            cmd    = (header & 0x1C) >> 2;               /* CCC */
            length = ((header & 0x03) << 8) | header2;   /* 10-bit length */
        }

        /* Effective length is always (L + 1) */
        length += 1;

        /* Ensure we have enough space for the new bytes */
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
            case 0b000: { /* Direct copy */
                if ((size_t)(end - data) < length) {
                    printf("Error: Unexpected end of data in direct copy.\n");
                    length = (uint32_t)(end - data);
                }
                memcpy(result->output + result->size, data, length);
                result->size += length;
                data        += length;
                break;
            }

            case 0b001: { /* Byte fill */
                if (data >= end) {
                    printf("Error: Unexpected end of data in byte fill.\n");
                    break;
                }
                uint8_t value = *data++;
                memset(result->output + result->size, value, length);
                result->size += length;
                break;
            }

            case 0b010: { /* Word fill */
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

            case 0b011: { /* Increasing fill */
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

            /* 100 / 101 / 110 -> SMW repeat, overlap-safe */
            case 0b100:
            case 0b101:
            case 0b110: {
                if (end - data < 2) {
                    printf("Error: Unexpected end of data in repeat command.\n");
                    break;
                }
                uint8_t hi = *data++;
                uint8_t lo = *data++;
                /* LC_LZ2 uses a big-endian address into the already-decompressed buffer */
                uint16_t addr = ((uint16_t)hi << 8) | lo;

                if ((size_t)addr >= result->size) {
                    printf("Error: Invalid repeat address (%u, out %zu).\n",
                           (unsigned)addr, result->size);
                    break;
                }

                /* Overlap-safe copy: behave like the original 65816 loop. */
                uint32_t i;
                for (i = 0; i < length; i++) {
                    result->output[result->size] = result->output[addr + i];
                    result->size++;
                }
                break;
            }

            default: {
                /* Should never happen (111 is handled via long-length header). */
                printf("Error: Unknown LC_LZ2 command %u.\n", cmd);
                data = end;
                break;
            }
        }
    }

    return result;
}

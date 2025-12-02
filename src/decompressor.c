#include "decompressor.h"

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

//Used by "Contra (U).nes"
Result* decompressRleKonami(const uint8_t* compressedData, size_t sectionSize) {
    const uint8_t* data = compressedData;
    Result* result = malloc(sizeof(Result));
    if (!result || !(result->output = malloc(sectionSize * 2))) {
        printf("Error: Memory allocation failed.\n");
        free(result);
        return NULL;
    }
    result->size = 0;
    while (*data != 0xFF) {
        uint8_t byte = *data++;
        if (byte <= 0x80) {
            uint8_t repeat_byte = *data++;
            memset(result->output + result->size, repeat_byte, byte);
            result->size += byte;
        } else {
            int count = byte - 0x80;
            while (count--) {
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

Result* decompressLz1(const uint8_t* compressedData, size_t sectionSize) {
    const uint8_t* data = compressedData;
    const uint8_t* end = compressedData + sectionSize;

    Result* result = malloc(sizeof(Result));
    if (!result) {
        printf("Error: Memory allocation failed.\n");
        return NULL;
    }

    // LZ1 usually expands heavily → allocate generously.
    size_t capacity = sectionSize * 8;
    if (capacity < 0x2000) {
        capacity = 0x2000;
    }

    result->output = malloc(capacity);
    if (!result->output) {
        printf("Error: Memory allocation failed.\n");
        free(result);
        return NULL;
    }

    result->size = 0;
    size_t flagsIndex = 0;  // The current index for bit flags
    uint8_t flagByte = *data++;  // Get the first byte of flags

    while (data < end) {
        // Check for the bit flag (one byte = 8 bits)
        for (int bit = 7; bit >= 0; --bit) {
            // If the current flag is 1, it's a back-reference
            if (flagByte & (1 << bit)) {
                // Back-reference: 12-bit offset + 4-bit length
                if (data + 1 >= end) {
                    printf("Error: Unexpected end of data in back-reference.\n");
                    free(result->output);
                    free(result);
                    return NULL;
                }

                uint8_t byte1 = *data++;
                uint8_t byte2 = *data++;

                // 12-bit offset (little-endian)
                uint16_t offset = (byte1 | ((byte2 & 0x0F) << 8));

                // 4-bit length (the length is in the low nibble of byte2, plus 3)
                uint8_t length = (byte2 >> 4) + 3;

                if (result->size < offset) {
                    printf("Error: Invalid back-reference offset (%u).\n", offset);
                    free(result->output);
                    free(result);
                    return NULL;
                }

                // Copy the data from the offset
                for (size_t i = 0; i < length; i++) {
                    result->output[result->size] = result->output[result->size - offset];
                    result->size++;
                }

            } else {
                // Literal byte: just copy it to the result
                if (data >= end) {
                    printf("Error: Unexpected end of data for literal byte.\n");
                    free(result->output);
                    free(result);
                    return NULL;
                }

                result->output[result->size++] = *data++;
            }

            // If we run out of flags, load the next flag byte
            if (--flagsIndex == 0 && data < end) {
                flagByte = *data++;
                flagsIndex = 8;
            }

            // If we consumed all input, exit early.
            if (data >= end) {
                return result;
            }
        }
    }

    return result;
}

//Used by "Super Mario World (U).sfc"
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

//Used by "Legend of Zelda, The - A Link to the Past (U).sfc"
Result* decompressLz2LE(const uint8_t* compressedData, size_t sectionSize) {
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
        uint8_t  cmd    = header >> 5;      // upper 3 bits
        uint32_t length = header & 0x1F;    // lower 5 bits
        if (cmd == 0x07) {
            if (data >= end) {
                printf("Error: Unexpected end of data in long-length header.\n");
                break;
            }
            uint8_t header2 = *data++;
            cmd    = (header & 0x1C) >> 2;                 // CCC
            length = ((header & 0x03) << 8) | header2;     // 10-bit length
        }
        length += 1;
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
            case 0b000: { // Direct copy
                if ((size_t)(end - data) < length) {
                    printf("Error: Unexpected end of data in direct copy.\n");
                    length = (uint32_t)(end - data);
                }
                memcpy(result->output + result->size, data, length);
                result->size += length;
                data        += length;
                break;
            }
            case 0b001: { // Byte fill
                if (data >= end) {
                    printf("Error: Unexpected end of data in byte fill.\n");
                    break;
                }
                uint8_t value = *data++;
                memset(result->output + result->size, value, length);
                result->size += length;
                break;
            }
            case 0b010: { // Word fill
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
            case 0b011: { // Increasing fill
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
            //
            // 100 CHANGED ONLY HERE: little-endian address
            //
            case 0b100: { //repeat byte but **LE address**
                if (end - data < 2) {
                    printf("Error: Unexpected end of data in repeat command (LE).\n");
                    break;
                }
                uint8_t lo = *data++;
                uint8_t hi = *data++;
                uint16_t addr = ((uint16_t)hi << 8) | lo; // LE CHANGE
                if ((size_t)addr >= result->size) {
                    printf("Error: Invalid repeat address (%u, out %zu).\n",
                           (unsigned)addr, result->size);
                    break;
                }
                for (uint32_t i = 0; i < length; i++) {
                    result->output[result->size] =
                        result->output[addr + i];
                    result->size++;
                }
                break;
            }
            case 0b101:
            case 0b110: {
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
                for (uint32_t i = 0; i < length; i++) {
                    result->output[result->size] =
                        result->output[addr + i];
                    result->size++;
                }
                break;
            }
            default:
                printf("Error: Unknown compression command %u.\n", cmd);
                data = end;
                break;
        }
    }

    return result;
}

Result* decompressLz3(const uint8_t* compressedData, size_t sectionSize) {
    const uint8_t* data = compressedData;
    const uint8_t* end  = compressedData + sectionSize;
    Result* result = malloc(sizeof(Result));
    if (!result) {
        printf("Error: Memory allocation failed.\n");
        return NULL;
    }
    size_t capacity = sectionSize * 8;
    if (capacity < 0x2000) {
        capacity = 0x2000;
    }
    result->output = malloc(capacity);
    if (!result->output) {
        printf("Error: Memory allocation failed.\n");
        free(result);
        return NULL;
    }
    result->size = 0;
    while (data < end) {
        uint8_t flags = *data++;
        for (int bit = 7; bit >= 0; bit--) {
            if (data >= end) {
                return result;
            }
            if (result->size >= capacity - 20) {
                size_t newCapacity = capacity * 2;
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
            if (flags & (1 << bit)) {
                result->output[result->size++] = *data++;
            } else {
                if (end - data < 2) {
                    printf("Error: Unexpected end of data in LZ3 backreference.\n");
                    return result;
                }
                uint8_t b1 = *data++;
                uint8_t b2 = *data++;
                uint32_t offset = ((uint32_t)(b2 & 0xF0) << 4) | b1;
                uint32_t length = (b2 & 0x0F) + 3;
                if (offset >= result->size) {
                    printf("Error: Invalid LZ3 offset (%u >= %zu).\n",
                           offset, result->size);
                    return result;
                }
                for (uint32_t i = 0; i < length; i++) {
                    result->output[result->size] =
                        result->output[result->size - offset - 1];
                    result->size++;
                }
            }
            if (result->size >= capacity - 32) {
                size_t newCapacity = capacity * 2;
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
        }
    }
    return result;
}

//Used by Super Mario Kart (U).sfc
//Used by Super Metroid (J)(U).sfc
//Used by SimCity (U).sfc
Result* decompressLz5(const uint8_t* compressedData, size_t sectionSize) {
    const uint8_t* data = compressedData;
    const uint8_t* end  = compressedData + sectionSize;
    Result* result = malloc(sizeof(Result));
    if (!result) {
        printf("Error: Memory allocation failed.\n");
        return NULL;
    }
    size_t capacity = sectionSize * 8;
    if (capacity < 0x2000) {
        capacity = 0x2000;
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
        uint8_t  cmd;
        uint32_t length;
        if ((header & 0xE0) == 0xE0) {
            if (data >= end) {
                printf("Error: Unexpected end of data in long-length header.\n");
                break;
            }
            uint8_t header2 = *data++;
            cmd    = (header >> 2) & 0x07;
            length = ((header & 0x03) << 8) | header2;
            length = length + 1;
        } else {
            cmd    = header >> 5;
            length = (header & 0x1F);
            length = length + 1;
        }
        if (result->size + length > capacity) {
            size_t newCapacity = capacity * 2;
            while (result->size + length > newCapacity) {
                newCapacity = newCapacity * 2;
            }
            uint8_t* newBuf = realloc(result->output, newCapacity);
            if (!newBuf) {
                printf("Error: Memory reallocation failed.\n");
                free(result->output);
                free(result);
                return NULL;
            }
            result->output = newBuf;
            capacity       = newCapacity;
        }
        switch (cmd) {
            case 0: { /* Direct copy */
                if ((size_t)(end - data) < length) {
                    printf("Error: Direct copy overrun in LZ5.\n");
                    length = (uint32_t)(end - data);
                }
                memcpy(result->output + result->size, data, length);
                result->size += length;
                data         += length;
                break;
            }
            case 1: { /* Byte fill */
                if (data >= end) {
                    printf("Error: Byte fill missing value in LZ5.\n");
                    break;
                }
                uint8_t value = *data++;
                memset(result->output + result->size, value, length);
                result->size += length;
                break;
            }
            case 2: { /* Word fill */
                if (end - data < 2) {
                    printf("Error: Word fill missing values in LZ5.\n");
                    break;
                }
                uint8_t b1 = *data++;
                uint8_t b2 = *data++;
                uint32_t i;
                for (i = 0; i < length; i++) {
                    if (i & 1) {
                        result->output[result->size++] = b2;
                    } else {
                        result->output[result->size++] = b1;
                    }
                }
                break;
            }
            case 3: { /* Increasing fill */
                if (data >= end) {
                    printf("Error: Increasing fill missing value in LZ5.\n");
                    break;
                }
                uint8_t value = *data++;
                uint32_t i;
                for (i = 0; i < length; i++) {
                    result->output[result->size++] = (uint8_t)(value + i);
                }
                break;
            }
            case 4:
            case 5: { /* Absolute repeat / XOR repeat */
                if (end - data < 2) {
                    printf("Error: Repeat missing address in LZ5.\n");
                    break;
                }
                uint8_t lo = *data++;
                uint8_t hi = *data++;
                uint16_t addr = (uint16_t)(lo | (hi << 8));
                if ((size_t)addr >= result->size) {
                    printf("Error: Repeat address OOB (%u >= %zu).\n",
                           (unsigned)addr, result->size);
                    break;
                }
                uint32_t i;
                for (i = 0; i < length; i++) {
                    uint8_t b = result->output[addr + i];
                    if (cmd == 5) {
                        b = b ^ 0xFF;
                    }
                    result->output[result->size++] = b;
                }
                break;
            }
            case 6:
            case 7: { /* Negative repeat / Negative XOR repeat */
                if (data >= end) {
                    printf("Error: Negative repeat missing offset.\n");
                    break;
                }
                uint8_t back = *data++;
                if (back == 0 || (size_t)back > result->size) {
                    printf("Error: LZ5 negative repeat OOB.\n");
                    break;
                }
                size_t start = result->size - back;
                uint32_t i;
                for (i = 0; i < length; i++) {
                    uint8_t b = result->output[start + i];
                    if (cmd == 7) {
                        b = b ^ 0xFF;
                    }
                    result->output[result->size++] = b;
                }
                break;
            }
            default: {
                printf("Error: Unknown LZ5 command %u.\n", cmd);
                data = end;
                break;
            }
        }
    }
    return result;
}

















/*
 * Dungeon Master II (DM2) Image Decompression
 * 
 * Based on the format documented at:
 * http://dmweb.free.fr/community/documentation/file-formats/dungeon-master-ii-data-files-notes/
 * 
 * Format Structure:
 * - Body binary format: Sequence of commands
 * - Format of Pixel: Raw pixel data
 * - Format of Repetition Command: Repeat a pixel value N times
 * - Decision of Type: Control byte determines command type
 * 
 * Command Types:
 * - 0x00-0x7F: Repetition command
 *   * Count = control byte value (1-127)
 *   * Next byte = pixel value to repeat
 * - 0x80-0xFE: Raw data command
 *   * Count = control byte - 0x80 (1-126)
 *   * Next N bytes = raw pixel data to copy
 * - 0xFF: End marker (terminates decompression)
 */
Result* decompressDm2(const uint8_t* compressedData, size_t sectionSize) {
    if (!compressedData || sectionSize == 0) {
        return NULL;
    }
    
    const uint8_t* data = compressedData;
    const uint8_t* end = compressedData + sectionSize;
    Result* result = malloc(sizeof(Result));

    if (!result) {
        printf("Error: Memory allocation failed for DM2 result structure.\n");
        return NULL;
    }

    // Allocate initial buffer - DM2 can expand significantly (typically 4-8x)
    size_t initialCapacity = sectionSize * 8;
    if (initialCapacity < 0x1000) {
        initialCapacity = 0x1000;
    }
    result->output = malloc(initialCapacity);
    if (!result->output) {
        printf("Error: Memory allocation failed for DM2 output buffer.\n");
        free(result);
        return NULL;
    }

    result->size = 0;
    size_t capacity = initialCapacity;
    
    while (data < end) {
        // Read control byte
        uint8_t controlByte = *data++;
        
        // Check for end marker (0xFF)
        if (controlByte == 0xFF) {
            break;
        }
        
        // Decision of Type: Determine command type based on control byte value
        if (controlByte < 0x80) {
            // REPETITION COMMAND (Format of Repetition Command)
            // Control byte value (0x00-0x7F) = count (1-127)
            // Next byte = pixel value to repeat
            
            uint8_t count = controlByte;
            if (count == 0) {
                // Zero count is invalid, skip
                continue;
            }
            
            // Check if we can read the pixel value
            if (data >= end) {
                // Out of bounds - can't read pixel value
                break;
            }
            
            uint8_t pixelValue = *data++;
            
            // Ensure we have enough buffer space
            if (result->size + count > capacity) {
                size_t newCapacity = capacity * 2;
                while (result->size + count > newCapacity) {
                    newCapacity *= 2;
                }
                uint8_t* newBuf = realloc(result->output, newCapacity);
                if (!newBuf) {
                    printf("Error: Memory reallocation failed in DM2 decompression.\n");
                    free(result->output);
                    free(result);
                    return NULL;
                }
                result->output = newBuf;
                capacity = newCapacity;
            }
            
            // Repeat the pixel value 'count' times
            memset(result->output + result->size, pixelValue, count);
            result->size += count;
            
        } else {
            // RAW DATA COMMAND (Format of Pixel)
            // Control byte value (0x80-0xFE) - 0x80 = count (1-126)
            // Next N bytes = raw pixel data to copy directly
            
            uint8_t count = controlByte - 0x80;
            if (count == 0) {
                // Invalid count (would be 0xFF which is end marker, but we already checked)
                break;
            }
            
            // Check if we have enough data to read
            if (data + count > end) {
                // Would read beyond bounds - only copy what's available
                count = (uint8_t)(end - data);
                if (count == 0) {
                    break;
                }
            }
            
            // Ensure we have enough buffer space
            if (result->size + count > capacity) {
                size_t newCapacity = capacity * 2;
                while (result->size + count > newCapacity) {
                    newCapacity *= 2;
                }
                uint8_t* newBuf = realloc(result->output, newCapacity);
                if (!newBuf) {
                    printf("Error: Memory reallocation failed in DM2 decompression.\n");
                    free(result->output);
                    free(result);
                    return NULL;
                }
                result->output = newBuf;
                capacity = newCapacity;
            }
            
            // Copy raw pixel data directly
            memcpy(result->output + result->size, data, count);
            result->size += count;
            data += count;
        }
    }
    
    // If we didn't decompress anything, return NULL
    if (result->size == 0) {
        free(result->output);
        free(result);
        return NULL;
    }
    
    return result;
}
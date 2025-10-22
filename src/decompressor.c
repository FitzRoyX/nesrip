#include "decompressor.h"
#include <stdlib.h>

unsigned char* decompress_rle_konami(unsigned char* data, int data_len, int* decompressed_len) {
    unsigned char* decompressed = malloc(data_len * sizeof(unsigned char)); // Allocate memory
    int i = 0, j = 0;

    while (i < data_len) {
        unsigned char byte = data[i];
        if (byte == 0xFF) {
            unsigned char repeat = data[i + 1];
            unsigned char value = data[i + 2];
            for (int k = 0; k < repeat; ++k) {
                decompressed[j++] = value;
            }
            i += 3;
        } else {
            decompressed[j++] = byte;
            i++;
        }
    }

    *decompressed_len = j;
    return decompressed;
}

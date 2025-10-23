#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "decompressor.h"

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

int decompress_rle_bakutoshu(void) {
	char buffer[BUFSIZ];
    int c;
    int i;
    size_t count_members;
    size_t count_repeats;

    while ((c = getchar()) != EOF && !feof(stdin)) {
            count_repeats = c & 0x1F;
            count_members = (c & 0xE0) >> 5;

            if (count_members == 5) {
                    break;
            } else if (count_members == 0) {
                    for (i = 0; i <= count_repeats; i++) {
                            putchar(getchar());
                    }
            } else {
                    count_members = (count_members >= 3) ? (count_members + 1) : count_members;
                    fread(buffer, 1, count_members, stdin);

                    for (i = 0; i <= count_repeats; i++) {
                            fwrite(buffer, 1, count_members, stdout);
                    }
            }
    }

    return 0;
}

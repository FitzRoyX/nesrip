#ifndef DECOMPRESSOR_H
#define DECOMPRESSOR_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint8_t* output;
	size_t size;
} Result;

Result* decompressRleKonami(const uint8_t* rom_data, size_t offset, size_t section_size);
int read_byte(const uint8_t* data, int* cursor);

#endif
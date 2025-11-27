#ifndef DECOMPRESSOR_H
#define DECOMPRESSOR_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint8_t* output;
	size_t size;
} Result;

Result* decompressRleKonami(const uint8_t* compressedData, size_t sectionSize);
Result* decompressLzss(const uint8_t* compressedData, size_t sectionSize);
Result* decompressLz1(const uint8_t* compressedData, size_t sectionSize);
Result* decompressLz2(const uint8_t* compressedData, size_t sectionSize);
Result* decompressLz2LE(const uint8_t* compressedData, size_t sectionSize);
Result* decompressLz3(const uint8_t* compressedData, size_t sectionSize);

#endif




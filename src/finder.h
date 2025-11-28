#ifndef FINDER_H
#define FINDER_H

#include <stdint.h>
#include <stddef.h>

// Rom is NOT a struct — include rom.h so the typedef is visible
#include "rom.h"

// Forward declarations of actual struct types
typedef struct ExtractionContext ExtractionContext;
typedef struct ExtractionArguments ExtractionArguments;

int findCompressedGraphics(Rom* rom, ExtractionArguments* arguments);
double computeFrequencyOfChange(const unsigned char* sheet, int width, int height);
uint32_t computeChecksum(const unsigned char* data, size_t size);
int hasChecksum(uint32_t hash);
void storeChecksum(uint32_t hash);
int sheetChecksumSeen(uint32_t hash);
void sheetChecksumStore(uint32_t hash);
int buildTilesheetFromDecompressed(ExtractionContext* baseContext, unsigned char* decompressedData, size_t decompressedSize, unsigned char** outSheet, int* outWidth, int* outHeight);
uint32_t hashBytes(const unsigned char* data, int size);

#endif

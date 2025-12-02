#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "decompressor.h"
#include "ripper.h"
#include "finder.h"
#include "utils.h"
#include "globals.h"
#include "stb/stb_image_write.h"

// ============================
//   GLOBAL CHECKSUM STORAGE
// ============================

static uint32_t* sheetChecksums = NULL;
static int sheetChecksumCount = 0;
static int sheetChecksumCapacity = 0;

static uint32_t* checksumHashes = NULL;
static int checksumCount = 0;
static int checksumCapacity = 0;

// ============================
//  SHEET CHECKSUM UTILITIES
// ============================

int sheetChecksumSeen(uint32_t hash) {
    for (int i = 0; i < sheetChecksumCount; i++) {
        if (sheetChecksums[i] == hash)
            return 1;
    }
    return 0;
}

void sheetChecksumStore(uint32_t hash) {
    if (sheetChecksumCount >= sheetChecksumCapacity) {
        sheetChecksumCapacity = (sheetChecksumCapacity == 0 ? 16 : sheetChecksumCapacity * 2);
        sheetChecksums = realloc(sheetChecksums, sheetChecksumCapacity * sizeof(uint32_t));
    }
    sheetChecksums[sheetChecksumCount++] = hash;
}

// ============================
//  FNV-1A RAW BYTE HASH
// ============================

uint32_t hashBytes(const unsigned char* data, int size) {
    uint32_t hash = 0x811C9DC5;
    for (int i = 0; i < size; i++) {
        hash ^= data[i];
        hash *= 0x01000193;
    }
    return hash;
}

// ============================
//  FREQUENCY OF CHANGE
// ============================

double computeFrequencyOfChange(const unsigned char* sheet, int width, int height) {
    int runCount = 0;
    int totalRunLength = 0;
    int pixels = width * height;
    for (int i = 4; i < pixels * 4; i += 4) {
        int same =
            sheet[i] == sheet[i - 4] &&
            sheet[i + 1] == sheet[i - 3] &&
            sheet[i + 2] == sheet[i - 2];
        if (!same)
            runCount++;
    }
    if (runCount == 0) return 0.0;
    return (double)pixels / (double)runCount;
}

// ============================
//  BUILD TILE SHEET FROM DECOMPRESSED DATA
// ============================

int buildTilesheetFromDecompressed(
    ExtractionContext* baseContext,
    unsigned char* decompressedData,
    size_t decompressedSize,
    unsigned char** outSheet,
    int* outWidth,
    int* outHeight)
{
    if (!baseContext || !decompressedData || decompressedSize == 0)
        return 0;
    if (decompressedSize % baseContext->tileLength != 0)
        return 0;
    int tileCount = (int)(decompressedSize / baseContext->tileLength);
    if (tileCount <= 0)
        return 0;
    ExtractionContext ctx = *baseContext;
    ctx.sheet = NULL;
    ctx.index = 0;
    ctx.tx = ctx.ty = 0;
    ctx.stx = ctx.sty = 0;
    ctx.maxX = ctx.maxY = 0;
    if (allocTilesheet(&ctx, tileCount))
        return 0;
    unsigned char* ptr = decompressedData;
    unsigned char* end = decompressedData + decompressedSize;
    while (ptr < end) {
        processTile(&ctx, ptr, &writeLine);
        incrementTilePos(&ctx);
        ptr += ctx.tileLength;
    }
    *outSheet = (unsigned char*)ctx.sheet;
    *outWidth = 128;
    *outHeight = ctx.maxY + 1;
    return 1;
}

// ============================
//  CHECKSUMS FOR CANDIDATE SELECTION
// ============================

uint32_t computeChecksum(const unsigned char* data, size_t size) {
    uint32_t hash = 0x811C9DC5;
    for (size_t i = 0; i < size; ++i) {
        hash ^= data[i];
        hash *= 0x01000193;
    }
    return hash;
}

int hasChecksum(uint32_t hash) {
    for (int i = 0; i < checksumCount; i++) {
        if (checksumHashes[i] == hash)
            return 1;
    }
    return 0;
}

void storeChecksum(uint32_t hash) {
    if (checksumCount >= checksumCapacity) {
        checksumCapacity = (checksumCapacity == 0 ? 16 : checksumCapacity * 2);
        checksumHashes = realloc(checksumHashes, checksumCapacity * sizeof(uint32_t));
    }
    checksumHashes[checksumCount++] = hash;
}

// ============================
//   MAIN FINDER LOGIC
// ============================

int findCompressedGraphics(Rom* rom, ExtractionArguments* arguments) {
    ExtractionContext context = {
        rom,
        arguments
    };
    printf("Finding compressed graphics between %s and %s.\n",
           context.args->sectionStartString,
           context.args->sectionEndString);
    if (!getSectionDetails(rom, &context))
        return 0;
    const char* compressionType = context.args->compressionType;
    if (strcmp(compressionType, "raw") == 0) {
        printf("FINDER: f command is only supported for compressed types.\n");
        return 0;
    }
    int originalStart = context.sectionStart;
    int originalEnd   = context.sectionEnd;
    int ffLocations[MAX_FF_LOCATIONS];
    int ffCount = 0;
    unsigned char* romBytes = (unsigned char*)rom->data;
    for (int addr = originalStart; addr <= originalEnd; ++addr) {
        if (ffCount >= MAX_FF_LOCATIONS)
            break;
        if (romBytes[addr] == 0xFF) {
            ffLocations[ffCount++] = addr;
        }
    }
    if (ffCount == 0) {
        printf("FINDER: No 0xFF bytes found in specified range.\n");
        return 0;
    }
    if (ffCount >= MAX_FF_LOCATIONS) {
        printf("FINDER: Warning, hit 0xFF location limit (%d); some candidates may be skipped.\n",
               MAX_FF_LOCATIONS);
    }

    int totalMatches = 0;
    int start = originalStart;
    int candidateIndex = 0;  // Initialize candidate index

    int totalAddresses = originalEnd - originalStart + 1; // Total addresses in range

    while (start <= originalEnd) {
        for (int i = 0; i < ffCount; ++i) {
            // --- Progress printout with two decimal places ---
            double progress = ((double)(start - originalStart) / totalAddresses) * 100;
            printf("FINDER: Checking 0xFF at %d / %d (AT %X). Progress=%.2f%%\n", 
                   i + 1, ffCount, ffLocations[i], progress);
            
            int endAddr = ffLocations[i];
            
            // Skip the current 0xFF byte if it's followed by another 0xFF byte
            if (endAddr + 1 < originalEnd && romBytes[endAddr + 1] == 0xFF) {
                printf("FINDER: Skipping 0xFF at %X because next byte is also 0xFF\n", endAddr);
                continue;
            }
            
            if (endAddr < start)
                continue;
            if (endAddr > originalEnd)
                break;
            size_t sectionSize = (size_t)(endAddr - start + 1);
            unsigned char* sectionData = romBytes + start;
            Result* decompressedData = NULL;
            if (strcmp(compressionType, "rle_konami") == 0) {
                decompressedData = decompressRleKonami(sectionData, sectionSize);
            } else if (strcmp(compressionType, "lzss") == 0) {
                decompressedData = decompressLzss(sectionData, sectionSize);
            } else if (strcmp(compressionType, "lz1") == 0) {
                decompressedData = decompressLz1(sectionData, sectionSize);
            } else if (strcmp(compressionType, "lz2") == 0) {
                decompressedData = decompressLz2(sectionData, sectionSize);
            } else if (strcmp(compressionType, "lz2le") == 0) {
                decompressedData = decompressLz2LE(sectionData, sectionSize);
            } else if (strcmp(compressionType, "lz3") == 0) {
                decompressedData = decompressLz3(sectionData, sectionSize);
            } else if (strcmp(compressionType, "lz5") == 0) {
                decompressedData = decompressLz5(sectionData, sectionSize);
            } else if (strcmp(compressionType, "dm2") == 0) {
                decompressedData = decompressDm2(sectionData, sectionSize);
            }
            if (!decompressedData || !decompressedData->output || decompressedData->size == 0) {
                if (decompressedData) {
                    free(decompressedData->output);
                    free(decompressedData);
                }
                continue;
            }
            if (decompressedData->size % context.tileLength != 0) {
                free(decompressedData->output);
                free(decompressedData);
                continue;
            }
            unsigned char* sheet = NULL;
            int width = 0, height = 0;
            if (!buildTilesheetFromDecompressed(&context,
                                                decompressedData->output,
                                                decompressedData->size,
                                                &sheet,
                                                &width,
                                                &height)) {
                free(decompressedData->output);
                free(decompressedData);
                continue;
            }
            int totalTiles = (decompressedData->size / context.tileLength);
            int qualifierTileCount = (totalTiles % 16 == 0); // Modify this for your specific game
            if (!qualifierTileCount) {
                printf("FINDER: Candidate %X-%X skipped due to having %d tiles.\n", start, endAddr, totalTiles);
                free(decompressedData->output);
                free(decompressedData);
                free(sheet);
                continue;
            }
            double freq = computeFrequencyOfChange(sheet, width, height);
            int qualifierFrequency = (freq >= 1.5 && freq <= 5.0);
            uint32_t checksumHash = computeChecksum(sheet, width * height * 4);
            int qualifierChecksum = !hasChecksum(checksumHash);
            if (qualifierChecksum) {
                storeChecksum(checksumHash);
            }

            if (qualifierTileCount && qualifierFrequency && qualifierChecksum) {
                char filename[512];
                // Format the candidate index with leading zeros
                snprintf(filename, sizeof(filename),
                         "%s[%04d]%x_%x.png", outputFolder, candidateIndex++, start, endAddr);
                printf("FINDER: Candidate %x-%x freq=%.2f, checksum, writing \"%s\".\n",
                       start, endAddr, freq, filename);
                if (!stbi_write_png(filename, width, height, 4, sheet, width * 4)) {
                    printf("FINDER: Failed to write \"%s\".\n", filename);
                } else {
                    ++totalMatches;
                }
            }
            free(sheet);
            free(decompressedData->output);
            free(decompressedData);
        }
        start++;
    }
    printf("FINDER: Finished scan. %d matching compressed graphic range(s) saved.\n", totalMatches);
    free(checksumHashes);
    return 1;
}
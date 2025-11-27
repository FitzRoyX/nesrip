#ifndef RIPPER_H
#define RIPPER_H

#include <stdint.h>
#include "rom.h"
#include "globals.h"

typedef enum BitPlaneType {
	ONE_BPP,
	TWO_BPP,
	TWO_BPP_SNES,
	THREE_BPP_SNES,
	THREE_BPP_SNES_MODE7,
	FOUR_BPP_SNES,
	FIVE_BPP,
	SIX_BPP,
	SEVEN_BPP,
	EIGHT_BPP_SNES,
	EIGHT_BPP_SNES_MODE7,
	BPP_COUNT
} BitplaneType;

typedef struct ExtractionArguments {
	//String arguments
	char* compressionType;
	char* bitplaneType;
	char* patternSizeString;
	char* patternDirectionString;
	char* deduplicator;
	char* sectionStartString;
	char* sectionEndString;
	//Output arguments
	char* outputFolder;
	char* filenameOverload;
} ExtractionArguments;

typedef struct ExtractionContext {
    Rom* rom;
    ExtractionArguments* args;
    int sectionStart;
    int sectionEnd;
    BitplaneType bitplaneType;
    int patternSize;
    int patternDirection;
    int deduplicator;
    int tileLength;
    char* sheet;
    int index;
    int tx;
    int ty;
    int stx;
    int sty;
    int maxX;
    int maxY;
    unsigned int workingHash;
} ExtractionContext;

typedef struct Pattern {
	unsigned char* data;
	unsigned int hash;
	struct Pattern *next;
	struct Pattern *down;
} Pattern;

int getSectionDetails(Rom* rom, ExtractionContext* context);
void incrementTilePos(ExtractionContext* context);
int allocTilesheet(ExtractionContext* context, int tileCount);
void processTile(ExtractionContext* context, unsigned char* sectionData, int (*callback)(ExtractionContext*, int, uint64_t));
int writeLine(ExtractionContext* context, int y, uint64_t data);
void initPatternChains();
void cleanupPatternChains();
int ripSection(Rom* rom, ExtractionArguments* arguments);

#endif

#ifndef RIPPER_H
#define RIPPER_H
#include "rom.h"

#define MAX_PATTERN_SIZE 16

typedef struct {
	//String arguments
	char* compressionType;
	char* bitplaneType;
	char* patternSizeString;
	char* patternDirectionString;
	char* paletteDescription;
	char* deduplicator;
	char* sectionStartString;
	char* sectionEndString;
	//Output arguments
	char* outputFolder;
	char* filenameOverload;
} ExtractionArguments;

int ripSection(Rom* rom, Cache* cache, ExtractionArguments* arguments);
void initPatternChains();
void cleanupPatternChains();

#endif

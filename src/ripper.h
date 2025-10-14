#ifndef RIPPER_H
#define RIPPER_H
#include "rom.h"

#define MAX_PATTERN_SIZE 16

typedef struct {
	//String arguments
	char* sectionStartString;
	char* sectionEndString;
	char* patternSizeString;
	char* patternDirectionString;
	char* paletteDescription;
	char* compressionType;
	char* bitplaneType;
	char* deduplicator;
	//Output arguments
	char* outputFolder;
	char* filenameOverload;
} ExtractionArguments;

int ripSection(Rom* rom, Cache* cache, ExtractionArguments* arguments);
void initPatternChains();
void cleanupPatternChains();

#endif

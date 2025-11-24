#ifndef GLOBALS_H
#define GLOBALS_H

#include "rom.h"
#include <stdbool.h>

#define MAX_PATTERN_SIZE 16
#define MAX_PALETTES 256
#define MAX_COLOR_ROWS 16384
#define MAX_FF_LOCATIONS 100000

extern Rom rom;
extern char* programName;
extern char* outputFolder;
extern char* outputFilename;
extern char* compressionType;
extern char* patternSize;
extern char* patternDirection;
extern char* bitplaneType;
extern char* databaseFilename;
extern char* deduplicator;
extern int patternOverride;
extern int bitplaneOverride;
extern int deduplicatorOverride;

typedef struct {
	int valid;
	char* name;
	char colors[4][4];
} ColorizerPalette;

typedef struct {
	int valid;
	int width;
	int height;
	unsigned char* tiles;
} ColorizerSheet;

extern ColorizerPalette palettes[MAX_PALETTES];
extern ColorizerSheet colorSheet;

extern const char *compressionTypes[];
extern const int lengthofCompressionTypes;

#endif

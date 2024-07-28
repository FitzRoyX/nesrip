#ifndef GLOBALS_H
#define GLOBALS_H
#include "rom.h"

extern Rom rom;
extern char* programName;
extern char* outputFolder;
extern char* outputFilename;
extern char* compressionType;
extern char* patternSize;
extern char* patternDirection;
extern char* paletteDescription;
extern char* bitplaneType;
extern char* descriptorFilename;
extern char* checkRedundant;
extern int patternOverride;
extern int paletteOverride;
extern int bitplaneOverride;
extern int checkRedundantOverride;

typedef struct
{
	int valid;
	char* name;
	char colors[4][4];
} ColorizerPalette;

typedef struct
{
	int valid;
	int width;
	int height;
	unsigned char* tiles;
} ColorizerSheet;

#define MAX_PALETTES    256
#define MAX_SHEETS      256

extern ColorizerPalette palettes[MAX_PALETTES];
extern ColorizerSheet sheets[MAX_SHEETS];

#endif
#ifndef GLOBALS_H
#define GLOBALS_H

#include "rom.h"
#include <stdbool.h>

extern Rom rom;
extern char* programName;
extern char* outputFolder;
extern char* outputFilename;
extern char* compressionType;
extern char* patternSize;
extern char* patternDirection;
extern char* paletteDescription;
extern char* bitplaneType;
extern char* databaseFilename;
extern char* checkRedundant;
extern int patternOverride;
extern int paletteOverride;
extern int bitplaneOverride;
extern int checkRedundantOverride;

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

typedef struct {
	int width, height, channels;
} PNGInfo;

typedef struct {
	PNGInfo imageInfo;
	int size;
	char* data;
} PNGImage;

typedef struct {
	int size;          // Current number of elements in the cache images
	int capacity;      // Maximum capacity of the cache images
	PNGImage** images; // pointer to array of PNGImages
} Cache;

#define MAX_PALETTES    256
#define MAX_COLOR_ROWS  16384

extern ColorizerPalette palettes[MAX_PALETTES];
extern ColorizerSheet colorSheet;
extern Cache* cache;
extern PNGImage image;
extern PNGInfo imageInfo;

#endif

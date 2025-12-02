#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include "globals.h"
#include "decompressor.h"
#include "logger.h"
#include "utils.h"
#include "ripper.h"
#include "stb/stb_image_write.h"

char paletteData[] = {
	0, 0, 0, 255, //000000 black
	255, 255, 255, 255, //ffffff white
	175, 175, 175, 255, //afafaf lgray
	100, 100, 100, 255, //646464 dgray
	150, 0, 0, 255, //960000 dred
	200, 0, 0, 255, //c80000 lred
	0, 150, 0, 255, //009600 dgreen
	0, 200, 0, 255, //00c800 lgreen
	0, 0, 150, 255, //000096 dblue
	0, 0, 200, 255, //0000c8 lblue
	0, 150, 150, 255, //009696 dteal
	0, 200, 200, 255, //00c8c8 lteal
	150, 150, 0, 255, //969600 dyellow
	200, 200, 0, 255, //c8c800 lyellow
	150, 0, 150, 255, //960096 dpurple
	200, 0, 200, 255, //c800c8 lpurple
//
	0, 0, 0, 255, //000000 black
	255, 255, 255, 255, //ffffff white
	175, 175, 175, 255, //afafaf lgray
	100, 100, 100, 255, //646464 dgray
	150, 0, 0, 255, //960000 dred
	200, 0, 0, 255, //c80000 lred
	0, 150, 0, 255, //009600 dgreen
	0, 200, 0, 255, //00c800 lgreen
	0, 0, 150, 255, //000096 dblue
	0, 0, 200, 255, //0000c8 lblue
	0, 150, 150, 255, //009696 dteal
	0, 200, 200, 255, //00c8c8 lteal
	150, 150, 0, 255, //969600 dyellow
	200, 200, 0, 255, //c8c800 lyellow
	150, 0, 150, 255, //960096 dpurple
	200, 0, 200, 255, //c800c8 lpurple
	0, 0, 0, 255, //000000 black
	255, 255, 255, 255, //ffffff white
	175, 175, 175, 255, //afafaf lgray
	100, 100, 100, 255, //646464 dgray
	150, 0, 0, 255, //960000 dred
	200, 0, 0, 255, //c80000 lred
	0, 150, 0, 255, //009600 dgreen
	0, 200, 0, 255, //00c800 lgreen
	0, 0, 150, 255, //000096 dblue
	0, 0, 200, 255, //0000c8 lblue
	0, 150, 150, 255, //009696 dteal
	0, 200, 200, 255, //00c8c8 lteal
	150, 150, 0, 255, //969600 dyellow
	200, 200, 0, 255, //c8c800 lyellow
	150, 0, 150, 255, //960096 dpurple
	200, 0, 200, 255, //c800c8 lpurple
	0, 0, 0, 255, //000000 black
	255, 255, 255, 255, //ffffff white
	175, 175, 175, 255, //afafaf lgray
	100, 100, 100, 255, //646464 dgray
	150, 0, 0, 255, //960000 dred
	200, 0, 0, 255, //c80000 lred
	0, 150, 0, 255, //009600 dgreen
	0, 200, 0, 255, //00c800 lgreen
	0, 0, 150, 255, //000096 dblue
	0, 0, 200, 255, //0000c8 lblue
	0, 150, 150, 255, //009696 dteal
	0, 200, 200, 255, //00c8c8 lteal
	150, 150, 0, 255, //969600 dyellow
	200, 200, 0, 255, //c8c800 lyellow
	150, 0, 150, 255, //960096 dpurple
	200, 0, 200, 255, //c800c8 lpurple
	0, 0, 0, 255, //000000 black
	255, 255, 255, 255, //ffffff white
	175, 175, 175, 255, //afafaf lgray
	100, 100, 100, 255, //646464 dgray
	150, 0, 0, 255, //960000 dred
	200, 0, 0, 255, //c80000 lred
	0, 150, 0, 255, //009600 dgreen
	0, 200, 0, 255, //00c800 lgreen
	0, 0, 150, 255, //000096 dblue
	0, 0, 200, 255, //0000c8 lblue
	0, 150, 150, 255, //009696 dteal
	0, 200, 200, 255, //00c8c8 lteal
	150, 150, 0, 255, //969600 dyellow
	200, 200, 0, 255, //c8c800 lyellow
	150, 0, 150, 255, //960096 dpurple
	200, 0, 200, 255, //c800c8 lpurple
	0, 0, 0, 255, //000000 black
	255, 255, 255, 255, //ffffff white
	175, 175, 175, 255, //afafaf lgray
	100, 100, 100, 255, //646464 dgray
	150, 0, 0, 255, //960000 dred
	200, 0, 0, 255, //c80000 lred
	0, 150, 0, 255, //009600 dgreen
	0, 200, 0, 255, //00c800 lgreen
	0, 0, 150, 255, //000096 dblue
	0, 0, 200, 255, //0000c8 lblue
	0, 150, 150, 255, //009696 dteal
	0, 200, 200, 255, //00c8c8 lteal
	150, 150, 0, 255, //969600 dyellow
	200, 200, 0, 255, //c8c800 lyellow
	150, 0, 150, 255, //960096 dpurple
	200, 0, 200, 255, //c800c8 lpurple
	0, 0, 0, 255, //000000 black
	255, 255, 255, 255, //ffffff white
	175, 175, 175, 255, //afafaf lgray
	100, 100, 100, 255, //646464 dgray
	150, 0, 0, 255, //960000 dred
	200, 0, 0, 255, //c80000 lred
	0, 150, 0, 255, //009600 dgreen
	0, 200, 0, 255, //00c800 lgreen
	0, 0, 150, 255, //000096 dblue
	0, 0, 200, 255, //0000c8 lblue
	0, 150, 150, 255, //009696 dteal
	0, 200, 200, 255, //00c8c8 lteal
	150, 150, 0, 255, //969600 dyellow
	200, 200, 0, 255, //c8c800 lyellow
	150, 0, 150, 255, //960096 dpurple
	200, 0, 200, 255, //c800c8 lpurple
	0, 0, 0, 255, //000000 black
	255, 255, 255, 255, //ffffff white
	175, 175, 175, 255, //afafaf lgray
	100, 100, 100, 255, //646464 dgray
	150, 0, 0, 255, //960000 dred
	200, 0, 0, 255, //c80000 lred
	0, 150, 0, 255, //009600 dgreen
	0, 200, 0, 255, //00c800 lgreen
	0, 0, 150, 255, //000096 dblue
	0, 0, 200, 255, //0000c8 lblue
	0, 150, 150, 255, //009696 dteal
	0, 200, 200, 255, //00c8c8 lteal
	150, 150, 0, 255, //969600 dyellow
	200, 200, 0, 255, //c8c800 lyellow
	150, 0, 150, 255, //960096 dpurple
	200, 0, 200, 255, //c800c8 lpurple
	0, 0, 0, 255, //000000 black
	255, 255, 255, 255, //ffffff white
	175, 175, 175, 255, //afafaf lgray
	100, 100, 100, 255, //646464 dgray
	150, 0, 0, 255, //960000 dred
	200, 0, 0, 255, //c80000 lred
	0, 150, 0, 255, //009600 dgreen
	0, 200, 0, 255, //00c800 lgreen
	0, 0, 150, 255, //000096 dblue
	0, 0, 200, 255, //0000c8 lblue
	0, 150, 150, 255, //009696 dteal
	0, 200, 200, 255, //00c8c8 lteal
	150, 150, 0, 255, //969600 dyellow
	200, 200, 0, 255, //c8c800 lyellow
	150, 0, 150, 255, //960096 dpurple
	200, 0, 200, 255, //c800c8 lpurple
	0, 0, 0, 255, //000000 black
	255, 255, 255, 255, //ffffff white
	175, 175, 175, 255, //afafaf lgray
	100, 100, 100, 255, //646464 dgray
	150, 0, 0, 255, //960000 dred
	200, 0, 0, 255, //c80000 lred
	0, 150, 0, 255, //009600 dgreen
	0, 200, 0, 255, //00c800 lgreen
	0, 0, 150, 255, //000096 dblue
	0, 0, 200, 255, //0000c8 lblue
	0, 150, 150, 255, //009696 dteal
	0, 200, 200, 255, //00c8c8 lteal
	150, 150, 0, 255, //969600 dyellow
	200, 200, 0, 255, //c8c800 lyellow
	150, 0, 150, 255, //960096 dpurple
	200, 0, 200, 255, //c800c8 lpurple
	0, 0, 150, 255, //000096 dblue
	0, 0, 200, 255, //0000c8 lblue
	0, 150, 150, 255, //009696 dteal
	0, 200, 200, 255 //00c8c8 lteal
};

char deduplicatedTileColor[] = {
	255, 0, 255, 255 //ff00ff magenta
};

Pattern* patterns[BPP_COUNT];
ColorizerPalette palettes[MAX_PALETTES];
ColorizerSheet colorSheet;
int colorSheetIndex = 0;


int allocTilesheet(ExtractionContext* context, int tileCount) {
	int width = 128;
	int tilesPerRow = 16 * context->patternSize;
	int rows = tileCount / tilesPerRow;
	if (tileCount % tilesPerRow != 0)
		rows++;
	int height = rows * context->patternSize * 8;
	if (height == 0)
		height = context->patternSize * 8;  // avoid malloc(0)
	size_t bytes = width * height * 4;
	context->sheet = malloc(bytes);
	if (context->sheet == NULL) {
		printf("Error: Couldn't allocate memory for tilesheet.\n");
		return 1;
	}
	memset(context->sheet, 0, bytes);
	return 0;
}

void drawPixel(char* sheet, int x, int y, char* color) {
	if (x < 0 || x >= 128 || y < 0)
		return;
	int index = (y * 128 + x) * 4;
	for (int i = 0; i < 4; i++) {
		sheet[index + i] = color[i];
	}
}

char* allocOverloadedFilename(ExtractionContext* context) {
    ExtractionArguments* args = context->args;
    size_t lenOutputFolder = strlen(args->outputFolder);
    size_t lenFilename = strlen("0");
    /* Need: folder + "0" + "." + "png" + '\0' */
    char* result = malloc(lenOutputFolder + lenFilename + 1 + 3 + 1);
    if (!result) {
        printf("Error: Couldn't allocate memory for filename data.\n");
        return NULL;
    }
    char* p = result;
    memcpy(p, args->outputFolder, lenOutputFolder);
    p += lenOutputFolder;
    memcpy(p, "0", lenFilename);
    p += lenFilename;
    *p++ = '.';
    memcpy(p, "png", 3);
    p += 3;
    *p = 0;
    return result;
}

int writeOutput(char* outputData, int width, int height, ExtractionContext* context) {
    char* outputFilename = NULL;
    if (context->args->filenameOverload != NULL)
        outputFilename = allocOverloadedFilename(context);
    /* Fallback: use filenameOverload directly */
    if (outputFilename == NULL)
        outputFilename = context->args->filenameOverload;
    if (outputFilename == NULL)
        return 0;
    printf("  Writing sheet to \"%s\".\n", outputFilename);
    if (!stbi_write_png(outputFilename, width, height, 4, outputData, width * 4)) {
        printf("An error occurred while writing to output file %s.\n", outputFilename);
        if (outputFilename != context->args->filenameOverload)
            free(outputFilename);
        return 0;
    }
    if (outputFilename != context->args->filenameOverload)
        free(outputFilename);
    return 1;
}

int getSectionDetails(Rom* rom, ExtractionContext* context) {
	ExtractionArguments* args = context->args;
	if (str2int(&(context->sectionStart), args->sectionStartString, 16) != STR2INT_SUCCESS || context->sectionStart < 0 || context->sectionStart >= rom->size) {
		printf("Error: Invalid section start address.\n");
		return 0;
	}
	if (str2int(&(context->sectionEnd), args->sectionEndString, 16) != STR2INT_SUCCESS || context->sectionEnd < 0 || context->sectionEnd >= rom->size) {
		printf("Error: Invalid section end address.\n");
		return 0;
	}
	if (str2int(&(context->patternSize), args->patternSizeString, 10) != STR2INT_SUCCESS || numberOfSetBits(context->patternSize) > 1 || (unsigned int)context->patternSize > MAX_PATTERN_SIZE) {
		printf("Error: Invalid pattern size.\n");
		return 0;
	}
	if (context->sectionEnd < context->sectionStart) {
		printf("Error: Section end is placed before start.\n");
		return 0;
	}
	if (strcmp(args->patternDirectionString, "h") == 0)
		context->patternDirection = false;
	else if (strcmp(args->patternDirectionString, "v") == 0)
		context->patternDirection = true;
	else
		printf("Error: Invalid pattern direction. Use \"h\" or \"v\".\n");
	if (strcmp(args->bitplaneType, "1") == 0) {
		context->bitplaneType = ONE_BPP;
		context->tileLength = 8;
	}
	else if (strcmp(args->bitplaneType, "2_nes") == 0) {
		context->bitplaneType = TWO_BPP_NES;
		context->tileLength = 16;
	}
	else if (strcmp(args->bitplaneType, "2_snes") == 0) {
		context->bitplaneType = TWO_BPP_SNES;
		context->tileLength = 16;
	}
	else if (strcmp(args->bitplaneType, "3_snes") == 0) {
		context->bitplaneType = THREE_BPP_SNES;
		context->tileLength = 24;
	}
	else if (strcmp(args->bitplaneType, "3_snes_mode7") == 0) {
		context->bitplaneType = THREE_BPP_SNES_MODE7;
		context->tileLength = 24;
	}
	else if (strcmp(args->bitplaneType, "4_snes") == 0) {
		context->bitplaneType = FOUR_BPP_SNES;
		context->tileLength = 32;
	}
	else if (strcmp(args->bitplaneType, "5") == 0) {
		context->bitplaneType = FIVE_BPP;
		context->tileLength = 40;
	}
	else if (strcmp(args->bitplaneType, "6") == 0) {
		context->bitplaneType = SIX_BPP;
		context->tileLength = 48;
	}
	else if (strcmp(args->bitplaneType, "7") == 0) {
		context->bitplaneType = SEVEN_BPP;
		context->tileLength = 56;
	}
	else if (strcmp(args->bitplaneType, "8_snes") == 0) {
		context->bitplaneType = EIGHT_BPP_SNES;
		context->tileLength = 64;
	}
	else if (strcmp(args->bitplaneType, "8_snes_mode7") == 0) {
		context->bitplaneType = EIGHT_BPP_SNES_MODE7;
		context->tileLength = 64;
	}
	if (strcmp(args->deduplicator, "true") == 0) {
		context->deduplicator = true;
	}
	else if (strcmp(args->deduplicator, "false") == 0) {
		context->deduplicator = false;
	}
	else {
		printf("Error: Invalid deduplicator value. Use \"true\" or \"false\". Defaulting to \"true\".\n");
		context->deduplicator = true;
	}
	return 1;
}

void incrementTilePos(ExtractionContext* context) {
	context->index++;
	context->stx++;
	if (context->stx == context->patternSize) {
		context->stx = 0;
		context->sty++;
		if (context->sty == context->patternSize) {
			context->sty = 0;
			context->tx++;
			if (context->tx == 16 / context->patternSize) {
				context->tx = 0;
				context->ty++;
			}
		}
	}
}

int writeLine(ExtractionContext* context, int y, uint64_t data) {
	int stx, sty;
	if (context->patternDirection) {
		stx = context->sty;
		sty = context->stx;
	} else {
		stx = context->stx;
		sty = context->sty;
	}

	int py = (context->ty * context->patternSize + sty) * 8 + y;
	int basePx = (context->tx * context->patternSize + stx) * 8;
	ColorizerSheet* sheet = &colorSheet;

	int cx;
	if (colorSheetIndex + context->index < 16)
		cx = (colorSheetIndex + context->index);
	else
		cx = (colorSheetIndex + context->index) % 16;
	int cy = (colorSheetIndex + context->index) / 16;

	for (int x = 0; x < 8; x++) {
		unsigned char c;
		int px;

		if (context->bitplaneType == EIGHT_BPP_SNES_MODE7) {
			/* Mode 7: 8bpp linear pixels in 64-bit row */
			c = (unsigned char)((data >> ((7 - x) * 8)) & 0xFF);
			px = basePx + x;  // left to right
		} else {
			/* Planar SNES formats: use existing interleaved bitplane extraction */
			c = (unsigned char)(
				  ((data & 0x01000000) >> 21)
				| ((data & 0x00010000) >> 14)
				| ((data & 0x00000100) >> 7)
				|  (data & 0x00000001)
			);
			data = (data >> 1) & 0x7F7F7F7F;
			px = basePx + 7 - x;
		}

		char color[4];
		int colorIndex = c * 4;
		for (int i = 0; i < 4; i++)
			color[i] = paletteData[colorIndex + i];

		if (sheet->valid && cx < sheet->width && cy < sheet->height) {
			ColorizerPalette* palette =
				&palettes[sheet->tiles[cy * sheet->width + cx]];
			if (palette->valid)
				memcpy(color, palette->colors[(unsigned int)c], sizeof(color));
		}

		drawPixel(context->sheet, px, py, color);
		if (px > context->maxX)
			context->maxX = px;
	}

	if (py > context->maxY)
		context->maxY = py;

	return 0;
}

int addToHash(ExtractionContext* context, int y, uint64_t data) {
	context->workingHash ^= (unsigned int)data;  // lower 32 bits
	return 0;
}

uint64_t getLineData(ExtractionContext* context, unsigned char* sectionData, int y) {
    switch (context->bitplaneType) {
        case ONE_BPP:
            return sectionData[y];
        case TWO_BPP_NES:
            return (uint64_t)sectionData[y] | ((uint64_t)sectionData[y + 8] << 8);
        case TWO_BPP_SNES:
            return (uint64_t)sectionData[y * 2] | ((uint64_t)sectionData[y * 2 + 1] << 8);
        case THREE_BPP_SNES:
            return (uint64_t)sectionData[y * 2] | ((uint64_t)sectionData[y * 2 + 1] << 8) | ((uint64_t)sectionData[y + 16] << 16);
        case THREE_BPP_SNES_MODE7: {
            uint32_t val = (uint32_t)sectionData[y * 3 + 2] | ((uint32_t)sectionData[y * 3 + 1] << 8) | ((uint32_t)sectionData[y * 3 + 0] << 16);
            uint64_t out = 0;
            for (int k = 0; k < 8; k++) {
                uint32_t bits = (val >> (3 * k)) & 7; // extract 3 bits
                out |= (uint64_t)(bits & 1) << (0  + k); // plane 0
                out |= (uint64_t)(bits & 2) << (7  + k); // plane 1
                out |= (uint64_t)(bits & 4) << (14 + k); // plane 2
            }
            return out;
		}
        case FOUR_BPP_SNES:
            return (uint64_t)sectionData[y * 2 + 0] | ((uint64_t)sectionData[y * 2 + 16] << 8) | ((uint64_t)sectionData[y * 2 + 1] << 16) | ((uint64_t)sectionData[y * 2 + 17] << 24);
        case EIGHT_BPP_SNES: {
            uint64_t out = 0;
            for (int i = 0; i < 8; i++) {
                out |= ((uint64_t)sectionData[y * 16 + i * 2] << (8 * i));
                out |= ((uint64_t)sectionData[y * 16 + i * 2 + 1] << (8 * (i + 8)));
            }
            return out;
		}
        case EIGHT_BPP_SNES_MODE7:
            return ((uint64_t)sectionData[y * 8 + 0] << 56) | ((uint64_t)sectionData[y * 8 + 1] << 48) | ((uint64_t)sectionData[y * 8 + 2] << 40) |
                   ((uint64_t)sectionData[y * 8 + 3] << 32) | ((uint64_t)sectionData[y * 8 + 4] << 24) | ((uint64_t)sectionData[y * 8 + 5] << 16) |
                   ((uint64_t)sectionData[y * 8 + 6] << 8) | ((uint64_t)sectionData[y * 8 + 7]);
        default:
            return 0;
    }
}

// This function processes the tile data
void processTile(ExtractionContext* context, unsigned char* sectionData, int (*callback)(ExtractionContext*, int, uint64_t)) {
    for (int y = 0; y < 8; y++)
        if (callback(context, y, getLineData(context, sectionData, y)))
            return;
}

int isTileMatch(ExtractionContext* context, unsigned char* tileDataA, unsigned char* tileDataB) {
	for (int y = 0; y < 8; y++) {
		uint64_t lineA = getLineData(context, tileDataA, y);
		uint64_t lineB = getLineData(context, tileDataB, y);
		if (lineA != lineB)
			return 0;
	}
	return 1;
}

int checkHasTileMatch(ExtractionContext* context, unsigned char* tileData, unsigned int hash) {
	Pattern* pattern = malloc(sizeof(Pattern));
	if (!pattern) {
		perror("Error allocating pattern");
		exit(EXIT_FAILURE);
	}
	pattern->data = tileData;
	pattern->hash = hash;
	pattern->next = NULL;
	pattern->down = NULL;
	Pattern* head = patterns[context->bitplaneType];
	if (!head) {
		patterns[context->bitplaneType] = pattern;
		return 0;
	}
	Pattern* p = head;
	Pattern* prev = NULL;
	while (p && p->hash != hash) {
		prev = p;
		p = p->next;
	}
	if (!p) {
		pattern->next = head;
		patterns[context->bitplaneType] = pattern;
		return 0;
	}
	Pattern* down = p;
	while (down) {
		if (isTileMatch(context, tileData, down->data)) {
			free(pattern);
			return 1;
		}
		down = down->down;
	}
	pattern->down = p;
	if (prev)
		prev->next = pattern;
	else
		patterns[context->bitplaneType] = pattern;
	pattern->next = p->next;
	return 0;
}

void drawDeduplicatedTile(ExtractionContext* context) {
	int stx, sty;
	if (context->patternDirection) {
		stx = context->sty;
		sty = context->stx;
	} else {
		stx = context->stx;
		sty = context->sty;
	}
	for (int y = 0; y < 8; y++) {
		int py = (context->ty * context->patternSize + sty) * 8 + y;
		int px = 0;
		for (int x = 0; x < 8; x++) {
			int rx = 7 - x;
			px = (context->tx * context->patternSize + stx) * 8 + rx;
			drawPixel(context->sheet, px, py, deduplicatedTileColor);
		}
		if (px > context->maxX)
			context->maxX = px;
		if (py > context->maxY)
			context->maxY = py;
	}
}

void initPatternChains() {
	for (int i = 0; i < BPP_COUNT; i++)
		patterns[i] = NULL;
}

void cleanupPatternChains() {
	for (int i = 0; i < BPP_COUNT; i++) {
		Pattern* p = patterns[i];
		while (p) {
			Pattern* next = p->next;
			Pattern* down = p->down;
			while (down) {
				Pattern* dnext = down->down;
				free(down);
				down = dnext;
			}
			free(p);
			p = next;
		}
	}
}

// Function to decompress data based on compression type
Result* decompressData(const char* compressionType, unsigned char* sectionData, size_t sectionSize) {
    if (strcmp(compressionType, "rle_konami") == 0)
        return decompressRleKonami(sectionData, sectionSize);
    else if (strcmp(compressionType, "lzss") == 0)
        return decompressLzss(sectionData, sectionSize);
    else if (strcmp(compressionType, "lz1") == 0)
        return decompressLz1(sectionData, sectionSize);
    else if (strcmp(compressionType, "lz2") == 0)
        return decompressLz2(sectionData, sectionSize);
    else if (strcmp(compressionType, "lz2le") == 0)
        return decompressLz2LE(sectionData, sectionSize);
    else if (strcmp(compressionType, "lz3") == 0)
        return decompressLz3(sectionData, sectionSize);
    else if (strcmp(compressionType, "lz5") == 0)
        return decompressLz5(sectionData, sectionSize);
    else if (strcmp(compressionType, "dm2") == 0)
        return decompressDm2(sectionData, sectionSize);
    return NULL;
}

unsigned char* deinterleaveData(unsigned char* data, size_t size, const char* interleaveType, size_t* filteredSize) {
    if (strcmp(interleaveType, "none") == 0) {
        // If interleaving type is "none", return the data as is
        *filteredSize = size;
        return data;
    }

    int mode = (strcmp(interleaveType, "even") == 0) ? 0 : 1; // Mode for "even" or "odd"
    if (!data || size == 0) {
        *filteredSize = 0;
        return NULL;
    }

    size_t count;
    unsigned char* out;
    // Deinterleave processing
    switch (mode) {
        case 0: // even (keep even indexed bytes)
            count = (size + 1) / 2;
            out = malloc(count);
            for (size_t i = 0, j = 0; i < size; i += 2, j++) {
                out[j] = data[i];
            }
            break;
        case 1: // odd (keep odd indexed bytes)
            count = size / 2;
            out = malloc(count);
            for (size_t i = 1, j = 0; i < size; i += 2, j++) {
                out[j] = data[i];
            }
            break;
        default:
            *filteredSize = 0;
            return NULL;
    }

    if (!out) {
        printf("Error: malloc failed in deinterleaveData.\n");
        *filteredSize = 0;
        return NULL;
    }

    *filteredSize = count;
    return out;
}

// Function to process the tiles
void processTilesInSection(ExtractionContext* context, unsigned char* data, size_t size) {
    unsigned char* dataPtr = data;
    unsigned char* dataEnd = data + size;

    while (dataPtr < dataEnd) {
        if (context->deduplicator) {
            context->workingHash = 0;
            processTile(context, dataPtr, &addToHash);

            if (!checkHasTileMatch(context, dataPtr, context->workingHash))
                processTile(context, dataPtr, &writeLine);
            else
                drawDeduplicatedTile(context);
        } else {
            processTile(context, dataPtr, &writeLine);
        }
        incrementTilePos(context);
        dataPtr += context->tileLength;
    }
}

// Function to finalize the section and handle output
void finalizeSection(ExtractionContext* context, int tileCount) {
    colorSheetIndex += tileCount;
    appendSectionToOutput(context->sheet, 128, context->maxY + 1);
    free(context->sheet);
    cleanupPatternChains();
    initPatternChains();
}

int ripSection(Rom* rom, ExtractionArguments* arguments) {
    // Check if this is a .dat file - if so, use DM2-specific extraction
    if (databaseFilename && strstr(databaseFilename, "dos.txt")) {
        const char* compressionType = arguments->compressionType;
        if (compressionType && strcmp(compressionType, "dm2") == 0) {
            return ripDm2GraphicsDat(rom, arguments);
        }
    }

    ExtractionContext context = {
        rom,
        arguments
    };

    printf("Ripping section %s to %s.\n", context.args->sectionStartString, context.args->sectionEndString);

    if (!getSectionDetails(rom, &context))
        return 0;

    unsigned char* sectionData = (unsigned char*)rom->data + context.sectionStart;
    unsigned char* endPointer   = (unsigned char*)rom->data + context.sectionEnd;
    const char* compressionType = context.args->compressionType;

    // Handle the decompressed formats
    if (strcmp(compressionType, "rle_konami") == 0 ||
        strcmp(compressionType, "lzss") == 0 ||
        strcmp(compressionType, "lz1") == 0 ||
        strcmp(compressionType, "lz2") == 0 ||
        strcmp(compressionType, "lz2le") == 0 ||
        strcmp(compressionType, "lz3") == 0 ||
        strcmp(compressionType, "lz5") == 0 ||
        strcmp(compressionType, "dm2") == 0) {

        size_t sectionSize = context.sectionEnd - context.sectionStart + 1;
        Result* decompressedData = decompressData(compressionType, sectionData, sectionSize);

        if (!decompressedData) {
            printf("Error: Failed to decompress the section.\n");
            return 0;
        }

        // Deinterleave if necessary
        unsigned char* decompressedPtr = decompressedData->output;
        size_t filteredSize = 0;
        decompressedPtr = deinterleaveData(decompressedPtr, decompressedData->size, interleaveType, &filteredSize);
        decompressedData->output = decompressedPtr;
        decompressedData->size = filteredSize;

        // Tile count based on REAL tile size
        int tileCount = decompressedData->size / context.tileLength;
        if (decompressedData->size % context.tileLength != 0) {
            printf("Warning: Decompressed data is not a multiple of %d bytes, truncating the extra bytes.\n", context.tileLength);
        }

        // Allocate tilesheet
        if (allocTilesheet(&context, tileCount)) {
            free(decompressedData->output);
            free(decompressedData);
            return 0;
        }

        if (context.sheet == NULL) {
            free(decompressedData->output);
            free(decompressedData);
            return 0;
        }

        // Process tiles
        processTilesInSection(&context, decompressedData->output, decompressedData->size);

        free(decompressedData->output);
        free(decompressedData);

        // Finalize section processing
        finalizeSection(&context, tileCount);

    } else if (strcmp(compressionType, "raw") == 0) { // Handling the "raw" case
        size_t sectionSize = (context.sectionEnd - context.sectionStart) + 1;

        // Check if the section can be divided into complete tiles
        if (sectionSize % context.tileLength != 0) {
            printf("Warning: Targeted section has some extra bytes that cannot "
                   "be used to make a full tile. Rounding down section end address.\n");

            context.sectionEnd -= (sectionSize % context.tileLength);
            sectionSize = (context.sectionEnd - context.sectionStart) + 1;
        }

        // Deinterleave raw section data if necessary
        sectionData = deinterleaveData(sectionData, sectionSize, interleaveType, &sectionSize);

        // Tile count based on section size
        int tileCount = sectionSize / context.tileLength;

        // Allocate tilesheet
        if (allocTilesheet(&context, tileCount))
            return 0;

        if (context.sheet == NULL)
            return 0;

        // Process raw tiles directly
        processTilesInSection(&context, sectionData, sectionSize);

        // Finalize section processing
        finalizeSection(&context, tileCount);
    } else {
        // Handle unknown compression types
        printf("Error: Unknown compression type \"%s\".\n", compressionType);
        return 0;
    }

    return 1;
}






/*
 * DM2 Graphics.dat Directory Entry Structure
 * Based on documentation at:
 * http://dmweb.free.fr/community/documentation/file-formats/dungeon-master-ii-data-files-notes/
 * 
 * Directory format:
 * - Offset 0x00-0x13: Header (20 bytes)
 * - Offset 0x00-0x01: Entry count (big-endian word)
 * - Offset 0x14: First entry starts
 * - Each entry: 8 bytes
 *   * Bytes 0-3: Offset (big-endian dword)
 *   * Bytes 4-7: Size (big-endian dword)
 */
typedef struct {
    uint32_t offset;
    uint32_t size;
} Dm2Entry;

int ripDm2GraphicsDat(Rom* rom, ExtractionArguments* arguments) {
    if (!rom || !rom->data || rom->size < 0x14) {
        printf("Error: Invalid ROM data for DM2 extraction.\n");
        return 0;
    }

    const uint8_t* data = (const uint8_t*)rom->data;
    uint16_t entryCount = (data[0] << 8) | data[1];

    if (entryCount == 0 || entryCount > 10000) {
        printf("Error: Invalid entry count: %d\n", entryCount);
        return 0;
    }

    printf("DM2: Found %d entries in graphics.dat\n", entryCount);

    const uint8_t* directory = data + 0x14;
    const char* outFolder = (arguments && arguments->outputFolder) ? arguments->outputFolder : outputFolder;
    if (!outFolder) {
        printf("Error: Output folder not set.\n");
        return 0;
    }

    // Process each entry
    for (uint16_t i = 0; i < entryCount; i++) {
        const uint8_t* entryData = directory + (i * 8);
        uint32_t entryOffset = entryData[0] | (entryData[1] << 8) | (entryData[2] << 16) | (entryData[3] << 24);
        uint32_t entrySize = entryData[4] | (entryData[5] << 8) | (entryData[6] << 16) | (entryData[7] << 24);

        printf("Processing Entry %d: Size %u, Offset %u\n", i, entrySize, entryOffset);

        if (entrySize == 0 || entryOffset + entrySize > rom->size) {
            printf("Skipping Entry %d: Invalid size or offset\n", i);
            continue;
        }

        const uint8_t* imgData = data + entryOffset;

        int formatType = 0;
        int width = 0, height = 0, bpp = 0;
        const uint8_t* compressedBody = NULL;
        size_t compressedBodySize = 0;

        // Try C4 format first (7-byte header)
        if (entrySize >= 7) {
            uint16_t widthRaw = imgData[0] | (imgData[1] << 8);
            uint16_t heightRaw = imgData[2] | (imgData[3] << 8);
            width = widthRaw & 0x3FF;  // Lower 10 bits
            height = heightRaw & 0x3FF; // Lower 10 bits

            // Check if this looks like C4 format (reasonable width and height)
            if (width >= 8 && height >= 8 && width <= 1024 && height <= 1024) {
                formatType = 1; // C4
                bpp = 4;
                compressedBody = imgData + 7;
                compressedBodySize = entrySize - 7;
            }
        }

        // If not C4, try C8 format (8-byte header)
        if (formatType == 0 && entrySize >= 8) {
            uint16_t widthRaw = imgData[0] | (imgData[1] << 8);
            uint16_t heightRaw = imgData[2] | (imgData[3] << 8);
            if (widthRaw > 0 && widthRaw <= 1024 && heightRaw > 0 && heightRaw <= 1024) {
                formatType = 2; // C8
                width = widthRaw;
                height = heightRaw;
                bpp = 8;
                compressedBody = imgData + 8;
                compressedBodySize = entrySize - 8;
            }
        }

        // Handle unknown formats
        if (formatType == 0 || width == 0 || height == 0) {
            printf("Skipping Entry %d: Unknown format\n", i);
            continue;
        }

        // Decompress the body (if compressed)
        const uint8_t* pixelData = compressedBody;
        size_t pixelDataSize = compressedBodySize;

        // Decompression for C4, C8
        if (formatType != 3) {
            Result* decompressed = decompressDm2(compressedBody, compressedBodySize);
            if (!decompressed || !decompressed->output || decompressed->size == 0) {
                printf("Skipping Entry %d: Decompression failed\n", i);
                continue;
            }
            pixelData = decompressed->output;
            pixelDataSize = decompressed->size;
            free(decompressed->output);
            free(decompressed);
        }

        // Allocate RGBA buffer
        unsigned char* rgbaData = malloc(width * height * 4);
        if (!rgbaData) {
            printf("Skipping Entry %d: Memory allocation failed\n", i);
            continue;
        }

        // Initialize to transparent black
        memset(rgbaData, 0, width * height * 4);

        // Convert pixel data to RGBA
        if (bpp == 4) {
            int scanlinePitch = (width + 1) / 2;
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int byteIndex = y * scanlinePitch + (x / 2);
                    if (byteIndex >= pixelDataSize) continue;

                    uint8_t pixelValue;
                    if ((x % 2) == 0) {
                        pixelValue = (pixelData[byteIndex] >> 4) & 0x0F;
                    } else {
                        pixelValue = pixelData[byteIndex] & 0x0F;
                    }

                    int rgbaIndex = (y * width + x) * 4;
                    rgbaData[rgbaIndex + 0] = paletteData[pixelValue * 4 + 0];
                    rgbaData[rgbaIndex + 1] = paletteData[pixelValue * 4 + 1];
                    rgbaData[rgbaIndex + 2] = paletteData[pixelValue * 4 + 2];
                    rgbaData[rgbaIndex + 3] = 255; // Fully opaque
                }
            }
        } else if (bpp == 8) {
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int pixelIndex = y * width + x;
                    if (pixelIndex >= pixelDataSize) continue;

                    uint8_t pixelValue = pixelData[pixelIndex];

                    int rgbaIndex = (y * width + x) * 4;
                    int paletteIndex = (pixelValue % 16) * 4;
                    rgbaData[rgbaIndex + 0] = paletteData[paletteIndex + 0];
                    rgbaData[rgbaIndex + 1] = paletteData[paletteIndex + 1];
                    rgbaData[rgbaIndex + 2] = paletteData[paletteIndex + 2];
                    rgbaData[rgbaIndex + 3] = 255;
                }
            }
        }

        // Save as PNG
        char filename[512];
        snprintf(filename, sizeof(filename), "%s%d.png", outFolder, i);
        if (!stbi_write_png(filename, width, height, 4, rgbaData, width * 4)) {
            printf("Failed to save PNG for Entry %d\n", i);
        } else {
            printf("Saved Entry %d to %s\n", i, filename);
        }

        free(rgbaData);
    }

    return 1;
}
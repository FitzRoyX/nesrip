#include <stdio.h>
#include <string.h>
#include <malloc.h>
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
	else if (strcmp(args->bitplaneType, "2") == 0) {
		context->bitplaneType = TWO_BPP;
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

uint64_t getLineData(ExtractionContext* context, unsigned char* sectionData, int y)
{
	switch (context->bitplaneType) {
		case ONE_BPP:
			return sectionData[y];
		case TWO_BPP:
			return (uint64_t)sectionData[y]
				 | ((uint64_t)sectionData[y + 8] << 8);
		case TWO_BPP_SNES:
			return (uint64_t)sectionData[y * 2]
				 | ((uint64_t)sectionData[y * 2 + 1] << 8);
		case THREE_BPP_SNES:
			return (uint64_t)sectionData[y * 2]
				 | ((uint64_t)sectionData[y * 2 + 1] << 8)
				 | ((uint64_t)sectionData[y + 16] << 16);
		case THREE_BPP_SNES_MODE7:
			uint32_t val = (uint32_t)sectionData[y * 3 + 2]
			 | ((uint32_t)sectionData[y * 3 + 1] << 8)
			 | ((uint32_t)sectionData[y * 3 + 0] << 16);
			uint64_t out = 0;
			for (int k = 0; k < 8; k++) {
				uint32_t bits = (val >> (3 * k)) & 7; // extract 3 bits
				out |= (uint64_t)(bits & 1) << (0  + k); // plane 0
				out |= (uint64_t)(bits & 2) << (7  + k); // plane 1
				out |= (uint64_t)(bits & 4) << (14 + k); // plane 2
			}
			return out;
		case FOUR_BPP_SNES:
			return  (uint64_t)sectionData[y * 2 + 0] // p0
				 | ((uint64_t)sectionData[y * 2 + 16] << 8) // p2
				 | ((uint64_t)sectionData[y * 2 + 1]  << 16)// p1
				 | ((uint64_t)sectionData[y * 2 + 17] << 24);// p3
		case EIGHT_BPP_SNES_MODE7:
			return ((uint64_t)sectionData[y * 8 + 0] << 56)
				 | ((uint64_t)sectionData[y * 8 + 1] << 48)
				 | ((uint64_t)sectionData[y * 8 + 2] << 40)
				 | ((uint64_t)sectionData[y * 8 + 3] << 32)
				 | ((uint64_t)sectionData[y * 8 + 4] << 24)
				 | ((uint64_t)sectionData[y * 8 + 5] << 16)
				 | ((uint64_t)sectionData[y * 8 + 6] << 8)
				 | ((uint64_t)sectionData[y * 8 + 7]);
		case EIGHT_BPP_SNES:
			/* TODO: planar 8bpp if needed */
			return 0;
		default:
			return 0;
	}
}

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

int ripSection(Rom* rom, ExtractionArguments* arguments) {
	ExtractionContext context = {
		rom,
		arguments
	};

	printf("Ripping section %s to %s.\n",
		   context.args->sectionStartString,
		   context.args->sectionEndString);

	if (!getSectionDetails(rom, &context))
		return 0;

	int tileCount = 0;
	unsigned char* sectionData = (unsigned char*)rom->data + context.sectionStart;
	unsigned char* endPointer   = (unsigned char*)rom->data + context.sectionEnd;

	const char* compressionType = context.args->compressionType;

	//
	// =====================================================================
	//  COMPRESSED BRANCH
	// =====================================================================
	//
	if (strcmp(compressionType, "rle_konami") == 0 ||
		strcmp(compressionType, "lzss") == 0 ||
		strcmp(compressionType, "lz1") == 0 ||
		strcmp(compressionType, "lz2") == 0 ||
		strcmp(compressionType, "lz2le") == 0 ||
		strcmp(compressionType, "lz3") == 0)
	{
		size_t sectionSize = context.sectionEnd - context.sectionStart + 1;
		Result* decompressedData = NULL;

		// --- dispatch correct decompressor ---
		if (strcmp(compressionType, "rle_konami") == 0) {
			decompressedData = decompressRleKonami(sectionData, sectionSize);
		} else if (strcmp(compressionType, "rle_smet") == 0) {
			decompressedData = decompressRleSmet(sectionData, sectionSize);
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
		}

		if (!decompressedData) {
			printf("Error: Failed to decompress the section.\n");
			return 0;
		}

		// tile count based on REAL tile size
		tileCount = decompressedData->size / context.tileLength;
		if (decompressedData->size % context.tileLength != 0) {
			printf("Warning: Decompressed data is not a multiple of %d bytes, "
				   "truncating the extra bytes.\n",
				   context.tileLength);
		}

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

		// process tiles
		unsigned char* decompressedPtr = decompressedData->output;
		unsigned char* decompressedEnd = decompressedData->output + decompressedData->size;

		while (decompressedPtr < decompressedEnd) {
			if (context.deduplicator) {
				context.workingHash = 0;
				processTile(&context, decompressedPtr, &addToHash);

				if (!checkHasTileMatch(&context, decompressedPtr, context.workingHash))
					processTile(&context, decompressedPtr, &writeLine);
				else
					drawDeduplicatedTile(&context);
			} else {
				processTile(&context, decompressedPtr, &writeLine);
			}

			incrementTilePos(&context);
			decompressedPtr += context.tileLength;
		}

		free(decompressedData->output);
		free(decompressedData);
	}

	//
	// =====================================================================
	//  RAW BRANCH
	// =====================================================================
	//
	else if (strcmp(compressionType, "raw") == 0)
	{
		size_t sectionSize = (context.sectionEnd - context.sectionStart) + 1;

		if (sectionSize % context.tileLength != 0) {
			printf("Warning: Targeted section has some extra bytes that cannot "
				   "be used to make a full tile.\n Rounding down section end address.\n");

			context.sectionEnd -= (sectionSize % context.tileLength);
			sectionSize = (context.sectionEnd - context.sectionStart) + 1;
		}

		tileCount = sectionSize / context.tileLength;

		if (allocTilesheet(&context, tileCount))
			return 0;

		if (context.sheet == NULL)
			return 0;

		while (sectionData <= endPointer - (context.tileLength - 1)) {
			if (context.deduplicator) {
				context.workingHash = 0;
				processTile(&context, sectionData, &addToHash);

				if (!checkHasTileMatch(&context, sectionData, context.workingHash))
					processTile(&context, sectionData, &writeLine);
				else
					drawDeduplicatedTile(&context);
			} else {
				processTile(&context, sectionData, &writeLine);
			}

			incrementTilePos(&context);
			sectionData += context.tileLength;
		}
	}

	//
	// =====================================================================
	//  UNKNOWN COMPRESSION
	// =====================================================================
	//
	else {
		printf("Error: Unknown compression type \"%s\".\n", compressionType);
		return 0;
	}

	//
	// =====================================================================
	//  FINAL OUTPUT + CLEANUP
	// =====================================================================
	//
	colorSheetIndex += tileCount;

	appendSectionToOutput(context.sheet, 128, context.maxY + 1);

	free(context.sheet);

	// cleanup pattern chains if decompressed formats were used
	if (strcmp(compressionType, "rle_konami") == 0 ||
		strcmp(compressionType, "rle_smet") == 0 ||
		strcmp(compressionType, "lzss") == 0 ||
		strcmp(compressionType, "lz1") == 0 ||
		strcmp(compressionType, "lz2") == 0 ||
		strcmp(compressionType, "lz2le") == 0 ||
		strcmp(compressionType, "lz3") == 0)
	{
		cleanupPatternChains();
		initPatternChains();
	}

	return 1;
}

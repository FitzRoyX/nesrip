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
	0, 0, 0, 255, //black
	255, 255, 255, 255, //white
	175, 175, 175, 255, //lgray
	100, 100, 100, 255, //dgray
//
	150, 0, 0, 255, //dred
	200, 0, 0, 255, //lred
	0, 150, 0, 255, //dgreen
	0, 200, 0, 255, //lgreen
//
	0, 0, 150, 255, //dblue
	0, 0, 200, 255, //lblue
	0, 150, 150, 255, //dteal
	0, 200, 200, 255, //lteal
//
	150, 150, 0, 255, //dyellow
	200, 200, 0, 255, //lyellow
	150, 0, 150, 255, //dpurple
	200, 0, 200, 255, //lpurple
};

char deduplicatedTileColor[] = {
	255, 0, 255, 255 //magenta
};

typedef enum {
	ONE_BPP,
	TWO_BPP,
	TWO_BPP_SNES,
	THREE_BPP_SNES,
	FOUR_BPP_SNES,
	FIVE_BPP,
	SIX_BPP,
	SEVEN_BPP,
	EIGHT_BPP_SNES,
	BPP_COUNT
} BitplaneType;

typedef struct Pattern {
	unsigned char* data;
	unsigned int hash;
	struct Pattern *next;
	struct Pattern *down;
} Pattern;

typedef struct {
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

Pattern* patterns[BPP_COUNT];
ColorizerPalette palettes[MAX_PALETTES];
ColorizerSheet colorSheet;
int colorSheetIndex = 0;

int allocTilesheet(ExtractionContext* context, int tileCount) {
	int width = 128;
	int height = (tileCount / (16 * context->patternSize)) * context->patternSize * 8;
	if (tileCount % (16 * context->patternSize) != 0)
		height += context->patternSize * 8;
	context->sheet = malloc(width * height * 4);
	if (context->sheet == NULL) {
		printf("Error: Couldn't allocate memory for tilesheet.\n");
		return 1;
	}
	memset(context->sheet, 0, width * height * 4);
	return 0;
}

void drawPixel(char* sheet, int x, int y, char* color) {
	for (int i = 0; i < 4; i++)	{
		sheet[(y * 128 + x) * 4 + i] = color[i];
	}
}

char* allocOverloadedFilename(ExtractionContext* context) {
	ExtractionArguments* args = context->args;
	size_t lenOutputFolder = strlen(args->outputFolder);
	size_t lenFilename = strlen("0");
	char* result = malloc(lenOutputFolder + lenFilename);
	if (result == NULL) {
		printf("Error: Couldn't allocate memory for filename data.\n");
		return NULL;
	}
	char* outputFilenamePtr = result;
	memcpy(outputFilenamePtr, args->outputFolder, lenOutputFolder);
	outputFilenamePtr += lenOutputFolder;
	memcpy(outputFilenamePtr, "0", lenFilename);
	outputFilenamePtr += lenFilename;
	*(outputFilenamePtr++) = '.';
	memcpy(outputFilenamePtr, ".png", 4);
	outputFilenamePtr += 4;
	outputFilenamePtr[0] = 0;
	return result;
}

int writeOutput(char* outputData, int width, int height, ExtractionContext* context) {
	char* outputFilename = NULL;
	if (context->args->filenameOverload != NULL)
		outputFilename = allocOverloadedFilename(context);
	if (outputFilename == NULL)
		return 0;
	printf("  Writing sheet to \"");
	printf("%s", outputFilename);
	printf("\".\n");
	if (!stbi_write_png(outputFilename, width, height, 4, outputData, 128 * 4)) {
		printf("An error occurred while writing to output file ");
		printf("%s", outputFilename);
		printf(".\n");
		free(outputFilename);
		return 0;
	}
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
	else if (strcmp(args->bitplaneType, "2snes") == 0) {
		context->bitplaneType = TWO_BPP_SNES;
		context->tileLength = 16;
	}
	else if (strcmp(args->bitplaneType, "3") == 0) {
		context->bitplaneType = THREE_BPP_SNES;
		context->tileLength = 24;
	}
	else if (strcmp(args->bitplaneType, "4") == 0) {
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
	else if (strcmp(args->bitplaneType, "8") == 0) {
		context->bitplaneType = EIGHT_BPP_SNES;
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
	if (context->stx < context->patternSize)
		return;
	context->stx = 0;
	context->sty++;
	if (context->sty < context->patternSize)
		return;
	context->sty = 0;
	context->tx++;
	if (context->tx < 16 / context->patternSize)
		return;
	context->tx = 0;
	context->ty++;
}

int writeLine(ExtractionContext* context, int y, unsigned int data) {
	int stx = (context->patternDirection) ? context->sty : context->stx;
	int sty = (context->patternDirection) ? context->stx : context->sty;
	int px;
	int py = (context->ty * context->patternSize + sty) * 8 + y;
	for (int x = 0; x < 8; x++)	{
		char c = (((data & 0x01000000) >> 21) | ((data & 0x00010000) >> 14) | ((data & 0x00000100) >> 7) | (data & 0x00000001));
		data = (data >> 1) & 0x7F7F7F7F;
		px = (context->tx * context->patternSize + stx) * 8 + 7 - x;
		char color[4];
		int colorIndex = c * 4;
		for (int i = 0; i < 4; i++) {
			color[i] = paletteData[colorIndex + i];
		}
		ColorizerSheet* sheet = &colorSheet;
		int cx = (colorSheetIndex + context->index) % 16;
		int cy = (colorSheetIndex + context->index) / 16;
		if (sheet->valid && cx < sheet->width && cy < sheet->height) {
			ColorizerPalette* palette = &palettes[sheet->tiles[cy * sheet->width + cx]];
			if (palette->valid) {
				// Fix: Cast 'c' to unsigned int to avoid char subscript warning
				memcpy(color, palette->colors[(unsigned int)c], sizeof(color));
			}
		}
		drawPixel(context->sheet, px, py, color);
	}
	if (px + 7 > context->maxX)
		context->maxX = px + 7;
	if (py > context->maxY)
		context->maxY = py;
	return 0;
}

int addToHash(ExtractionContext* context, int y, unsigned int data) {
	context->workingHash ^= data;
	return 0;
}

unsigned int getLineData(ExtractionContext* context, unsigned char* sectionData, int y) {
	unsigned int data = 0;
	switch (context->bitplaneType) {
		case ONE_BPP:
			data = sectionData[y];
			break;
		case TWO_BPP:
			data = (sectionData[y] | (sectionData[y + 8] << 8));
			break;
		case TWO_BPP_SNES:
			data = (sectionData[y * 2] | (sectionData[y * 2 + 1] << 8));
			break;
		case THREE_BPP_SNES:
			data = (sectionData[y * 2] | (sectionData[y * 2 + 1] << 8));
			break;
		case FOUR_BPP_SNES: {
			unsigned int lowerNibble1 = sectionData[y * 2];
			unsigned int lowerNibble2 = sectionData[y * 2 + 1];
			unsigned int upperNibble1 = sectionData[y * 2 + 16];
			unsigned int upperNibble2 = sectionData[y * 2 + 17];
			data = (upperNibble1 << 8) | lowerNibble1;
			data |= (upperNibble2 << 24) | (lowerNibble2 << 16);
			break;
		}
		default:
			break;
	}
	return data;
}

void processTile(ExtractionContext* context, unsigned char* sectionData, int(*callback)(ExtractionContext*,int,unsigned int)) {
	for (int y = 0; y < 8; y++)	{
		unsigned int data = getLineData(context, sectionData, y);
		if ((*callback)(context, y, data)) return;
	}
}

int isTileMatch(ExtractionContext* context, unsigned char* tileDataA, unsigned char* tileDataB) {
	for (int y = 0; y < 8; y++)	{
		unsigned int lineA, lineB;
		lineA = getLineData(context, tileDataA, y);
		lineB = getLineData(context, tileDataB, y);
		if (lineA != lineB)
			return 0;
	}
	return 1;
}

int checkHasTileMatch(ExtractionContext* context, unsigned char* tileData, unsigned int hash) {
	Pattern* pattern = (Pattern*)malloc(sizeof(Pattern));
	if (pattern == NULL) {
		perror("Error: Couldn't allocate memory for pattern data.\n");
		exit(EXIT_FAILURE);
	}
	pattern->data = tileData;
	pattern->hash = hash;
	pattern->next = NULL;
	pattern->down = NULL;
	Pattern* hashChainStartPointer = patterns[context->bitplaneType];
	if (hashChainStartPointer == NULL) {
		patterns[context->bitplaneType] = pattern;
		return 0;
	}
	Pattern* hashChainPointer = hashChainStartPointer;
	Pattern* previousHashChainPointer = NULL;
	Pattern* downStartPointer = NULL;
	while (true) {
		if (hashChainPointer->hash == hash) {
			downStartPointer = hashChainPointer;
			break;
		}
		if (hashChainPointer->next == NULL)
			break;
		previousHashChainPointer = hashChainPointer;
		hashChainPointer = hashChainPointer->next;
	}
	if (downStartPointer == NULL) {
		pattern->next = hashChainStartPointer;
		patterns[context->bitplaneType] = pattern;
		return 0;
	}
	Pattern* downPointer = downStartPointer;
	do {
		if (isTileMatch(context, tileData, downPointer->data)) {
			free(pattern);
			return 1;
		}
		downPointer = downPointer->down;
	} while (downPointer != NULL);
	if (downPointer == NULL) {
		pattern->down = downStartPointer;
		pattern->next = downStartPointer->next;
		if (previousHashChainPointer != NULL)
			previousHashChainPointer->next = pattern;
		else
			patterns[context->bitplaneType] = pattern;
	}
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
	int px, py;
	for (int y = 0; y < 8; y++) {
		py = (context->ty * context->patternSize + sty) * 8 + y;
		for (int x = 0; x < 8; x++) {
			px = (context->tx * context->patternSize + stx) * 8 + 7 - x;
			drawPixel(context->sheet, px, py, deduplicatedTileColor);
		}
		if (px + 7 > context->maxX) {
			context->maxX = px + 7;
		}
		if (py > context->maxY) {
			context->maxY = py;
		}
	}
}

void initPatternChains() {
	for (int i = 0; i < BPP_COUNT; i++)	{
		patterns[i] = (Pattern*)NULL;
	}
}

void cleanupPatternChains() {
	for (int i = 0; i < BPP_COUNT; i++)	{
		Pattern* patternChainPointer = patterns[i];
		while (patternChainPointer != NULL) {
			Pattern* next;
			Pattern* downChainPointer = patternChainPointer->down;
			while (downChainPointer != NULL) {
				next = downChainPointer->down;
				free(downChainPointer);
				downChainPointer = next;
			}
			next = patternChainPointer->next;
			free(patternChainPointer);
			patternChainPointer = next;
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
    //  COMPRESSED BRANCH: rle_konami, lzss, lclz2
    // =====================================================================
    //
    if (strcmp(compressionType, "rle_konami") == 0 ||
        strcmp(compressionType, "lzss") == 0 ||
        strcmp(compressionType, "lclz2") == 0)
    {
        size_t sectionSize = context.sectionEnd - context.sectionStart + 1;
        Result* decompressedData = NULL;

        // --- dispatch correct decompressor ---
        if (strcmp(compressionType, "rle_konami") == 0) {
            decompressedData = decompressRleKonami(sectionData, sectionSize);
        } else if (strcmp(compressionType, "lzss") == 0) {
            decompressedData = decompressLzss(sectionData, sectionSize);
        } else if (strcmp(compressionType, "lclz2") == 0) {
            decompressedData = decompressLcLz2(sectionData, sectionSize);
        }

        if (!decompressedData) {
            printf("Error: Failed to decompress the section.\n");
            return 0;
        }

        // --- tile count based on REAL tile size ---
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

        // --- process tiles ---
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
        strcmp(compressionType, "lzss") == 0 ||
        strcmp(compressionType, "lclz2") == 0)
    {
        cleanupPatternChains();
        initPatternChains();
    }

    return 1;
}

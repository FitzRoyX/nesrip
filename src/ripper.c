#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "globals.h"
#include "logger.h"
#include "utils.h"
#include "ripper.h"

//  default grayscale bwld colors
//	0, 0, 0, 255,
//	255, 255, 255, 255,
//	178, 178, 178, 255,
//	107, 107, 107, 255

//  redscale bwor colors
//	0, 0, 0, 255,
//	255, 255, 255, 255,
//	255, 127, 0, 255,
//	190, 0, 0, 255

char paletteData[] = 
{
	0, 0, 0, 255,
	255, 255, 255, 255,
	178, 178, 178, 255,
	107, 107, 107, 255
};

char RedundantColor[] = 
{
	255, 0, 255, 255
};

typedef enum
{
	ONE_BPP,
	TWO_BPP,

	BPP_COUNT
} BitplaneType;

typedef struct Pattern 
{
	unsigned char* data;
	unsigned int hash;
	struct Pattern *next;
	struct Pattern *down;
} Pattern;

typedef struct
{
	Rom* rom;
	ExtractionArguments* args;
	int sectionStart;
	int sectionEnd;
	int patternSize;
	int patternDirection;
	BitplaneType bitplaneType;
	int checkRedundant;
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

int allocTilesheet(ExtractionContext* context, int tileCount)
{
	int width = 128;
	int height = (tileCount / (16 * context->patternSize)) * context->patternSize * 8;

	if (tileCount % (16 * context->patternSize) != 0)
		height += context->patternSize * 8;

	context->sheet = (char*)malloc(width * height * 4);

	if (context->sheet == NULL)
	{
		printf("Error: Couldn't allocate memory for tilesheet.\n");
		return 1;
	}

	memset(context->sheet, 0, width * height * 4);
	return 0;
}

char* getColor(char color, char* paletteDescription)
{
	char c = paletteDescription[color];

	switch (c)
	{
	case 'b':
		return paletteData;
	case 'w':
		return paletteData + 4;
	case 'l':
		return paletteData + 8;
	case 'd':
		return paletteData + 12;
	}
	
	return paletteData;
}

void drawPixel(char* sheet, int x, int y, char* color)
{
	for (int i = 0; i < 4; i++)
	{
		sheet[(y * 128 + x) * 4 + i] = color[i];
	}
}

char* allocOverloadedFilename(ExtractionContext* context) {
	ExtractionArguments* args = context->args;
	size_t lenOutputFolder = strlen(args->outputFolder);
	size_t lenFilename = strlen("0");
	//*size_t lenFilename = strlen(args->filenameOverload);*/
	size_t lenPaletteDescription = strlen(args->paletteDescription);

	char* result = (char*)malloc(lenOutputFolder + lenFilename + lenPaletteDescription + 6);

	if (result == NULL) {
		printf("Error: Couldn't allocate memory for filename data.\n");
		return NULL;
	}

	char* outputFilenamePtr = result;

	memcpy(outputFilenamePtr, args->outputFolder, lenOutputFolder);
	outputFilenamePtr += lenOutputFolder;
	memcpy(outputFilenamePtr, "0", lenFilename);
	//memcpy(outputFilenamePtr, args->filenameOverload, lenFilename);
	outputFilenamePtr += lenFilename;
	*(outputFilenamePtr++) = '.';
	memcpy(outputFilenamePtr, args->paletteDescription, lenPaletteDescription);
	outputFilenamePtr += lenPaletteDescription;
	memcpy(outputFilenamePtr, ".png", 4);
	outputFilenamePtr += 4;
	outputFilenamePtr[0] = 0;

	return result;
}

char* allocSectionFilename(ExtractionContext* context)
{
	ExtractionArguments* args = context->args;
	size_t lenOutputFolder = strlen(args->outputFolder);
	size_t lenSectionStart = strlen(args->sectionStartString);
	size_t lenSectionEnd = strlen(args->sectionEndString);
	size_t lenPaletteDescription = strlen(args->paletteDescription);

	char* result = (char*)malloc(lenOutputFolder + lenSectionStart + lenSectionEnd + lenPaletteDescription + 9);

	if (result == NULL)
	{
		printf("Error: Couldn't allocate memory for filename data.\n");
		return NULL;
	}

	char* outputFilenamePtr = result;

	memcpy(outputFilenamePtr, args->outputFolder, lenOutputFolder);
	outputFilenamePtr += lenOutputFolder;
	memcpy(outputFilenamePtr, args->sectionStartString, lenSectionStart);
	outputFilenamePtr += lenSectionStart;
	*(outputFilenamePtr++) = '_';
	memcpy(outputFilenamePtr, args->sectionEndString, lenSectionEnd);
	outputFilenamePtr += lenSectionEnd;
	*(outputFilenamePtr++) = '_';
	*(outputFilenamePtr++) = args->patternDirectionString[0];
	*(outputFilenamePtr++) = '.';
	memcpy(outputFilenamePtr, args->paletteDescription, lenPaletteDescription);
	outputFilenamePtr += lenPaletteDescription;
	memcpy(outputFilenamePtr, ".png", 4);
	outputFilenamePtr += 4;
	outputFilenamePtr[0] = 0;

	return result;
}

int writeOutput(char* outputData, int width, int height, ExtractionContext* context) {
	char* outputFilename = NULL;

	if (context->args->filenameOverload != NULL) {
		outputFilename = allocOverloadedFilename(context);
		//printf("AllocOverloaded Filename: %s", outputFilename);
	}
	else {
		outputFilename = allocSectionFilename(context);
		//printf("AllocSection Filename: %s", outputFilename);
	}

	if (outputFilename == NULL)
		return 0;

	printf("  Writing sheet to \"");
	printf("%s", outputFilename);
	printf("\".\n");

	if (!stbi_write_png(outputFilename, width, height, 4, outputData, 128 * 4)) {
		printf("An error occurred while writing to output file ");
		printf("%s", outputFilename);
		printf(".\n");
		return 0;
	}

	free(outputFilename);

	return 1;
}

int getSectionDetails(Rom* rom, ExtractionContext* context)
{
	ExtractionArguments* args = context->args;

	if (str2int(&(context->sectionStart), args->sectionStartString, 16) != STR2INT_SUCCESS || context->sectionStart < 0 || context->sectionStart >= rom->size)
	{
		printf("Error: Invalid section start address.\n");
		return 0;
	}


	if (str2int(&(context->sectionEnd), args->sectionEndString, 16) != STR2INT_SUCCESS || context->sectionEnd < 0 || context->sectionEnd >= rom->size)
	{
		printf("Error: Invalid section end address.\n");
		return 0;
	}

	if (str2int(&(context->patternSize), args->patternSizeString, 10) != STR2INT_SUCCESS || numberOfSetBits(context->patternSize) > 1 || (unsigned int)context->patternSize > MAX_PATTERN_SIZE)
	{
		printf("Error: Invalid pattern size.\n");
		return 0;
	}

	if (context->sectionEnd < context->sectionStart)
	{
		printf("Error: Section end is placed before start.\n");
		return 0;
	}

	if (strcmp(args->patternDirectionString, "h") == 0)
		context->patternDirection = false;
	else if (strcmp(args->patternDirectionString, "v") == 0)
		context->patternDirection = true;
	else
		printf("Error: Invalid pattern direction. Use \"h\" or \"v\".\n");

	if (strcmp(args->bitplaneType, "1") == 0)
	{
		context->bitplaneType = ONE_BPP;
		context->tileLength = 8;
	}
	else if (strcmp(args->bitplaneType, "2") == 0)
	{
		context->bitplaneType = TWO_BPP;
		context->tileLength = 16;
	}

	if (strcmp(args->checkRedundant, "true") == 0)
	{
		context->checkRedundant = true;
	}
	else if (strcmp(args->checkRedundant, "false") == 0)
	{
		context->checkRedundant = false;
	}
	else
	{
		printf("Error: Invalid Redundant check value. Use \"true\" or \"false\". Defaulting to \"true\".\n");
		context->checkRedundant = true;
	}

	return 1;
}

void incrementTilePos(ExtractionContext* context)
{
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

int writeLine(ExtractionContext* context, int y, unsigned int data)
{
	int stx = (context->patternDirection) ? context->sty : context->stx;
	int sty = (context->patternDirection) ? context->stx : context->sty;

	int px;
	int py = (context->ty * context->patternSize + sty) * 8 + y;

	for (int x = 0; x < 8; x++)
	{
		char c = (((data & 0x01000000) >> 21) | ((data & 0x00010000) >> 14) | ((data & 0x00000100) >> 7) | (data & 0x00000001));
		data = (data >> 1) & 0x7F7F7F7F;

		px = (context->tx * context->patternSize + stx) * 8 + 7 - x;

		char color[4];
		memcpy(color, getColor(c, context->args->paletteDescription), sizeof(color));

		ColorizerSheet* sheet = &colorSheet;
		int cx = (colorSheetIndex + context->index) % 16;
		int cy = (colorSheetIndex + context->index) / 16;
		if (sheet->valid && cx < sheet->width && cy < sheet->height)
		{
			ColorizerPalette* palette = &palettes[sheet->tiles[cy * sheet->width + cx]];
			if (palette->valid)
			{
				memcpy(color, palette->colors[c], sizeof(color));
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

int addToHash(ExtractionContext* context, int y, unsigned int data)
{
	context->workingHash ^= data;
	return 0;
}

unsigned int getLineData(ExtractionContext* context, unsigned char* sectionData, int y)
{
	int data = 0;

	switch (context->bitplaneType) {
		case ONE_BPP:
			data = sectionData[y];
			break;
		case TWO_BPP:
			data = (sectionData[y] | (sectionData[y + 8] << 8));
			break;
		default:
			break;
	}

	return data;
}

void processTile(ExtractionContext* context, unsigned char* sectionData, int(*callback)(ExtractionContext*,int,unsigned int))
{
	for (int y = 0; y < 8; y++)
	{
		unsigned int data = getLineData(context, sectionData, y);
		if ((*callback)(context, y, data)) return;
	}
}

int isTileMatch(ExtractionContext* context, unsigned char* tileDataA, unsigned char* tileDataB)
{
	for (int y = 0; y < 8; y++)
	{
		unsigned int lineA, lineB;
		lineA = getLineData(context, tileDataA, y);
		lineB = getLineData(context, tileDataB, y);

		if (lineA != lineB)
			return 0;
	}

	return 1;
}

int checkHasTileMatch(ExtractionContext* context, unsigned char* tileData, unsigned int hash)
{
	Pattern* pattern = (Pattern*)malloc(sizeof(Pattern));
	pattern->data = tileData;
	pattern->hash = hash;
	pattern->next = NULL;
	pattern->down = NULL;

	Pattern* hashChainStartPointer = patterns[context->bitplaneType];

	if (hashChainStartPointer == NULL)
	{
		patterns[context->bitplaneType] = pattern;
		return 0;
	}

	Pattern* hashChainPointer = hashChainStartPointer;
	Pattern* previousHashChainPointer = NULL;
	Pattern* downStartPointer = NULL;

	while (true)
	{
		if (hashChainPointer->hash == hash)
		{
			downStartPointer = hashChainPointer;
			break;
		}
		
		if (hashChainPointer->next == NULL)
			break;

		previousHashChainPointer = hashChainPointer;
		hashChainPointer = hashChainPointer->next;
	};

	if (downStartPointer == NULL)
	{
		pattern->next = hashChainStartPointer;
		patterns[context->bitplaneType] = pattern;
		return 0;
	}

	Pattern* downPointer = downStartPointer;

	do
	{
		if (isTileMatch(context, tileData, downPointer->data))
		{
			free(pattern);
			return 1;
		}

		downPointer = downPointer->down;
	} while (downPointer != NULL);

	if (downPointer == NULL)
	{
		pattern->down = downStartPointer;
		pattern->next = downStartPointer->next;
		if (previousHashChainPointer != NULL)
			previousHashChainPointer->next = pattern;
		else
			patterns[context->bitplaneType] = pattern;
	}

	return 0;
}

void drawRedundantTile(ExtractionContext* context)
{
	int stx = (context->patternDirection) ? context->sty : context->stx;
	int sty = (context->patternDirection) ? context->stx : context->sty;

	int px, py;
	for (int y = 0; y < 8; y++)
	{
		py = (context->ty * context->patternSize + sty) * 8 + y;

		for (int x = 0; x < 8; x++)
		{
			px = (context->tx * context->patternSize + stx) * 8 + 7 - x;
			drawPixel(context->sheet, px, py, RedundantColor);
		}

		if (px + 7 > context->maxX)
			context->maxX = px + 7;

		if (py > context->maxY)
			context->maxY = py;
	}
}

int ripSectionRaw(Rom* rom, Cache* cache, ExtractionContext* context) {
	printf("Ripping raw section from ");
	printf("%s", context->args->sectionStartString);
	printf(" to ");
	printf("%s", context->args->sectionEndString);
	printf(" in ROM.\n");

	if (!getSectionDetails(rom, context))
		return 0;

	if (((context->sectionEnd - context->sectionStart) + 1) % context->tileLength != 0)
	{
		printf("Warning: Targeted section has some extra bytes that cannot be used to make a full tile.\n         Rounding down section end address.\n");
		context->sectionEnd -= (context->sectionEnd - context->sectionStart + 1) % context->tileLength;
	}

	int tileCount = (context->sectionEnd - context->sectionStart + 1) / context->tileLength;

	if (allocTilesheet(context, tileCount))
		return 0;

	if (context->sheet == NULL)
		return 0;

	//int sheetCount = 0;
	//int tx = 0;
	//int ty = 0;
	//int stx = 0;
	//int sty = 0;

	unsigned char* sectionData = rom->data + context->sectionStart;
	unsigned char* endPointer = rom->data + context->sectionEnd;

	while (sectionData < endPointer) {
		if (context->checkRedundant) {
			context->workingHash = 0;
			processTile(context, sectionData, &addToHash);

			if (!checkHasTileMatch(context, sectionData, context->workingHash))
				processTile(context, sectionData, &writeLine);
			else
				drawRedundantTile(context);
		} else {
			processTile(context, sectionData, &writeLine);
		}

		incrementTilePos(context);
		sectionData += context->tileLength;
	}

	colorSheetIndex += tileCount;
	char* ctr = context->args->filenameOverload;
	char filename[15];
	snprintf(filename, sizeof(filename), "output/%s.png", ctr);
	addToCache(cache, context->sheet, context->maxX + 1, context->maxY + 1, 4, 128 * 4);
	stbi_write_png(filename, context->maxX + 1, context->maxY + 1, 4, context->sheet, 128 * 4);
	//writeOutput(context->sheet, context->maxX + 1, context->maxY + 1, context);
	free(context->sheet);

	return 1;
}

void initPatternChains()
{
	for (int i = 0; i < BPP_COUNT; i++)
	{
		patterns[i] = (Pattern*)NULL;
	}
}

void cleanupPatternChains()
{
	for (int i = 0; i < BPP_COUNT; i++)
	{
		Pattern* patternChainPointer = patterns[i];

		while (patternChainPointer != NULL)
		{
			Pattern* next;
			Pattern* downChainPointer = patternChainPointer->down;

			while (downChainPointer != NULL)
			{
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

int ripSection(Rom* rom, Cache* cache, ExtractionArguments* arguments) {
	ExtractionContext context = 
	{
		rom,
		arguments
	};

	if (strcmp(arguments->compressionType, "raw") != 0)	{
		printf("Error: Unknown compression type \"");
		printf("%s", arguments->compressionType);
		printf("\".\n");
		return 0;
		
	}

	if (!ripSectionRaw(rom, cache, &context)) {
		printf("An error occured during ripping.\n");
		return 0;
	}

	return 1;
}

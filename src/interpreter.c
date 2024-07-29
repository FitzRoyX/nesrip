#include <stdio.h>
#include <malloc.h>
#include "globals.h"
#include "logger.h"
#include "ripper.h"
#include "utils.h"
#include "sha_2/sha-256.h"

char* hashString;
char* tempHashString;
int foundRom = false;
char* tokenContextStack[2];
char** tokenContext = &tokenContextStack[0];

#define CHECK_COMMAND(command, callback) \
if (strcmp(token, command) == 0) \
{ \
	if (callback()) \
		break; \
	\
	token = updateToken(NULL); \
	continue; \
}

#define PULL_TOKEN(command, token) \
token = updateToken(NULL); \
\
if (token == NULL) \
{ \
	printInvalidDatabaseCommand(command, "File ended"); \
	return 1; \
}

char* updateToken(char* string)
{
	char* token = stringTokenize(string, " \n", tokenContext);
	return token;
}

void pushTokenContext(void)
{
	++tokenContext;
}

void popTokenContext(void)
{
	--tokenContext;
}

int removeTails(char* string, int length, const char* pattern)
{
	int removed = 0;
	char* match = strstr(string, pattern);

	while (match != NULL)
	{
		int length = strcspn(match, "\n");
		memmove(match, match + length, strlen(match + length));
		match = strstr(match, pattern);
		removed += length;
	}

	memset(string + length - removed - 1, 0, removed);
	return length - removed;
}

int removeCarriageReturns(char* string, int length)
{
	return removeTails(string, length, "\r");
}

int removeComments(char* string, int length)
{
	return removeTails(string, length, "//");
}

int handleColorizerPaletteCommand()
{
	while (true)
	{
		char* indexString, * name, * colors[4];
		PULL_TOKEN("Palette", indexString);

		if (strcmp(indexString, "end") == 0)
			break;

		PULL_TOKEN("Palette", name);
		PULL_TOKEN("Palette", colors[0]);
		PULL_TOKEN("Palette", colors[1]);
		PULL_TOKEN("Palette", colors[2]);
		PULL_TOKEN("Palette", colors[3]);

		int index;
		if (str2int(&index, indexString, 0) != STR2INT_SUCCESS || index < 0 || index >= MAX_PALETTES)
		{
			printf("Error: Palette index out of range.\n");
			return 1;
		}

		ColorizerPalette* palette = &palettes[index];
		palette->valid = true;
		palette->name = strdup(name);
		for (int i = 0; i < 4; i++)
		{
			int color;
			str2int(&color, colors[i], 16);
			palette->colors[i][0] = color >> 16 & 255;
			palette->colors[i][1] = color >> 8 & 255;
			palette->colors[i][2] = color & 255;
			palette->colors[i][3] = 255;
		};
	}

	return 0;
}

int handleColorizerSheetCommand()
{
	ColorizerSheet* sheet = &colorSheet;
	sheet->valid = true;
	sheet->width = 16;
	sheet->height = MAX_COLOR_ROWS;
	sheet->tiles = malloc(sheet->width * sheet->height);

	for (int y = 0; y < MAX_COLOR_ROWS; y++)
	{
		for (int x = 0; x < sheet->width; x++)
		{
			char* paletteString;
			PULL_TOKEN("Sheet", paletteString);

			if (strcmp(paletteString, "end") == 0)
				goto end;

			int palette;
			if (str2int(&palette, paletteString, 0) != STR2INT_SUCCESS || palette < 0 || palette >= MAX_PALETTES)
			{
				printf("Error: Palette index out of range.\n");
				return 1;
			}

			sheet->height = y + 1;
			sheet->tiles[y * sheet->width + x] = palette;
		}
	}

end:
	return 0;
}

void interpretColorizer(char* colorizerFilename)
{
	char* database;
	int databaseLength = readAllBytesFromFile(colorizerFilename, &database, true);
	if (databaseLength <= 0)
		return;

	databaseLength = removeCarriageReturns(database, databaseLength);
	databaseLength = removeComments(database, databaseLength);

	pushTokenContext();

	char* token = updateToken(database);
	while (token != NULL)
	{
		CHECK_COMMAND("p", handleColorizerPaletteCommand);
		CHECK_COMMAND("s", handleColorizerSheetCommand);

		if (strcmp(token, "end") == 0)
			break;

		printf("Invalid database token: ");
		printf(token);
		printf("\n");
		break;
	}

	popTokenContext();

	free(database);
}

int handleHashCommand()
{
	char* token;
	PULL_TOKEN("Hash", token);

	if (strlen(token) < SIZE_OF_SHA_256_HASH * 2)
	{
		printf("Warning: Matching hash \"");
		printf(token);
		printf("\"  is too small.\n");
		return 0;
	}

	memcpy(tempHashString, token, SIZE_OF_SHA_256_HASH * 2);
	tempHashString[SIZE_OF_SHA_256_HASH * 2] = 0;

	strupr(tempHashString);

	if (strcmp(hashString, tempHashString) == 0)
	{
		printf("Matched hash!\n");
		foundRom = true;

		char colorizerFilename[32];
		sprintf(colorizerFilename, "colorizer/%.8s.txt", hashString);
		interpretColorizer(colorizerFilename);
	}

	return 0;
}

int handlePatternCommand()
{
	char* size, * direction;
	PULL_TOKEN("Pattern", size);
	PULL_TOKEN("Pattern", direction);

	if (patternOverride)
		return 0;

	patternSize = size;
	patternDirection = direction;

	return 0;
}

int handlePaletteCommand()
{
	char* token;
	PULL_TOKEN("Palette", token);

	if (paletteOverride)
		return 0;

	paletteDescription = token;
	return 0;
}

int handleSectionCommand()
{
	char* prefix, * sectionStart, * sectionEnd;

	PULL_TOKEN("Section", prefix);
	PULL_TOKEN("Section", sectionStart);
	PULL_TOKEN("Section", sectionEnd);

	ExtractionArguments args =
	{
		sectionStart,
		sectionEnd,
		patternSize,
		patternDirection,
		paletteDescription,
		compressionType,
		bitplaneType,
		checkRedundant,
		outputFolder,
		prefix
	};

	ripSection(&rom, &args);
	return 0;
}

int handleCompressionCommand()
{
	char* token;
	PULL_TOKEN("Compression", token);

	compressionType = token;
	return 0;
}

int handleBitplaneCommand()
{
	char* token;
	PULL_TOKEN("Bitplane", token);

	if (bitplaneOverride)
		return 0;

	bitplaneType = token;
	return 0;
}

int handleCheckRedundantCommand()
{
	char* token;
	PULL_TOKEN("CheckRedundant", token);

	if (checkRedundantOverride)
		return 0;

	checkRedundant = token;
	return 0;
}

int handleClearRedundantCommand()
{
	cleanupPatternChains();
	initPatternChains();
	return 0;
}

void interpretDatabase()
{
	initPatternChains();

	uint8_t hash[SIZE_OF_SHA_256_HASH];
	calc_sha_256(hash, rom.data, rom.size);

	hashString = (char*)malloc(sizeof(char) * SIZE_OF_SHA_256_HASH * 2 + 1);
	tempHashString = (char*)malloc(sizeof(char) * SIZE_OF_SHA_256_HASH * 2 + 1);

	if (hashString == NULL)
	{
		printf("Error: Couldn't allocate memory for ROM hash string.\n");
		return;
	}

	char* hashStringPtr = hashString;

	for (int i = 0; i < SIZE_OF_SHA_256_HASH; i++)
	{
		sprintf(hashStringPtr, "%02X", hash[i]);
		hashStringPtr += 2;
	}
	hashStringPtr[0] = 0;

	printf("ROM Hash: ");
	printf(hashString);
	printf("\n");

	char* database;
	int databaseLength = readAllBytesFromFile(descriptorFilename, &database, true);
	foundRom = false;

	databaseLength = removeCarriageReturns(database, databaseLength);
	databaseLength = removeComments(database, databaseLength);

	char* token = updateToken(database);
	while (token != NULL)
	{
		if (!foundRom)
		{
			if (strcmp(token, "hash") == 0)
			{
				if (handleHashCommand())
					break;
			}

			token = updateToken(NULL);
			continue;
		}

		CHECK_COMMAND("p", handlePatternCommand);
		CHECK_COMMAND("i", handlePaletteCommand);
		CHECK_COMMAND("c", handleCompressionCommand);
		CHECK_COMMAND("b", handleBitplaneCommand);
		CHECK_COMMAND("r", handleCheckRedundantCommand);
		CHECK_COMMAND("k", handleClearRedundantCommand);
		CHECK_COMMAND("s", handleSectionCommand);

		if (strcmp(token, "end") == 0)
			break;

		printf("Invalid database token: ");
		printf(token);
		printf("\n");
		break;
	}

	if (!foundRom)
		printf("Error: Could not match ROM with database.\n");

	cleanupPatternChains();
	free(hashString);
	free(tempHashString);
}

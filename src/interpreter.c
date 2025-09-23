#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "globals.h"
#include "logger.h"
#include "ripper.h"
#include "utils.h"
#include "sha_2/sha-256.h"
#include "interpreter.h"

int foundRom = false;
char* tokenContextStack[2];
char** tokenContext = &tokenContextStack[0];

#define CHECK_COMMAND(command, callback, cache) \
if (strcmp(token, command) == 0) \
{ \
	if (callback(cache)) \
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

char* updateToken(char* string) {
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

void removeTails(char* string, size_t length, const char* pattern) {
	char* pos;
	while((pos = strstr(string, pattern)) != NULL) {
		memmove(pos, pos + strlen(pattern), strlen(pos)); //shift rest of the string left by size of pattern
	}
}

size_t removeCarriageReturns(char* string, size_t length) {
	removeTails(string, length, "\r");
	return strlen(string);
}

size_t removeComments(char* string, size_t length) {
	size_t removed = 0;
	char pattern[] = "//";
	//char* result = removeString(string, pattern);
	char* match = strstr(string, pattern);
	size_t pos = match - string;
	char* ptr = match;

	while (ptr != NULL) {
		size_t size = strcspn(ptr, "\n");
		
		deleteCharacters(match, pos, size);
		ptr = strstr(match, pattern);
		pos = ptr - match;
		
		removed += size;
	}

	memset(string + length - removed, 0, removed);
	return length - removed;

}

int handleColorizerPaletteCommand(Cache* cache)
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
		palette->name = _strdup(name);
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

int handleColorizerSheetCommand(Cache* cache) {
	ColorizerSheet* sheet = &colorSheet;
	sheet->valid = true;
	sheet->width = 16;
	sheet->height = MAX_COLOR_ROWS;
	sheet->tiles = malloc(sheet->width * sheet->height);

	for (int y = 0; y < MAX_COLOR_ROWS; y++) {
		for (int x = 0; x < sheet->width; x++) {
			char* paletteString;
			PULL_TOKEN("Sheet", paletteString);

			if (strcmp(paletteString, "end") == 0)
				goto end;

			int palette;
			if (str2int(&palette, paletteString, 0) != STR2INT_SUCCESS || palette < 0 || palette >= MAX_PALETTES) {
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

void interpretColorizer(char* colorizerFilename, Cache* cache) {
	char* database;
	size_t databaseLength = readAllBytesFromFile(colorizerFilename, &database, true);
	if (databaseLength <= 0)
		return;

	databaseLength = removeCarriageReturns(database, databaseLength);
	databaseLength = removeComments(database, databaseLength);

	pushTokenContext();

	char* token = updateToken(database);
	while (token != NULL)
	{
		CHECK_COMMAND("p", handleColorizerPaletteCommand, cache);
		CHECK_COMMAND("s", handleColorizerSheetCommand, cache);

		if (strcmp(token, "end") == 0)
			break;

		printf("Invalid database token: ");
		printf("%s", token);
		printf("\n");
		break;
	}

	popTokenContext();

	free(database);
}

int handleHashCommand(char *hashString, Cache* cache) {
	
	char* token;
	
	PULL_TOKEN("Hash", token);
	if (strlen(token) < SIZE_OF_SHA_256_HASH) {
		printf("Warning: Matching hash \"");
		printf("%s", token);
		printf("\"  is too small.\n");
		return 0;
	}

	toUpperCase(token);
	if (strcmp(hashString, token) == 0) {
		printf("Matched hash!\n");
		foundRom = true;

		char colorizerFilename[32] = "";
		sprintf_s(colorizerFilename, sizeof(colorizerFilename), "colorizer/%.8s.txt", hashString);
		if (fileExists(colorizerFilename)) {
			interpretColorizer(colorizerFilename, cache);
		}
		
	}

	return 0;
}

int handlePatternCommand(Cache* cache)
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

int handlePaletteCommand(Cache* cache)
{
	char* token;
	PULL_TOKEN("Palette", token);

	if (paletteOverride)
		return 0;

	paletteDescription = token;
	return 0;
}

int handleSectionCommand(Cache* cache) {
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

	ripSection(&rom, cache, &args);
	return 0;
}

int handleCompressionCommand(Cache* cache)
{
	char* token;
	PULL_TOKEN("Compression", token);

	compressionType = token;
	return 0;
}

int handleBitplaneCommand(Cache* cache)
{
	char* token;
	PULL_TOKEN("Bitplane", token);

	if (bitplaneOverride)
		return 0;

	bitplaneType = token;
	return 0;
}

int handleCheckRedundantCommand(Cache* cache)
{
	char* token;
	PULL_TOKEN("CheckRedundant", token);

	if (checkRedundantOverride)
		return 0;

	checkRedundant = token;
	return 0;
}

int handleClearRedundantCommand(Cache* cache)
{
	cleanupPatternChains();
	initPatternChains();
	return 0;
}

void interpretDatabase(Cache* cache) {
	initPatternChains();

	uint8_t hash[SIZE_OF_SHA_256_HASH];
	calc_sha_256(hash, rom.data, rom.size);

	char hashString[SIZE_OF_SHA_256_HASH * 2 + 1] = "";
	char temp[4] = ""; // Temporary buffer for each hex value
	size_t dataSize = sizeof(hash) / sizeof(hash[0]);

	for (size_t i = 0; i < dataSize; i++) {
		sprintf_s(temp, sizeof(temp), "%02X", hash[i]);
#ifdef C99
		strcat(hashString, temp);
#else
		strcat_s(hashString, sizeof(hashString), temp);
#endif // C99
	}
	

	printf("ROM Hash: ");
	printf("%s", hashString);
	printf("\n");

	char* database;
	size_t databaseLength = readAllBytesFromFile(descriptorFilename, &database, true);
	foundRom = false;

	databaseLength = removeCarriageReturns(database, databaseLength);
	databaseLength = removeComments(database, databaseLength);

	char* token = updateToken(database);
	while (token != NULL) {
		if (!foundRom) {
			if (strcmp(token, "hash") == 0) {
				if (handleHashCommand(hashString, cache))
					break;
			}

			token = updateToken(NULL);
			continue;
		}

		CHECK_COMMAND("p", handlePatternCommand, cache);
		CHECK_COMMAND("i", handlePaletteCommand, cache);
		CHECK_COMMAND("c", handleCompressionCommand, cache);
		CHECK_COMMAND("b", handleBitplaneCommand, cache);
		CHECK_COMMAND("r", handleCheckRedundantCommand, cache);
		CHECK_COMMAND("k", handleClearRedundantCommand, cache);
		CHECK_COMMAND("s", handleSectionCommand, cache);

		if (strcmp(token, "end") == 0)
			break;


		printf("Invalid database token: ");
		printf("%s", token);
		printf("\n");
		break;
	}

	if (!foundRom)
		printf("Error: Could not match ROM with database.\n");
	else {
		if (cache->size > 0) {
			char customMsg[256];
			const char* filename = "separator_8x8_16.png";
			PNGInfo* imageInfo = getImageInfo(filename);
			unsigned char* image_data = stbi_load(filename, &imageInfo->width, &imageInfo->height, &imageInfo->channels, 0);
			if (image_data == NULL) {
				snprintf(customMsg, sizeof(customMsg), "Error loading from file: %s", filename);
				perror(customMsg);
				exit(EXIT_FAILURE);
			}
			processCache(cache, image_data, imageInfo);
		}
	}

	cleanupPatternChains();
}

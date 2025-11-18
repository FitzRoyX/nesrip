#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include "globals.h"
#include "interpreter.h"
#include "logger.h"
#include "ripper.h"
#include "rom.h"
#include "utils.h"
#include "sha_2/sha-256.h"

Rom rom;
Cache* cache;
char* programName;
char* outputFolder;
char* outputFilename = NULL;
char* bitplaneType = "2";
char* patternSize = "1";
char* patternDirection = "h";
char* compressionType = "raw";
char* databaseFilename = "nes_gfx_db.txt";
char* deduplicator = "true";
int patternOverride = false;
int bitplaneOverride = false;
int deduplicatorOverride = false;

static void quitProgram(int code) {
	if (outputFolder != NULL)
		free(outputFolder);
	freeRom(&rom);
	exit(code);
}

int main(int argc, char** argv) {
	printf("Compiler C Standard is %s\n", cStdInUse(__STDC_VERSION__));
	programName = getFilename(argv[0]);
	if (argc < 2) {
		printNoInput();
		return 0;
	}

	char* inputRomName = argv[1];
	rom = readRom(inputRomName);
	if (rom.size == -1) {
		printf("An error occured while opening input ROM file.\n");
		return 0;
	}
	char* inputFilename = getFilename(inputRomName);
	size_t outputFolderLength = getFilenameLengthWithoutExtension(inputFilename);
 	outputFolder = (char*)malloc(outputFolderLength + 9);
	if (outputFolder == NULL) {
		printf("Error: Couldn't allocate memory for output folder string.\n");
		quitProgram(0);
	}
#if defined(__clang__)
	memcpy_s(outputFolder, strlen(outputFolder), "output/", 7);
	memcpy_s(outputFolder + 7, strlen(outputFolder), inputFilename, outputFolderLength);
#elif defined(__GNUC__) || defined(__GNUG__) || defined(C99)
	memcpy(outputFolder, "output/", 7);
	memcpy(outputFolder + 7, inputFilename, outputFolderLength);
#elif defined(_MSC_VER)
	memcpy_s(outputFolder, strlen(outputFolder), "output/", 7);
	memcpy_s(outputFolder + 7, strlen(outputFolder), inputFilename, outputFolderLength);
#endif // defined(__clang__)
	outputFolder[outputFolderLength + 7] = '/';
	outputFolder[outputFolderLength + 8] = 0;
	printf("Ensuring output folder exists.\n");
	CreateDirectoryA("output", 0);
	CreateDirectoryA(outputFolder, 0);
	//TODO: Handle ROM hash detection and graphics ripping here
	int capacity = 10;
	cache = (Cache*)calloc(1, sizeof(Cache));
	if (cache == NULL) {
		printf("Error: Couldn't allocate memory for cache.\n");
		quitProgram(0);
	}
	initCache(cache, capacity);
	interpretDatabase(cache);
	quitProgram(0);
}

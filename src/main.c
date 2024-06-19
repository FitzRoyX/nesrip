#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include "args.h"
#include "globals.h"
#include "interpreter.h"
#include "logger.h"
#include "ripper.h"
#include "rom.h"
#include "utils.h"
#include "sha_2/sha-256.h"

Rom rom;
char* programName;
char* outputFolder;
char* outputFilename = NULL;
char* compressionType = "raw";
char* patternSize = "1";
char* patternDirection = "h";
char* paletteDescription = "bwot";
char* bitplaneType = "2";
char* descriptorFilename = "nes_gfxdb.txt";
char* checkRedundant = "true";
int patternOverride = false;
int paletteOverride = false;
int bitplaneOverride = false;
int checkRedundantOverride = false;

void quitProgram(int code)
{
	if (outputFolder != NULL)
		free(outputFolder);

	freeRom(&rom);
	exit(code);
}

int main(int argc, char** argv)
{
	programName = getFilename(argv[0]);

	if (argc < 2)
	{
		printNoInput();
		return 0;
	}

	if (checkHelpArg(argv[1]))
		return 0;

	char* inputRomName = argv[1];
	rom = readRom(inputRomName);

	if (rom.size < 0)
	{
		printf("An error occured while opening input ROM file.\n");
		return 0;
	}

	if (argc > 2)
	{
		if (handleAdditionnalArgs(0, argc - 2, argv + 2))
			quitProgram(0);
	}

	char* inputFilename = getFilename(inputRomName);
	int outputFolderLength = getFilenameLengthWithoutExtension(inputFilename);

	outputFolder = (char*)malloc(outputFolderLength + 9);

	if (outputFolder == NULL)
	{
		printf("Error: Couldn't allocate memory for output folder string.\n");
		quitProgram(0);
	}

	memcpy(outputFolder, "output/", 7);
	memcpy(outputFolder+7, inputFilename, outputFolderLength);
	outputFolder[outputFolderLength + 7] = '/';
	outputFolder[outputFolderLength + 8] = 0;

	printf("Ensuring output folder exists.\n");
	CreateDirectoryA("output", 0);
	CreateDirectoryA(outputFolder, 0);

	if (argc > 2)
	{
		if (handleAdditionnalArgs(1, argc - 2, argv + 2))
			quitProgram(0);
	}

	//TODO: Handle ROM hash detection and graphics ripping here
	interpretDatabase();

	quitProgram(0);
}
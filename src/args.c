#include <string.h>
#include "globals.h"
#include "ripper.h"
#include "logger.h"
#include "utils.h"

#define CHECK_ARG(arg, callback) \
if (strcmp(argv[0], arg) == 0) \
{ \
	if (callback(pass, &argc, &argv)) \
		return 1; \
	continue; \
}

#define CHECK_ARGC(arg, n) \
if (*argc < n) \
{ \
	printInvalidArgUsage(arg, "Not enough parameters."); \
	return 1; \
}

#define INC(n) \
*argc -= n; \
*argv += n

int checkHelpArg(char* arg)
{
	if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0)
	{
		printHelp();
		return 1;
	}

	return 0;
}

int handleDirectSectionRipArg(int pass, int* argc, char*** argv)
{
	CHECK_ARGC("-S", 3);

	if (pass != 1)
	{
		INC(3);
		return 0;
	}

	ExtractionArguments args =
	{
		(*argv)[1],
		(*argv)[2],
		patternSize,
		patternDirection,
		paletteDescription,
		compressionType,
		outputFolder,
		outputFilename
	};

	ripSection(&rom, cache, &args);
	return 1;
}

int handleOutputArg(int pass, int* argc, char*** argv)
{
	CHECK_ARGC("-o", 2);

	if (pass != 0)
	{
		INC(2);
		return 0;
	}

	outputFilename = (*argv)[1];

	INC(2);
	return 0;
}

int handlePatternArg(int pass, int* argc, char*** argv)
{
	CHECK_ARGC("-p", 3);

	if (pass != 0)
	{
		INC(3);
		return 0;
	}

	patternSize = (*argv)[1];
	patternDirection = (*argv)[2];
	patternOverride = true;

	INC(3);
	return 0;
}

int handleCompressionArg(int pass, int* argc, char*** argv)
{
	CHECK_ARGC("-c", 2);

	if (pass != 0)
	{
		INC(2);
		return 0;
	}

	compressionType = (*argv)[1];

	INC(2);
	return 0;
}

int handlePaletteDescriptionArg(int pass, int* argc, char*** argv)
{
	CHECK_ARGC("-i", 2);

	if (pass != 0)
	{
		INC(2);
		return 0;
	}

	if (strlen((*argv)[1]) != 4)
	{
		printInvalidArgUsage("-i", "Palette description is too long or too short. Use a 4 letter long combination of \"b\", \"o\", \"t\" or \"w\".");
		return 1;
	}

	for (int i = 0; i < 4; i++)
	{
		char c = (*argv)[1][i];
		if (c != 'b' && c != 'o' && c != 't' && c != 'w')
		{
			printInvalidArgUsage("-i", "Palette description contains invalid characters. Make sure to only use \"b\", \"o\", \"t\" or \"w\"");
			return 1;
		}
	}

	paletteDescription = (*argv)[1];
	paletteOverride = true;
	INC(2);
	return 0;
}

int handleDescriptorFilenameArg(int pass, int* argc, char*** argv)
{
	CHECK_ARGC("-d", 2);

	if (pass != 0)
	{
		INC(2);
		return 0;
	}

	descriptorFilename = (*argv)[1];
	INC(2);
	return 0;
}

int handleBitplaneArg(int pass, int* argc, char*** argv)
{
	CHECK_ARGC("-b", 2);

	if (pass != 0)
	{
		INC(2);
		return 0;
	}

	bitplaneType = (*argv)[1];
	bitplaneOverride = true;
	INC(2);
	return 0;
}

int handleCheckRedundantArg(int pass, int* argc, char*** argv)
{
	CHECK_ARGC("-r", 2);

	if (pass != 0)
	{
		INC(2);
		return 0;
	}

	checkRedundant = (*argv)[1];
	checkRedundantOverride = true;
	INC(2);
	return 0;
}

/*
*	Check and handle call arguments
*
*	Pass 0: Arguments that don't stop execution
*	Pass 1: Arguments that stop execution
*
*	Some arguments ignore the pass value (eg. --help)
*/
int handleAdditionnalArgs(int pass, int argc, char** argv)
{
	while (argc > 0)
	{
		if (checkHelpArg(argv[0]))
			return 1;

		CHECK_ARG("-S", handleDirectSectionRipArg);
		CHECK_ARG("-o", handleOutputArg);
		CHECK_ARG("-p", handlePatternArg);
		CHECK_ARG("-c", handleCompressionArg);
		CHECK_ARG("-i", handlePaletteDescriptionArg);
		CHECK_ARG("-d", handleDescriptorFilenameArg);
		CHECK_ARG("-b", handleBitplaneArg);
		CHECK_ARG("-r", handleCheckRedundantArg);

		printInvalidArg(argv[0]);
		return 1;
	}

	return 0;
}
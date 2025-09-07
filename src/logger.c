#include <stdio.h>
#include <string.h>
#include "logger.h"

void printProgamName()
{
	printf("%s", programName);
}

void printHelp()
{
	printf("Usage: ");
	printProgamName();
	printf(" file [arguments]\n");
	printf("Arguments:\n");
	printf(" -S {start address} {end address}                 Directly rip graphics from specified memory in ROM.\n");
	printf(" -o {filename}                                    Output filename (without file extension) when using -S.\n");
	printf(" -d {filename}                                    Graphics database filename.\n");
	printf(" -c {compression type, raw}                       Graphics decompression algorithm.\n");
	printf(" -p {pattern size, 1/2/4/8/16} {direction, h/v}   Set or override tile block size and direction.\n");
	printf(" -i {4 letter combination of b/o/t/w}             Set or override palette order for rendering.\n");
}

void printNoInput()
{
	printf("No input ROM file specified.\n");
	printf("Run ");
	printProgamName();
	printf(" --help to see usage information.\n");
}

void printInvalidArg(char* arg)
{
	printf("Unrecognised argument: ");
	printf("%s", arg);
	printf(".\n");
	printf("Run ");
	printProgamName();
	printf(" --help to see usage information.\n");
}

void printInvalidArgUsage(const char* arg, const char* error)
{
	printf("Invalid usage of argument ");
	printf("%s", arg);
	printf(": ");
	printf("%s", error);
	printf("\nRun ");
	printProgamName();
	printf(" --help to see usage information.\n");
}

void printInvalidDatabaseCommand(const char* arg, const char* error)
{
	printf("Error: Invalid graphics database command \"");
	printf("%s", arg);
	printf("\": ");
	printf("%s", error);
	printf("\nAborting rip process.\n");
}
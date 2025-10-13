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
	printf(" -s {} {}                Section start and end addresses.\n");
	printf(" -b {1/2}                Bitplane type.\n");
	printf(" -p {1/2/4/8/16} {h/v}   Pattern size and direction.\n");
	printf(" -c {}                   Ccompression type.\n");
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

void printInvalidDatabaseCommand(const char* arg, const char* error) {
	printf("Error: Invalid graphics database command \"");
	printf("%s", arg);
	printf("\": ");
	printf("%s", error);
	printf("\nAborting rip process.\n");
}

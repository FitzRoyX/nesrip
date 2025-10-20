#include <stdio.h>
#include <string.h>
#include "logger.h"

void printProgamName() {
	printf("%s", programName);
}

void printNoInput() {
	printf("No input ROM file specified.\n");
}

void printInvalidDatabaseCommand(const char* arg, const char* error) {
	printf("Error: Invalid graphics database command \"");
	printf("%s", arg);
	printf("\": ");
	printf("%s", error);
	printf("\nAborting rip process.\n");
}

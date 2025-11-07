#include "librom.h"
#include "..\utils.h"
#include <stdio.h>

extern struct librom;

size_t readAllBytesFromFile(char* filename, char** output, int zeroTerminate);
int fileExists(const char* filename);

static struct librom initialize_rom(char *filename, char endian) {
	if (fileExists(filename) == 0) {
		printf("Error: ROM file \"%s\" does not exist.\n", filename);
		struct librom empty = {0};
		return empty;
	}
	if (endian != 'L' && endian != 'B') {
		printf("Error: Invalid endianness specified. Use 'L' for little-endian or 'B' for big-endian. Value given \"%c\"\n", endian);
		struct librom empty = {0};
		return empty;
	}

	struct librom result = {0};
	result.size = readAllBytesFromFile(filename, &result.data, 0);
	result.filename = filename;
	result.endian = endian;
	result.cursor = 0;

	return result;
}
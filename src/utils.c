#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include "utils.h"

str2int_errno str2int(int* out, char* s, int base)
{
    char* end;
    if (s[0] == '\0' || isspace(s[0]))
        return STR2INT_INCONVERTIBLE;
    errno = 0;
    long l = strtol(s, &end, base);
    /* Both checks are needed because INT_MAX == LONG_MAX is possible. */
    if (l > INT_MAX || (errno == ERANGE && l == LONG_MAX))
        return STR2INT_OVERFLOW;
    if (l < INT_MIN || (errno == ERANGE && l == LONG_MIN))
        return STR2INT_UNDERFLOW;
    if (*end != '\0')
        return STR2INT_INCONVERTIBLE;
    *out = l;
    return STR2INT_SUCCESS;
}

int numberOfSetBits(int i)
{
    i = i - ((i >> 1) & 0x55555555);
    i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
    i = (i + (i >> 4)) & 0x0F0F0F0F;
    i *= 0x01010101;
    return  i >> 24;
}

int numDigits(int n)
{
    if (n == 0)
        return 1;
    return floor(log10(abs(n))) + 1;
}

int readAllBytesFromFile(char* filename, char** output, int zeroTerminate)
{
    FILE* fileptr = fopen(filename, "rb");
    long filelen;

    if (fileptr == NULL)
    {
        printf("Error: Can't find file \"");
        printf(filename);
        printf("\".\n");
        return -1;
    }

    fseek(fileptr, 0, SEEK_END);
    filelen = ftell(fileptr) + zeroTerminate;
    rewind(fileptr);

    char* result = (char*)malloc(filelen * sizeof(char));

    if (result == NULL)
    {
        printf("Error: Couldn't allocate memory for reading file data.\n");
        return -1;
    }

    fread(result, filelen, 1, fileptr);
    fclose(fileptr);

    for (int i = filelen - zeroTerminate; i < filelen; i++)
    {
        result[i] = 0;
    }

    *output = result;
    return filelen;
}

char* getFilename(char* path)
{
	char* searchPointer = path + strlen(path) - 1;

	while (searchPointer >= path)
    {
		if (searchPointer[0] == '\\' || searchPointer[0] == '/')
			return searchPointer + 1;

		searchPointer--;
	}

    return path;
}

int getFilenameLengthWithoutExtension(char* filename)
{
    char* searchPointer = filename + strlen(filename) - 1;

	while (searchPointer >= filename)
    {
		if (searchPointer[0] == '.')
			return searchPointer - filename;

		searchPointer--;
	}

    return strlen(filename);
}

// strtok_r from musl libc
char* stringTokenize(char* restrict s, const char* restrict sep, char** restrict p)
{
	if (!s && !(s = *p)) return NULL;
	s += strspn(s, sep);
	if (!*s) return *p = 0;
	*p = s + strcspn(s, sep);
	if (**p) *(*p)++ = 0;
	else *p = 0;
	return s;
}

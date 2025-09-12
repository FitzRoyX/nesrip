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

double numDigits(int n)
{
    if (n == 0)
        return 1;
    return floor(log10(abs(n))) + 1;
}

size_t readAllBytesFromFile(char* filename, char** output, int zeroTerminate)
{
    FILE* fileptr;
    errno_t err;
    long filelen;
    int error = false;

    #if C99
        fileptr = fopen(filename, "rb");
        if (fileptr == NULL) {
            error = true;
        }
    #else
        err = fopen_s(&fileptr, filename, "rb");
        if (err != 0) {
            error = true;
        }
    #endif

    if (error) {
        printf("Error: Can't find file \"");
        printf("%s", filename);
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

size_t getFilenameLengthWithoutExtension(char* filename)
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

const char* cStdInUse(long int stdc) {
    const char* version;
    switch (stdc) {
        case 199409L:
            version = "C90";
            break;
        case 199901L:
            version = "C99";
            break;
        case 201112L:
            version = "C11";
            break;
        case 201710L:
            version = "C17";
            break;
        case 202311L:
            version = "C23";
            break;
        default:
            version = "Unknown";
            break;
    }
    return version;
}

int is_null_terminated(const char* str, size_t max_length) {
    for (size_t i = 0; i <= max_length; i++) {
        if (str[i] == '\0') {
            return 1;  // Null-terminated
        }
    }
    return 0;  // Not null-terminated
}

void toUpperCase(char* str) {
    while (*str) {
        *str = toupper(*str);
        str++;
    }
}

int fileExists(const char* filename) {
    FILE *file;
#if defined(__GNUC__) || defined(__GNUG__) || defined(C99)
    file = fopen(filename, "r");
    if (file) {
        fclose(file);
        return 1;
    }
#else
    errno_t error = fopen_s(&file, filename, "r");
    if (error == 0) {
        fclose(file);
        return 1;
    }
#endif
    return 0;
}

void deleteCharacters(char* str, size_t pos, size_t n) {
    size_t len = strlen(str);

    // Ensure position and range are valid
    if (pos < 0 || pos >= len || n <= 0) {
        printf("Invalid position or range.\n");
        return;
    }

    // Shift characters to the left
    for (size_t i = pos; i < len - n; i++) {
        str[i] = str[i + n];
    }

    // Null-terminate the string
    str[len - n] = '\0';
}

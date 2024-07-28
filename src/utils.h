#ifndef UTILS_H
#define UTILS_H

typedef enum
{
    STR2INT_SUCCESS,
    STR2INT_OVERFLOW,
    STR2INT_UNDERFLOW,
    STR2INT_INCONVERTIBLE
} str2int_errno;

#define true 1
#define false 0

str2int_errno str2int(int* out, char* s, int base);
int numberOfSetBits(int i);
int numDigits(int n);
int readAllBytesFromFile(char* filename, char** output, int zeroTerminate);
char* getFilename(char* path);
int getFilenameLengthWithoutExtension(char* filename);
char* stringTokenize(char* restrict s, const char* restrict sep, char** restrict p);

#endif
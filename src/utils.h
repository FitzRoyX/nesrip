
#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#include "globals.h"

typedef enum {
    STR2INT_SUCCESS,
    STR2INT_OVERFLOW,
    STR2INT_UNDERFLOW,
    STR2INT_INCONVERTIBLE
} str2int_errno;

#define true 1
#define false 0

#ifndef __STDC_VERSION__ 
    #define __STDC_VERSION__ 0
#endif // !__STDC_VERSION__

#if __STDC_VERSION__ == 199901L
    #define C99  1
#else 
    #define C99  0
#endif

str2int_errno str2int(int* out, char* s, int base);
int numberOfSetBits(int i);
double numDigits(int n);
size_t readAllBytesFromFile(char* filename, char** output, int zeroTerminate);
char* getFilename(char* path);
size_t getFilenameLengthWithoutExtension(char* filename);
char* stringTokenize(char* restrict s, const char* restrict sep, char** restrict p);
const char* cStdInUse(long int);
int is_null_terminated(const char* str, size_t max_length);
void toUpperCase(char* str);
int fileExists(const char* filename);
void deleteCharacters(char* str, size_t pos, size_t n);
void initCache(Cache* cache, int initialCapacity);
void addToCache(Cache* cache, char* value, int x, int y, int comp);
PNGInfo* getImageInfo(const char* filename);
void processCache(Cache* cache, char* separator, PNGInfo* info);
void generateTile(uint8_t* image, uint8_t color1[4], uint8_t color2[4]);
void generate_image(uint8_t* image, int repeat_count);
void generate_TransparentImage(uint8_t* image, int repeat_count);
#endif
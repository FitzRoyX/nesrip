#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb/stb_image_write.h"   
#include "utils.h"
#include "globals.h"

static uint8_t* g_outputImage = NULL;
static int g_outputWidth = 0;
static int g_outputHeight = 0;

str2int_errno str2int(int* out, char* s, int base) {
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

int numberOfSetBits(int i) {
	i = i - ((i >> 1) & 0x55555555);
	i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
	i = (i + (i >> 4)) & 0x0F0F0F0F;
	i *= 0x01010101;
	return  i >> 24;
}

double numDigits(int n) {
	if (n == 0)
		return 1;
	return floor(log10(abs(n))) + 1;
}

size_t readAllBytesFromFile(char* filename, char** output, int zeroTerminate) {
	FILE* fileptr;
	errno_t err; //annex k type alias, not standard c
	long filelen;
	int error = 0;
	#if C99
		fileptr = fopen(filename, "rb");
		if (fileptr == NULL) {
			error = 1;
		}
    #else
        if (fopen_s(&fileptr, filename, "rb") != 0) {
            error = 1;
        }
    #endif
	if (error) {
		printf("Error: Can't find file \"%s\".\n", filename);
		return -1;
	}
	fseek(fileptr, 0, SEEK_END);
	filelen = ftell(fileptr) + zeroTerminate;
	rewind(fileptr);
	int saved_errno = errno; //annex k type alias, not standard c
	char* result = malloc(filelen);
	if (result == NULL) {
		printf("Error: Couldn't allocate memory for reading file data.\n");
		fclose(fileptr);
		return -1;
	}
	errno = saved_errno; //annex k type alias, not standard c
	fread(result, filelen - zeroTerminate, 1, fileptr);
	fclose(fileptr);
	if (zeroTerminate) {
		result[filelen - 1] = 0;
	}
	*output = result;
	return filelen;
}

char* getFilename(char* path) {
	char* searchPointer = path + strlen(path) - 1;
	while (searchPointer >= path) {
		if (searchPointer[0] == '\\' || searchPointer[0] == '/')
			return searchPointer + 1;
		searchPointer--;
	}
	return path;
}

size_t getFilenameLengthWithoutExtension(char* filename) {
	char* searchPointer = filename + strlen(filename) - 1;
	while (searchPointer >= filename) {
		if (searchPointer[0] == '.')
			return searchPointer - filename;
		searchPointer--;
	}
	return strlen(filename);
}

// strtok_r from musl libc
char* stringTokenize(char* restrict s, const char* restrict sep, char** restrict p) {
    if (!s && !(s = *p)) {
        return NULL;
    }
    s += strspn(s, sep);
    if (!*s) {
        *p = 0;
        return NULL;
    }
    *p = s + strcspn(s, sep);
    if (**p) {
        *(*p)++ = 0;
    } else {
        *p = 0;
    }
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

int isNullTerminated(const char* str, size_t max_length) {
	for (size_t i = 0; i <= max_length; i++) {
		if (str[i] == '\0') {
			return 1;
		}
	}
	return 0;
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
	errno_t error = fopen_s(&file, filename, "r"); //non-standard c
	if (error == 0) {
		fclose(file);
		return 1;
	}
#endif
	return 0;
}

void deleteCharacters(char* str, size_t pos, size_t n) {
	size_t len = strlen(str);
	if (pos >= len || n == 0) {
		return;
	}
	for (size_t i = pos; i < len - n; i++) {
		str[i] = str[i + n];
	}
	str[len - n] = '\0';
}

#define TILE_SIZE 8

static void generateTransparentTile(uint8_t* image) {
	// Create an 8x8 transparent tile (RGBA format)
	for (int y = 0; y < TILE_SIZE; y++) {
		for (int x = 0; x < TILE_SIZE; x++) {
			int index = (y * TILE_SIZE + x) * 4;
			image[index + 0] = 0;   // Red
			image[index + 1] = 0;   // Green
			image[index + 2] = 0;   // Blue
			image[index + 3] = 0;   // Alpha (fully transparent)
		}
	}
}

void generateTransparentImage(uint8_t* image, int repeat_count) {
	uint8_t tile[TILE_SIZE * TILE_SIZE * 4]; // RGBA format
	generateTransparentTile(tile);
	int image_width = TILE_SIZE * repeat_count;
	for (int y = 0; y < TILE_SIZE; y++) {
		for (int repeat = 0; repeat < repeat_count; repeat++) {
			for (int x = 0; x < TILE_SIZE; x++) {
				int src_idx = (y * TILE_SIZE + x) * 4;
				int dest_idx = (y * image_width + repeat * TILE_SIZE + x) * 4;
				image[dest_idx + 0] = tile[src_idx + 0];
				image[dest_idx + 1] = tile[src_idx + 1];
				image[dest_idx + 2] = tile[src_idx + 2];
				image[dest_idx + 3] = tile[src_idx + 3];
			}
		}
	}
}

void generateSeparatorTile(uint8_t* image, uint8_t color1[4], uint8_t color2[4]) {
	for (int y = 0; y < TILE_SIZE; y++) {
		for (int x = 0; x < TILE_SIZE; x++) {
			int index = 4 * (y * TILE_SIZE + x);
			if (x == y || x == TILE_SIZE - y - 1) {
				// Set color for the "X" pattern
				image[index + 0] = color1[0]; // Red
				image[index + 1] = color1[1]; // Green
				image[index + 2] = color1[2]; // Blue
				image[index + 3] = color1[3]; // Alpha
			}
			else {
				// Set background color
				image[index + 0] = color2[0];
				image[index + 1] = color2[1];
				image[index + 2] = color2[2];
				image[index + 3] = color2[3];
			}
		}
	}
}

void generateSeparator(uint8_t* image, int repeat_count) {
	uint8_t tile[TILE_SIZE * TILE_SIZE * 4]; // RGBA format
	// Define two colors: Magenta for "X" and Yellow for background
	uint8_t color1[4] = { 255, 0, 255, 255 };   // Magenta
	uint8_t color2[4] = { 255, 255, 0, 255 }; // Yellow
	// Generate the tile
	generateSeparatorTile(tile, color1, color2);
	int image_width = TILE_SIZE * repeat_count;
	for (int y = 0; y < TILE_SIZE; y++) {
		for (int repeat = 0; repeat < repeat_count; repeat++) {
			for (int x = 0; x < TILE_SIZE; x++) {
				int src_idx = (y * TILE_SIZE + x) * 4;
				int dest_idx = (y * image_width + repeat * TILE_SIZE + x) * 4;
				image[dest_idx + 0] = tile[src_idx + 0];
				image[dest_idx + 1] = tile[src_idx + 1];
				image[dest_idx + 2] = tile[src_idx + 2];
				image[dest_idx + 3] = tile[src_idx + 3];
			}
		}
	}
}

void appendSectionToOutput(const char* sheet, int width, int height) {
    if (sheet == NULL || width <= 0 || height <= 0) {
        return;
    }

    int bytesPerRow = width * 4;
    int sectionBytes = bytesPerRow * height;

    // First section: just copy
    if (g_outputImage == NULL) {
        g_outputWidth = width;
        g_outputHeight = height;
        g_outputImage = (uint8_t*)malloc((size_t)sectionBytes);
        if (!g_outputImage) {
            perror("failed to allocate memory for output image");
            exit(EXIT_FAILURE);
        }
        memcpy(g_outputImage, sheet, (size_t)sectionBytes);
        return;
    }

    // Subsequent sections: append separator + new section
    if (width != g_outputWidth) {
        fprintf(stderr,
            "Error: section width (%d) does not match output width (%d)\n",
            width, g_outputWidth);
        exit(EXIT_FAILURE);
    }

    int separatorHeight = TILE_SIZE; // 8
    int repeat_count = width / TILE_SIZE;
    int separatorBytes = bytesPerRow * separatorHeight;

    size_t oldBytes = (size_t)bytesPerRow * g_outputHeight;
    size_t newBytes = oldBytes + (size_t)separatorBytes + (size_t)sectionBytes;

    uint8_t* newImage = (uint8_t*)malloc(newBytes);
    if (!newImage) {
        perror("failed to allocate memory for combined output");
        exit(EXIT_FAILURE);
    }

    // Copy old data
    memcpy(newImage, g_outputImage, oldBytes);

    // Build separator band
    uint8_t* separator = (uint8_t*)malloc((size_t)separatorBytes);
    if (!separator) {
        free(newImage);
        perror("failed to allocate memory for separator");
        exit(EXIT_FAILURE);
    }
    generateSeparator(separator, repeat_count);
    memcpy(newImage + oldBytes, separator, (size_t)separatorBytes);
    free(separator);

    // Append new section data
    memcpy(newImage + oldBytes + separatorBytes, sheet, (size_t)sectionBytes);

    free(g_outputImage);
    g_outputImage = newImage;
    g_outputHeight += separatorHeight + height;
}

void finalizeOutputImage(void) {
    if (g_outputImage == NULL) {
        // No sections ripped => nothing to write
        return;
    }

    char filename[260];
    snprintf(filename, sizeof(filename), "%s0.png", outputFolder);

    printf("  Writing combined sheets to \"");
    printf("%s", filename);
    printf("\".\n");

    if (!stbi_write_png(
            filename,
            g_outputWidth,
            g_outputHeight,
            4,
            g_outputImage,
            g_outputWidth * 4)) {
        perror("Failed to write combined image");
        free(g_outputImage);
        g_outputImage = NULL;
        g_outputWidth = 0;
        g_outputHeight = 0;
        exit(EXIT_FAILURE);
    }

    free(g_outputImage);
    g_outputImage = NULL;
    g_outputWidth = 0;
    g_outputHeight = 0;
}


#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"   

#include "utils.h"
#include "globals.h"

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

void initCache(Cache* cache, int initialCapacity) {
    cache->size = 0;
    cache->capacity = initialCapacity;
	cache->images = (PNGImage**)calloc(initialCapacity, sizeof(PNGImage));
}

// Function to add data to the cache
void addToCache(Cache* cache, char* value, int width, int height, int comp) {
    if (cache->size == cache->capacity) {
        // Double the capacity if the cache is full
        cache->capacity *= 2;
        PNGImage** tmp = (PNGImage**)realloc(cache->images, cache->capacity * sizeof(PNGImage));
        if (tmp == NULL) {
			free(cache);
            perror("Failed to reallocate memory to increase size of the Cache");
            exit(EXIT_FAILURE);
        }
		cache->images = tmp;
    }

    int idx = cache->size++;

    // First copy the image before it is freed in the ripper logic
	size_t imageSize = (size_t)width * height * comp * sizeof(char);
    if (imageSize <= 4000) {
        imageSize = 4000; // minimum size to avoid small allocations
    }
    char* image_data = (char*)calloc(1, imageSize);
    if (image_data == NULL) {
        free(cache);
        perror("Failed to allocate memory for image data in Cache");
        exit(EXIT_FAILURE);
    }

    memmove(image_data, value, imageSize);

    PNGImage* image = (PNGImage*)calloc(1, sizeof(PNGImage));
    if (image == NULL) {
		free(cache);
        perror("Failed to allocate memory for PNGImage");
        exit(EXIT_FAILURE);
	}
	
    image->imageInfo.width = width;
    image->imageInfo.height = height;
    image->imageInfo.channels = comp;
    image->data = image_data;
    image->size = (int)imageSize;
    cache->images[idx] = image;
}

void processCache(Cache* cache, char* separator, PNGInfo* info) {
    int maxSize = 0;
    for (size_t i = 0; i < cache->size; i++) {
		PNGImage* image = cache->images[i];
        maxSize = maxSize + image->size;
    }

    int sep_size = info->height * info->width * info->channels * sizeof(char);
    int totalSepSize = sep_size * (cache->size -1);

    char* combinedimage = (char*)calloc((size_t)maxSize + totalSepSize, sizeof(char));
    if (!combinedimage) {
        perror("failed to allocate memory for combined image");
        free(separator);
        free(cache);
        exit(EXIT_FAILURE);
    }

	int offset = 0, height = 0, width = 0, combined_height = 0;
    char filename[256];
    for (int i = 0; i < cache->size; i++) {
        PNGImage* image = cache->images[i];
		combined_height += image->imageInfo.height;
		if (image->imageInfo.width > width)
		    width = image->imageInfo.width;

        if (i == 2) {
            char* testimage = (char*)calloc((size_t)image->size, sizeof(char));
            memcpy(testimage, image->data, image->size);
            snprintf(filename, sizeof(filename), "output/before_combined_%d.png", i + 1);
            stbi_write_png(filename, width, image->imageInfo.height, 4, testimage, 128 * 4);
        }

        // Copy the current image
        memcpy(combinedimage + offset, image->data, image->size);
		offset += image->size;
        snprintf(filename, sizeof(filename), "output/combined_%d.png", i + 1);
        stbi_write_png(filename, width, combined_height, 4, combinedimage, 128 * 4);

        // Insert separator only between images
        if (i < (cache->size - 1)) {
            memcpy(combinedimage + offset, separator, sep_size);
            offset += sep_size;
			combined_height += info->height;
        }
        if(i < 2) { continue; }
        break;
    }

    strcpy(filename, "output/0.png");
    printf("  Writing combined sheets to \"");
    printf("%s", filename);
    printf("\".\n");

    if (combinedimage == NULL) {
        perror("No images in cache to combine");
        exit(EXIT_FAILURE);
	}
    if (!stbi_write_png(filename, width, combined_height, 4, combinedimage, 128 * 4)) {
        perror("Failed to write combined image\n");
        exit(EXIT_FAILURE);
    }
}

PNGInfo* getImageInfo(const char* filename) {
    PNGInfo* info = (PNGInfo*)calloc(1, sizeof(PNGInfo));
    if (info == NULL) {
        perror("Memory allocation failed for PNGInfo");
        exit(EXIT_FAILURE);
    }

    if (stbi_info(filename, &info->width, &info->height, &info->channels)) {
        return info;
    } else {
        if (info != NULL) {
            free(info);
        }
        perror("Failed to retrieve image info. Ensure the file exists and is a valid image.\n");
        exit(EXIT_FAILURE);
    }
}

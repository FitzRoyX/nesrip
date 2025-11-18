#include "decompressor.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t read_byte(const uint8_t* data, size_t* cursor) {
	return data[(*cursor)++];
}

Result* decompressRleKonami(const uint8_t* rom_data, size_t offset, size_t section_size) {
	size_t cursor = offset;
	const uint8_t* data = rom_data + offset;
	Result* result = malloc(sizeof(Result));
	if (!result) {
		printf("Error: Failed to allocate output buffer.\n");
		return NULL;
	}
	result->output = malloc(section_size * 2 * sizeof(uint8_t));
	if (!result->output) {
		printf("Error: Failed to allocate memory for decompressed output.\n");
		free(result);
		return NULL;
	}
	result->size = 0;
	while (1) {
		uint8_t byte = read_byte(data, &cursor);
		if (byte == 0xFF) {
			break;
		}
		if (byte <= 0x80) {
			uint8_t _readed = read_byte(data, &cursor);
			for (int i = 0; i < byte; i++) {
				result->output[result->size] = _readed;
				result->size++;
			}
		} else {
			for (int i = 0; i < (byte - 0x80); i++) {
				result->output[result->size] = read_byte(data, &cursor);
				result->size++;
			}
		}
	}
	return result;
}
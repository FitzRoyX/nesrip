#include <cstddef>
#ifndef LIB_ROM_H
#define LIB_ROM_H

struct librom {
	char* filename;
	char* data;
	size_t size;
	char endian; // 'L' for little-endian, 'B' for big-endian
	int cursor;
};

// Global instance of librom
extern struct librom;

#endif

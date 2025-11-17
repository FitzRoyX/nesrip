#ifndef ROM_H
#define ROM_H

typedef struct {
	char* originalData;
	char* data;
	size_t size;
} Rom;

Rom readRom(char* romName);
void freeRom(Rom* rom);

#endif

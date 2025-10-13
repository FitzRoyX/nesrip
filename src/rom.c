#include <stdio.h>
#include <malloc.h>
#include "rom.h"
#include "utils.h"

Rom readRom(char* romName) {
    Rom result = {(char*)NULL, (char*)NULL};

    result.size = readAllBytesFromFile(romName, &result.originalData, false);
    result.data = result.originalData;

    if (result.size < 0)
        return result;
    
    if (result.data[0] == 'N' && result.data[1] == 'E' && result.data[2] == 'S' && result.data[3] == 0x1A) {
        result.data += 0x10;
        result.size -= 0x10;
    }
    
    return result;
}

void freeRom(Rom* rom) {
    if (rom->originalData == NULL)
        return;
    free(rom->originalData);
}

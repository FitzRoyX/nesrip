#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "rom.h"
#include "utils.h"

Rom readRom(char* romName) {
    Rom result = {NULL, NULL, 0};
    size_t size = readAllBytesFromFile(romName, &result.originalData, false);
    if (size == -1) {
        result.size = -1;
        return result;
    }
    result.size = size;
    result.data = result.originalData;
    if (strstr(romName, ".nes") || strstr(romName, ".unh")) {
        if (result.data[0] == 'N' && result.data[1] == 'E' && result.data[2] == 'S' && result.data[3] == 0x1A) {
            result.data += 0x10;
            result.size -= 0x10;
        }
        databaseFilename = "gfxdb_nes.txt";
    } else if (strstr(romName, ".sfc") || strstr(romName, ".smc")) {
        databaseFilename = "gfxdb_snes.txt";
    } else if (strstr(romName, ".dat")) {
        databaseFilename = "gfxdb_dos.txt";
    } else {
        printf("Unsupported ROM extension for: %s\n", romName);
        result.size = -1;
        return result;
    }
    printf("Using database: %s\n", databaseFilename);
    return result;
}

void freeRom(Rom* rom) {
    if (rom->originalData == NULL)
        return;
    free(rom->originalData);
}
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "sha_2/sha-256.h"

#define hash_size SIZE_OF_SHA_256_HASH * 2 + 1

void interpretDatabase(void);

#endif

#include "./malloc.h"

// memory buffer for sequentially storing macros
char _mallocBuffer[MALLOC_BUFFERSIZE];
uint16_t _bufferIndex;

void init_agon_malloc(void) {
    _bufferIndex = 0;
}

char *agon_malloc(uint16_t length) {
    char *ptr;
    if((_bufferIndex + length) > MALLOC_BUFFERSIZE - 1) return 0;

    ptr = &_mallocBuffer[_bufferIndex];
    _bufferIndex += length;
    return ptr;
}

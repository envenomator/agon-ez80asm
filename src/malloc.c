#include "./malloc.h"

// memory buffer for sequentially storing macros
char _mallocBuffer[MALLOC_BUFFERSIZE];
uint24_t _bufferIndex;

void init_agon_malloc(void) {
    _bufferIndex = 0;
}

char *agon_malloc(uint16_t length) {
    if((_bufferIndex + length) < MALLOC_BUFFERSIZE) {
        char* ptr = &_mallocBuffer[_bufferIndex];
        _bufferIndex += length;
        return ptr;
    }
    return 0;
}

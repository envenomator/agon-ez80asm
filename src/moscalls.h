#ifndef MOSCALLS_H
#define MOSCALLS_H

#include <stdio.h>

extern uint8_t removefile(const char *filename);
extern unsigned int getfilesize(uint8_t fh);

// either CEDEV assembly, or defined in moscalls.c
extern unsigned char fast_strcasecmp(const char* s1, const char* s2);

#endif
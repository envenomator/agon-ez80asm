#include <stdlib.h>
#include <string.h>
#include "config.h"

#ifndef CEDEV
// use regular strcasecmp from std lib
unsigned char fast_strcasecmp(const char* s1, const char* s2) {
	return strcasecmp(s1, s2);
}

unsigned char fast_strncasecmp(const char* s1, const char* s2, uint8_t n) {
	return strncasecmp(s1, s2, n);
}
#endif
#ifndef UTILS_H
#define UTILS_H

#include "config.h"

typedef struct {
    char    start[TOKEN_MAX + 1];
    uint8_t length;
    char    *next;
    char    terminator;
} tokentype;

void text_RED(void);
void text_YELLOW(void);
void text_NORMAL(void);

void remove_ext (char* myStr, char extSep, char pathSep);
void trimRight(char *str);
void error(char* msg);
bool isEmpty(const char *str);
bool notEmpty(const char *str);
bool split_suffix(char *mnemonic, char *suffix, char *buffer);
uint8_t getLineToken(tokentype *token, char *src, char terminator);
uint8_t getOperatorToken(tokentype *token, char *src);

#ifdef AGON
int strcasecmp(char *s1, char *s2);
#endif
#endif // UTILS_H

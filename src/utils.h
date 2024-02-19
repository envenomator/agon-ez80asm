#ifndef UTILS_H
#define UTILS_H

#include "config.h"

// Token type that points to the (changed) underlying data string
typedef struct {
    char    *start;
    char    *next;
    char    terminator;
} streamtoken_t;

void text_RED(void);
void text_YELLOW(void);
void text_NORMAL(void);

void remove_ext (char* myStr, char extSep, char pathSep);
void trimRight(char *str);
void error(char* msg);
bool isEmpty(const char *str);
bool notEmpty(const char *str);
uint8_t getOperatorToken(streamtoken_t *token, char *src);          // terminates on operator symbols like +-/%<<>>
void    getLabelToken(streamtoken_t *token, char *src);                // terminates on ':' character
uint8_t getMnemonicToken(streamtoken_t *token, char *src);          // terminated on spaces
uint8_t getOperandToken(streamtoken_t *token, char *src);           // terminates on , or ;  transparent literals
uint8_t getDefineValueToken(streamtoken_t *token, char *src);       // terminates on all operator symbols, ',' and ';' transparent literals and strings
void parse_command(char *src);

#ifdef AGON
int strcasecmp(char *s1, char *s2);
#endif
#endif // UTILS_H

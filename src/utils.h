#ifndef UTILS_H
#define UTILS_H

#include "config.h"
#include <ctype.h>
// DEFAULT COLOR INDEXES
enum {
    DARK_BLACK = 0,
    DARK_RED,
    DARK_GREEN,
    DARK_YELLOW,
    DARK_BLUE,
    DARK_MAGENTA,
    DARK_CYAN,
    DARK_WHITE,
    BRIGHT_BLACK,
    BRIGHT_RED,
    BRIGHT_GREEN,
    BRIGHT_YELLOW,
    BRIGHT_BLUE,
    BRIGHT_MAGENTA,
    BRIGHT_CYAN,
    BRIGHT_WHITE
};

// ERROR LEVELS
enum {
    LEVEL_ERROR,
    LEVEL_WARNING
};

// Token type that points to the (changed) underlying data string
typedef struct {
    char    *start;
    char    *next;
    char    terminator;
} streamtoken_t;

void remove_ext (char* myStr, char extSep, char pathSep);
void trimRight(char *str);

void error(char* msg);
void warning(char* msg);
int32_t getValue(char *str, bool req_firstpass);
uint8_t getEscapedChar(char c);
uint8_t getLiteralValue(char *string);
uint8_t getOperatorToken(streamtoken_t *token, char *src);          // terminates on operator symbols like +-/%<<>>
void    getLabelToken(streamtoken_t *token, char *src);                // terminates on ':' character
uint8_t getMnemonicToken(streamtoken_t *token, char *src);          // terminated on spaces
uint8_t getOperandToken(streamtoken_t *token, char *src);           // terminates on , or ;  transparent literals
uint8_t getDefineValueToken(streamtoken_t *token, char *src);       // terminates on all operator symbols, ',' and ';' transparent literals and strings
uint16_t getnextline(char *dst);
void     resetnextline(char *src);
char *  allocateString(char *name);
void *  allocateMemory(size_t size);

uint8_t strcompound(char *dest, const char *src1, const char *src2);
void validateRange8bit(int32_t value);
void validateRange16bit(int32_t value);
void validateRange24bit(int32_t value);
#endif // UTILS_H


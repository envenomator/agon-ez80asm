#ifndef UTILS_H
#define UTILS_H

#include "config.h"

// DEFAULT COLOR INDEXES
enum {
    BLACK = 0,
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

// Token type that points to the (changed) underlying data string
typedef struct {
    char    *start;
    char    *next;
    char    terminator;
} streamtoken_t;

void remove_ext (char* myStr, char extSep, char pathSep);
void trimRight(char *str);

void error(char* msg);

uint8_t getOperatorToken(streamtoken_t *token, char *src);          // terminates on operator symbols like +-/%<<>>
void    getLabelToken(streamtoken_t *token, char *src);                // terminates on ':' character
uint8_t getMnemonicToken(streamtoken_t *token, char *src);          // terminated on spaces
uint8_t getOperandToken(streamtoken_t *token, char *src);           // terminates on , or ;  transparent literals
uint8_t getDefineValueToken(streamtoken_t *token, char *src);       // terminates on all operator symbols, ',' and ';' transparent literals and strings
void    parse_command(char *src);

int i_strcasecmp(const char *s1, const char *s2);

#endif // UTILS_H

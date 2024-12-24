#ifndef UTILS_H
#define UTILS_H

#include <ctype.h>
#include "config.h"
#include "typedefs.h"

void remove_ext (char* myStr, char extSep, char pathSep);
void trimRight(char *str);

void error(const char *msg, const char *contextformat, ...);
void warning(const char *msg, const char *contextformat, ...);
void colorPrintf(int color, const char *msg, ...);

int32_t getExpressionValue(char *str, requiredResult_t requiredPass);
uint8_t getEscapedChar(char c);
uint8_t getLiteralValue(const char *string);
void    getLabelToken(streamtoken_t *token, char *src);                // terminates on ':' character
uint8_t getMnemonicToken(streamtoken_t *token, char *src);          // terminated on spaces
uint8_t getOperandToken(streamtoken_t *token, char *src);           // terminates on , or ;  transparent literals
uint8_t getDefineValueToken(streamtoken_t *token, char *src);       // terminates on all operator symbols, ',' and ';' transparent literals and strings

uint16_t getnextline(char **ptr, char *dst);
char *  allocateString(const char *name);
void *  allocateMemory(size_t size);

uint8_t strcompound(char *dest, const char *src1, const char *src2);
void validateRange8bit(int32_t value, const char *name);
void validateRange16bit(int32_t value, const char *name);
void validateRange24bit(int32_t value, const char *name);
#endif // UTILS_H


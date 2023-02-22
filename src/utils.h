#ifndef UTILS_H
#define UTILS_H

#define TOKEN_MAX   256

typedef struct {
    char    start[TOKEN_MAX];
    uint8_t length;
    char    *next;
    char    terminator;
} tokentype;

void remove_ext (char* myStr, char extSep, char pathSep);
void trimRight(char *str);
void error(char* msg);
void debugmsg(char *msg);
bool isEmpty(const char *str);
bool notEmpty(const char *str);
void trimEdges(char *str);
void split_suffix(char *mnemonic, char *suffix, char *buffer);
uint8_t getLineToken(tokentype *token, char *src, char terminator);
uint8_t getOperatorToken(tokentype *token, char *src);

uint8_t get_token(tokentype *token, char *src);

#endif // UTILS_H

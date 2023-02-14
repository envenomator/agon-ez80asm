#ifndef UTILS_H
#define UTILS_H

#define TOKEN_MAX   256

typedef struct {
    char start[TOKEN_MAX];
    char *end;
    char *next;
    char terminator;
} tokentype;

void remove_ext (char* myStr, char extSep, char pathSep);
void strstripleft(const char *source_str, char *dest_str);
void error(char* msg);
void debugmsg(char *msg);
bool isempty(const char *str);
bool notempty(const char *str);
void trimEdges(char *str);
char *parse_token(char *token, char  *src, char delimiter, bool required);
uint8_t get_token(tokentype *token, char *src);
void split_suffix(char *mnemonic, char *suffix, char *buffer);
#endif // UTILS_H

#ifndef UTILS_H
#define UTILS_H

#define TOKEN_MAX   128

typedef struct {
    char    start[TOKEN_MAX];
    uint8_t length;
    char    *next;
    char    terminator;
} tokentype;

void remove_ext (char* myStr, char extSep, char pathSep);
void trimRight(char *str);
void error(char* msg);
bool isEmpty(const char *str);
bool notEmpty(const char *str);
void split_suffix(char *mnemonic, char *suffix, char *buffer);
uint8_t getLineToken(tokentype *token, char *src, char terminator);
uint8_t getOperatorToken(tokentype *token, char *src);

bool openFile(uint8_t *file, char *name, uint8_t mode);
bool reOpenFile(uint8_t number, uint8_t mode);
void prepare_filenames(char *input_filename);
void closeAllFiles();
bool openfiles(void);

char *agon_fgets(char *s, int size, uint8_t fileid);
int agon_fputs(char *s, uint8_t fileid);
size_t agon_fwrite(void *ptr, size_t size, size_t nmemb, uint8_t fileid);
size_t agon_fread(void *ptr, size_t size, size_t nmemb, uint8_t fileid);

int8_t strcasecmp(char *s1, char *s2);

#endif // UTILS_H

#ifndef MACRO_H
#define MACRO_H

typedef struct {
    char*   name;
    uint8_t argcount;
    char**  arguments;
} macro;

void    initMacros(void);
bool    defineMacro(char *name, uint8_t argcount, char **arguments);
macro * findMacro(char *name);
void    getMacroFilename(char *filename, char *name);

#endif // MACRO_H

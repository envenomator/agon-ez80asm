#include "macro.h"
#include "globals.h"
#include "hash.h"
#include "config.h"

// Total allocated memory for macros
uint24_t macromemsize;

// tables
macro_t *macroTable[MACRO_TABLESIZE]; // indexed table
uint8_t macroTableCounter;

void initMacros(void) {
    macromemsize = 0;
    macroTableCounter = 0;
    memset(macroTable, 0, sizeof(macroTable));
}

macro_t* findMacro(char *name){
    int index;
    macro_t *try;

    index = hash(name) % MACRO_TABLESIZE;
    try = macroTable[index];

    while(true)
    {
        if(try == NULL) return NULL;
        if(strcmp(try->name, name) == 0) return try;
        try = try->next;
    }
}

void setMacroBody(macro_t *macro, const char *body) {
    macro->body = (char*)malloc(strlen(body)+1);
    if(macro->body == NULL) {
        error(message[ERROR_MACROMEMORYALLOCATION]);
        return;
    }
    strcpy(macro->body, body);
}

macro_t *defineMacro(char *name, uint8_t argcount, char *arguments) {
    int len, index, i;
    char *ptr,*subs;
    macro_t *tmp,*try;

    // allocate space in buffer for macro_t struct
    tmp = (macro_t *)malloc(sizeof(macro_t));
    if(tmp == 0) return NULL;

    // allocate space in buffer for string and store it to buffer
    len = strlen(name);
    if(len > MAXNAMELENGTH) {
        error(message[ERROR_MACRONAMELENGTH]);
        return NULL;
    }
    tmp->name = (char*)malloc(len+1);
    macromemsize += len+1;
    if(tmp->name == 0) return NULL;

    strcpy(tmp->name, name);
    tmp->next = NULL;

    tmp->argcount = argcount;
    tmp->substitutions = NULL;
    if(argcount == 0) tmp->arguments = NULL;
    else {
        tmp->arguments = (char **)malloc(argcount * sizeof(char *)); // allocate array of char*
        macromemsize += argcount * sizeof(char *);
        tmp->substitutions = (char **)malloc(argcount * sizeof(char *));
        macromemsize += argcount * sizeof(char *);
        if((tmp->arguments == NULL) || (tmp->substitutions == NULL)) return NULL;
        for(i = 0; i < argcount; i++) {
            len = strlen(arguments + i*(MACROARGLENGTH+1));
            if(len > MACROARGLENGTH) {
                error(message[ERROR_MACROARGLENGTH]);
                return NULL;
            }
            ptr = (char*)malloc(len+1);
            macromemsize += len+1;
            subs = (char*)malloc(MACROARGLENGTH+1);
            macromemsize += MACROARGLENGTH+1;
            if((ptr == NULL) || (subs == NULL)) return NULL;
            strcpy(ptr, arguments + i*(MACROARGLENGTH+1));
            tmp->arguments[i] = ptr;
            tmp->substitutions[i] = subs;
        }
    }

    index = hash(name) % MACRO_TABLESIZE;
    try = macroTable[index];

    // First item on index
    if(try == NULL) {
        macroTable[index] = tmp;
        macroTableCounter++;
        return tmp;
    }

    // Collision on index, place at end of linked list if unique
    while(true) {
        if(strcmp(try->name, name) == 0) {
            error(message[ERROR_MACRODEFINED]);
            return NULL;
        }
        if(try->next) {
            try = try->next;
        }
        else {
            try->next = tmp;
            macroTableCounter++;
            return tmp;
        }
    }
}

uint8_t macroExpandArg(char *dst, char *src, macro_t *m) {
    uint8_t i;

    for(i = 0; i < m->argcount; i++) {
        if(strcmp(src, m->arguments[i]) == 0) {
            strcpy(dst, m->substitutions[i]);
            return strlen(dst);
        }
    }
    return 0;
}
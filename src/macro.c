#include "macro.h"
#include "globals.h"

// Total allocated memory for macros
uint24_t macromemsize;

// tables
macro_t macroTable[MAXIMUM_MACROS]; // indexed table
uint8_t macroTableCounter;

void initMacros(void) {
    macromemsize = 0;
    macroTableCounter = 0;
    memset(macroTable, 0, sizeof(macroTable));
}

int findMacroIndex(char *key) {
    int base = 0;
	int lim, cmp;
    int p;

	for (lim = macroTableCounter; lim != 0; lim >>= 1) {
		p = base + (lim >> 1);
		cmp = strcmp(key,macroTable[p].name);
		if (cmp == 0)
			return p;
		if (cmp > 0) {
			base = p + 1;
			lim--;
		}
	}
	return (MAXIMUM_MACROS);
}

macro_t* findMacro(char *name){
    uint8_t index = findMacroIndex(name);
    if(index < MAXIMUM_MACROS) return &macroTable[index];
    else return NULL;
}

void setMacroBody(macro_t *macro, const char *body) {
    macro->body = (char*)malloc(strlen(body)+1);
    if(macro->body == NULL) {
        error(message[ERROR_MACROMEMORYALLOCATION]);
        return;
    }
    strcpy(macro->body, body);
}

macro_t * defineMacro(char *name, uint8_t argcount, char *arguments) {
    int len,i,j;
    int index;
    char *ptr,*subs;
    macro_t newmacro,tempmacro;

    index = findMacroIndex(name);

    if(index >= MAXIMUM_MACROS) {
        if(macroTableCounter < MAXIMUM_MACROS) {
            // prep new macro structure
            len = strlen(name);
            if(len > MAXNAMELENGTH) {
                error(message[ERROR_MACRONAMELENGTH]);
                return NULL;
            }
            newmacro.name = (char*)malloc(len+1);
            macromemsize += len+1;
            if(newmacro.name == NULL) return NULL;
            strcpy(newmacro.name, name);
            newmacro.argcount = argcount;
            newmacro.substitutions = NULL;
            if(argcount == 0) newmacro.arguments = NULL;
            else {
                newmacro.arguments = (char **)malloc(argcount * sizeof(char *)); // allocate array of char*
                macromemsize += argcount * sizeof(char *);
                newmacro.substitutions = (char **)malloc(argcount * sizeof(char *));
                macromemsize += argcount * sizeof(char *);
                if((newmacro.arguments == NULL) || (newmacro.substitutions == NULL)) return NULL;
                for(j = 0; j < argcount; j++) {
                    len = strlen(arguments + j*(MACROARGLENGTH+1));
                    if(len > MACROARGLENGTH) {
                        error(message[ERROR_MACROARGLENGTH]);
                        return NULL;
                    }
                    ptr = (char*)malloc(len+1);
                    macromemsize += len+1;
                    subs = (char*)malloc(MACROARGLENGTH+1);
                    macromemsize += MACROARGLENGTH+1;
                    if((ptr == NULL) || (subs == NULL)) return NULL;
                    strcpy(ptr, arguments + j*(MACROARGLENGTH+1));
                    newmacro.arguments[j] = ptr;
                    newmacro.substitutions[j] = subs;
                }
            }
            // start ordered placement in array
            for(i = 0; i < macroTableCounter; i++) {
                if(strcmp(name, macroTable[i].name) < 0) break;
            }
            if(i < macroTableCounter) { // ordered swap out
                for(; i < macroTableCounter; i++ ) {
                    tempmacro = macroTable[i];
                    macroTable[i] = newmacro;
                    newmacro = tempmacro;
                }
            }
            macroTable[macroTableCounter++] = newmacro;
            return findMacro(name);
        }
        else error(message[ERROR_MAXMACROS]);
    }
    else error(message[ERROR_MACRODEFINED]);
    return NULL;
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
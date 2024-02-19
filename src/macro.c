#include "./macro.h"
#include "globals.h"
// tables
macro_t macroTable[MAXIMUM_MACROS]; // indexed table
uint8_t macroTableCounter;

void initMacros(void) {
    int i;

    macroTableCounter = 0;
    for(i = 0; i < MAXIMUM_MACROS; i++){
        macroTable[i].name = NULL;
        macroTable[i].argcount = 0;
        macroTable[i].arguments = NULL;
        macroTable[i].substitutions = NULL;
    }
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

bool defineMacro(char *name, uint8_t argcount, char *arguments) {
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
                return false;
            }
            newmacro.name = agon_malloc(len+1);
            if(newmacro.name == NULL) return false;
            strcpy(newmacro.name, name);
            newmacro.argcount = argcount;
            newmacro.substitutions = NULL;
            if(argcount == 0) newmacro.arguments = NULL;
            else {
                newmacro.arguments = (char **)agon_malloc(argcount * sizeof(char *)); // allocate array of char*
                newmacro.substitutions = (char **)agon_malloc(argcount * sizeof(char *));
                if((newmacro.arguments == NULL) || (newmacro.substitutions == NULL)) return false;
                for(j = 0; j < argcount; j++) {
                    len = strlen(arguments + j*(MACROARGLENGTH+1));
                    if(len > MACROARGLENGTH) {
                        error(message[ERROR_MACROARGLENGTH]);
                        return false;
                    }
                    ptr = agon_malloc(len+1);
                    subs = agon_malloc(MACROARGLENGTH+1);
                    if((ptr == NULL) || (subs == NULL)) return false;
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
            macroTable[macroTableCounter] = newmacro;
            macroTableCounter++;
            return true;
        }
        else error(message[ERROR_MAXMACROS]);
    }
    else error(message[ERROR_MACRODEFINED]);
    return false;
}

// Find an 'argument' from a macro_tfile, and replace by the correct substitution string
// If nothing is found, nothing is replaced - the argument may be something else entirely
/*
void macroArgFindSubst(char *op, macro_t*m) {
    uint8_t i;

    for(i = 0; i < m->argcount; i++) {
        if(strcmp(op, m->arguments[i]) == 0) {
            strcpy(op, m->substitutions[i]);
            //printf("DEBUG expand - length <%d>\n", strlen(op));
            return;
        }
    }
}
*/
uint8_t macroExpandArg(char *dst, char *src, macro_t *m) {
    uint8_t i;

    for(i = 0; i < m->argcount; i++) {
        if(strcmp(src, m->arguments[i]) == 0) {
            strcpy(dst, m->substitutions[i]);
            //printf("DEBUG expand - length <%d>\n", strlen(dst));
            return strlen(dst);
        }
    }
    return 0;
}
#include "./macro.h"

// tables
macro macroTable[MAXIMUM_MACROS]; // indexed table
uint8_t macroTableCounter;

void initMacros(void) {
    int i;

    macroTableCounter = 0;
    for(i = 0; i < MAXIMUM_MACROS; i++){
        macroTable[i].name = NULL;
        macroTable[i].argcount = 0;
        macroTable[i].arguments = NULL;
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

macro * findMacro(char *name){
    uint8_t index = findMacroIndex(name);
    if(index < MAXIMUM_MACROS) return &macroTable[index];
    else return NULL;
}

bool defineMacro(char *name, uint8_t argcount, char **arguments) {
    int len,i,j;
    int index;
    char *ptr;
    macro newmacro,tempmacro;

    index = findMacroIndex(name);

    if(index >= MAXIMUM_MACROS) {
        if(macroTableCounter < MAXIMUM_MACROS) {
            // prep new macro structure
            len = strlen(name);
            newmacro.name = agon_malloc(len+1);
            if(newmacro.name == NULL) return false;
            strcpy(newmacro.name, name);
            newmacro.argcount = argcount;
            if(argcount == 0) newmacro.arguments = NULL;
            else {
                newmacro.arguments = (char **)agon_malloc(argcount * sizeof(char *)); // allocate array of char*
                if(newmacro.arguments == NULL) return false;
                for(j = 0; j < argcount; j++) {
                    len = strlen(arguments[j]);
                    ptr = agon_malloc(len+1);
                    if(ptr == NULL) return false;
                    strcpy(ptr, arguments[j]);
                    newmacro.arguments[j] = ptr;
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

void getMacroFilename(char *filename, char *name) {
    strcpy(filename, "macro.");
    strcat(filename, name);
}
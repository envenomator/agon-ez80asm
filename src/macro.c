#include "macro.h"
#include "globals.h"
#include "hash.h"
#include "config.h"
#include "instruction.h"
#include "moscalls.h"

// Total allocated memory for macros
uint24_t macromemsize;
uint8_t macroCounter;

void initMacros(void) {
    macromemsize = 0;
    macroCounter = 0;
}

void setMacroBody(macro_t *macro, const char *body) {
    macro->body = (char*)malloc(strlen(body)+1);
    if(macro->body == NULL) {
        error(message[ERROR_MACROMEMORYALLOCATION],0);
        return;
    }
    strcpy(macro->body, body);
}

macro_t *defineMacro(char *name, uint8_t argcount, char *arguments, uint16_t startlinenumber) {
    unsigned int len, i;
    uint8_t index;
    char *ptr,*subs;
    macro_t *tmp;
    instruction_t *try, *macroinstruction;

    // allocate space in buffer for macro_t struct
    tmp = (macro_t *)malloc(sizeof(macro_t));
    macromemsize += sizeof(macro_t);
    if(tmp == 0) return NULL;

    macroinstruction = (instruction_t *)malloc(sizeof(instruction_t));
    macromemsize += sizeof(instruction_t);
    if(macroinstruction == 0) return NULL;

    // Link together
    macroinstruction->type = MACRO;
    macroinstruction->macro = tmp;

    len = (unsigned int)strlen(name)+1;
    macroinstruction->name = (char *)malloc(len);
    macromemsize += len;
    if(macroinstruction->name == 0) return NULL;

    tmp->name = macroinstruction->name;
    strcpy(macroinstruction->name, name);
    macroinstruction->next = NULL;

    // Set up macro specific content
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
            len = (unsigned int)strlen(arguments + i*(MACROARGLENGTH+1));
            if(len > MACROARGLENGTH) {
                error(message[ERROR_MACROARGLENGTH],0);
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
    tmp->originfilename = currentcontentitem->name;
    tmp->originlinenumber = startlinenumber;

    index = lowercaseHash256(name);
    try = instruction_table[index];

    // First item on index
    if(try == NULL) {
        instruction_table[index] = macroinstruction;
        macroCounter++;
        return tmp;
    }

    // Collision on index, place at end of linked list if unique
    while(true) {
        if(fast_strcasecmp(try->name, name) == 0) {
            error(message[ERROR_MACRODEFINED],"%s",name);
            return NULL;
        }
        if(try->next) {
            try = try->next;
        }
        else {
            try->next = macroinstruction;
            macroCounter++;
            return tmp;
        }
    }
}

void replaceSubstring(char *target, const char *needle, const char *replacement)
{
    char buffer[MACROARGLENGTH + 1] = { 0 };
    char *insert_point = &buffer[0];
    const char *tmp = target;
    size_t needle_len = strlen(needle);
    size_t replacement_len = strlen(replacement);

    while (1) {
        const char *p = strstr(tmp, needle);

        // walked past last occurrence of needle; copy remaining part
        if (p == NULL) {
            strcpy(insert_point, tmp);
            break;
        }

        // copy part before needle
        memcpy(insert_point, tmp, p - tmp);
        insert_point += p - tmp;

        // copy replacement string
        memcpy(insert_point, replacement, replacement_len);
        insert_point += replacement_len;

        // adjust pointers, move on
        tmp = p + needle_len;
    }

    // write altered string back to target
    strcpy(target, buffer);
}

uint8_t macroExpandArg(char *dst, char *src, macro_t *m) {

    strcpy(dst, src);
    for(uint8_t i = 0; i < m->argcount; i++) {
        replaceSubstring(dst, m->arguments[i], m->substitutions[i]);
    }
    return strlen(dst);
}
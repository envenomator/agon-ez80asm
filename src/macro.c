#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "config.h"
#include "defines.h"
#include "utils.h"
#include "macro.h"
#include "globals.h"
#include "hash.h"
#include "config.h"
#include "instruction.h"
#include "moscalls.h"

// Total allocated memory for macros
uint24_t macromemsize;
uint8_t macroCounter;

// internal tracking number per expansion. Starts at 0 and sequentially increases each expansion to create a macro expansion scope (for labels)
uint24_t macroExpandID;

void initMacros(void) {
    macromemsize = 0;
    macroCounter = 0;
}

void setMacroBody(macro_t *macro, const char *body) {
    macro->body = (char*)allocateMemory(strlen(body)+1);
    if(macro->body == NULL) return;
    strcpy(macro->body, body);
}

macro_t *defineMacro(const char *name, uint8_t argcount, const char *arguments, uint16_t startlinenumber) {
    unsigned int len, i;
    uint8_t index;
    char *ptr;
    macro_t *tmp;
    instruction_t *try, *macroinstruction;

    // allocate space in buffer for macro_t struct
    tmp = (macro_t *)allocateMemory(sizeof(macro_t));
    macromemsize += sizeof(macro_t);
    if(tmp == NULL) return NULL;

    macroinstruction = (instruction_t *)allocateMemory(sizeof(instruction_t));
    macromemsize += sizeof(instruction_t);
    if(macroinstruction == NULL) return NULL;

    // Link together
    macroinstruction->type = MACRO;
    macroinstruction->macro = tmp;

    len = (unsigned int)strlen(name)+1;
    macroinstruction->name = (char *)allocateMemory(len);
    macromemsize += len;
    if(macroinstruction->name == NULL) return NULL;

    tmp->name = macroinstruction->name;
    strcpy(macroinstruction->name, name);
    macroinstruction->next = NULL;

    // Set up macro specific content
    tmp->argcount = argcount;
    tmp->substitutions = NULL;
    if(argcount == 0) tmp->arguments = NULL;
    else {
        tmp->arguments = (char **)allocateMemory(argcount * sizeof(char *)); // allocate array of char*
        macromemsize += argcount * sizeof(char *);
        if(tmp->arguments == NULL) return NULL;
        tmp->substitutions = (char **)allocateMemory(argcount * sizeof(char *));
        macromemsize += argcount * sizeof(char *);
        if(tmp->substitutions == NULL) return NULL;
        if((tmp->arguments == NULL) || (tmp->substitutions == NULL)) return NULL;

        const char *argptr = arguments;
        for(i = 0; i < argcount; i++, argptr += MACROARGLENGTH+1) {
            len = (unsigned int)strlen(argptr);
            if(len > MACROARGLENGTH) {
                error(message[ERROR_MACROARGLENGTH],0);
                return NULL;
            }
            ptr = (char*)allocateMemory(len+1);
            macromemsize += len+1;
            if(ptr == NULL) return NULL;

            strcpy(ptr, argptr);
            tmp->arguments[i] = ptr;
            tmp->substitutions[i] = NULL;
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

// replace the 'argument' substring in a target string, with the 'substitution' substring
// substitution will only happen on a full word match in the target string:
// An argument 'word' is a concatenation of alphanumerical characters, ending in a non-alphanumerical character
// Example arguments:
// - word
// - wOrD8
// - arg1
void replaceArgument(char *target, const char *argument, const char *substitution)
{
    char buffer[MACROARGSUBSTITUTIONLENGTH + 1] = { 0 };
    bool bufferdirty = false;
    char *insert_point = &buffer[0];
    const char *tmp = target;
    size_t argument_len = strlen(argument);
    size_t substitution_len = strlen(substitution);

    while (1) {
        const char *p = strstr(tmp, argument);

        // walked past last occurrence of argument; copy remaining part
        if (p == NULL) {
            strcpy(insert_point, tmp);
            break;
        }

        // copy part before potential argument
        memcpy(insert_point, tmp, p - tmp);
        insert_point += p - tmp;

        if(isalnum(*(p + argument_len)) == 0) {
            // whole word argument match - copy substitution string
            memcpy(insert_point, substitution, substitution_len);
            insert_point += substitution_len;            
            bufferdirty = true;
        }
        else {
            // part match - copy original string
            memcpy(insert_point, argument, argument_len);
            insert_point += argument_len;
        }

        // adjust pointers, move on
        tmp = p + argument_len;
    }

    // write altered string back to target
    if(bufferdirty) strcpy(target, buffer);
}

uint8_t macroExpandArg(char *dst, const char *src, const macro_t *m) {
    strcpy(dst, src);
    for(uint8_t i = 0; i < m->argcount; i++) {
        replaceArgument(dst, m->arguments[i], m->substitutions[i]);
    }
    return strlen(dst);
}
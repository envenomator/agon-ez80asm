#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "globals.h"
#include "console.h"
#include "utils.h"
#include "str2num.h"
#include "label.h"
#include "instruction.h"
#include <stdint.h>
#include "io.h"

// return a base filename, stripping the given extension from it
void remove_ext (char* myStr, char extSep, char pathSep) {
    char *lastExt, *lastPath;
    // Error checks.
    if (myStr == NULL) return;
    // Find the relevant characters.
    lastExt = strrchr (myStr, extSep);
    lastPath = (pathSep == 0) ? NULL : strrchr (myStr, pathSep);
    // If it has an extension separator.
    if (lastExt != NULL) {
        // and it's to the right of the path separator.
        if (lastPath != NULL) {
            if (lastPath < lastExt) {
                // then remove it.
                *lastExt = '\0';
            }
        } else {
            // Has extension separator with no path separator.
            *lastExt = '\0';
        }
    }
}

void error(char* msg) {
    if(!global_errors) {
        vdp_set_text_colour(DARK_RED);
        if(currentExpandedMacro) printf("MACRO [%s]",currentExpandedMacro->name);
        else if(strlen(filename[FILE_CURRENT])) printf("\"%s\"", filename[FILE_CURRENT]);
        if(linenumber) printf(" - line %d - ", linenumber);
        printf("%s\r\n",  msg);
        vdp_set_text_colour(BRIGHT_WHITE);
        global_errors++;
    }
}

void trimRight(char *str) {
    while(*str) str++;
    str--;
    while(isspace(*str)) str--;
    str++;
    *str = 0;
}

typedef enum {
    TOKEN_REGULAR,
    TOKEN_STRING,
    TOKEN_LITERAL,
    TOKEN_BRACKET
} tokenclass;

// further parse a command-token string to currentline.mnemonic & currentline.suffix
void parse_command(char *src) {
    currentline.mnemonic = src;

    while(*src && (*src != '.')) src++;
    if(*src) {
        // suffix start found
        *src = 0; // terminate mnemonic
        currentline.suffixpresent = true;
        currentline.suffix = src + 1;
        return;
    }
    // no suffix found
    currentline.suffixpresent = false;
    currentline.suffix = NULL;
    return;
}

void getLabelToken(streamtoken_t *token, char *src) {
    token->start = src; // no need to remove leading spaces
    while(*src && (*src != ':') && (*src != ';')) src++;
    token->terminator = *src;
    token->next = src+1;
    *src = 0;

    return;
}
// fill the streamtoken_t object, according to the stream
// returns the number of Mnemonic characters found, or 0 if none
uint8_t getMnemonicToken(streamtoken_t *token, char *src) {
    uint8_t length = 0;

    // skip leading space
    while(*src && (isspace(*src))) src++;
    if(*src == 0) {
        memset(token, 0, sizeof(streamtoken_t));
        return 0;
    }
    token->start = src;
    while(!isspace(*src) && (*src != ';') && *src) {
        length++;
        src++;
    }
    token->terminator = *src;
    if(*src) token->next = src+1;
    else token->next = NULL;

    *src = 0; // terminate stream
    return length;
}

// point to one position after regular token, or to '0' in string
char * _findRegularTokenEnd(char *src) {
    while(*src) {
        if((*src == ';') || (*src == ',')) break;
        src++;
    }
    return src;
}

// point to one position after string token, or to '0' in the string
char * _findStringTokenEnd(char *src) {
    bool escaped = false;
    while(*src) {
        if(*src == '\\') escaped = !escaped;
        if((*src == '\"') && !escaped) break;
        src++;
    }
    if(*src) return src;
    else return src+1;
}
// point to one position after literal token, or to '0' in the string
char * _findLiteralTokenEnd(char *src) {
    bool escaped = false;
    while(*src) {
        if(*src == '\'') escaped = !escaped;
        if((*src == '\'') && !escaped) break;        
        src++;
    }
    if(*src) return src;
    else return src+1;
}


// fill the streamtoken_t object, parse it as an operand
// returns the number of Operator characters found, or 0 if none
uint8_t getOperandToken(streamtoken_t *token, char *src) {
    uint8_t length = 0;
    //bool normalmode = true;

    // skip leading space
    while(*src && (isspace(*src))) src++;
    if(*src == 0) {
        memset(token, 0, sizeof(streamtoken_t));
        return 0;
    }
    token->start = src;

    // hunt for end-character (0 , or ; in normal non-literal mode)
    while(*src) {
        //if(*src == '\'') normalmode = !normalmode;
        //if((normalmode) && ((*src == ',') || (*src == ';'))) break;
        if((*src == ',') || (*src == ';')) break;
        src++;
        length++;
    }

    token->terminator = *src;
    if(*src) token->next = src+1;
    else token->next = NULL;

    if(length) {
        *src-- = 0; // terminate early and revert one character
        while(isspace(*src)) { // remove trailing space(s)
            *src-- = 0; // terminate on trailing spaces
            if(length-- == 0) break;
        }
    }
    return length;
}

uint8_t getDefineValueToken(streamtoken_t *token, char *src) {
    uint8_t length = 0;
    tokenclass state;
    bool escaped = false;
    bool terminated;

    if(src == NULL) {
        memset(token, 0, sizeof(streamtoken_t));
        return 0;
    }

    // skip leading space
    while(*src && (isspace(*src))) src++;
    if(*src == 0) {
        memset(token, 0, sizeof(streamtoken_t));
        return 0;
    }
    token->start = src;

    switch(*src) {
        case '\"':
            state = TOKEN_STRING;
            src++;
            break;
        case '\'':
            state = TOKEN_LITERAL;
            src++;
            break;
        case '(':
            state = TOKEN_BRACKET;
            src++;
            break;
        default:
            state = TOKEN_REGULAR;
    }

    while(*src) {
        terminated = false;
        switch(state) {
            case TOKEN_STRING:
                switch(*src) {
                    case '\\':
                        escaped = !escaped;
                        break;
                    case '\"':
                        if(!escaped) state = TOKEN_REGULAR;
                        escaped = false;
                        break;
                    default:
                        escaped = false;
                        break;
                }
                break;
            case TOKEN_LITERAL:
                switch(*src) {
                    case '\\':
                        escaped = !escaped;
                        break;
                    case '\'':
                        if(!escaped) state = TOKEN_REGULAR;
                        escaped = false;
                        break;
                    default:
                        escaped = false;
                        break;
                }
                break;
            case TOKEN_BRACKET:
                if(*src == ')') state = TOKEN_REGULAR;
                break;
            case TOKEN_REGULAR:
                terminated = ((*src == ';') || (*src == ',') || (*src == '='));
                break;            
        }
        if(terminated) break;
        src++;
        length++;
    }

    token->terminator = *src;
    if(*src) token->next = src+1;
    else token->next = NULL;

    if(length) {
        *src-- = 0; // terminate early and revert one character
        while(isspace(*src)) { // remove trailing space(s)
            *src-- = 0; // terminate on trailing spaces
            if(length-- == 0) break;
        }
    }
    if(state == TOKEN_STRING) error(message[ERROR_STRING_NOTTERMINATED]);
    return length;
}

// operator tokens assume the following input
// - no ',' ';' terminators
// - only 'operator' terminators or a 0
uint8_t getOperatorToken(streamtoken_t *token, char *src) {
    uint8_t length = 0;
    bool normalmode = true;
    bool shift = false, error = false;

    // skip leading space
    while(*src && (isspace(*src))) src++;

    if(*src == 0) { // empty string
        memset(token, 0, sizeof(streamtoken_t));
        return 0;
    }
    token->start = src;

    // check for literal mode at start, tokens like '\n'
    if(*src == '\'') {
        src++;
        length++;
        normalmode = false;
    }

    while(*src) {
        if(*src == '\'') normalmode = !normalmode;
        if(normalmode && strchr("+-*<>&|^~/",*src)) { // terminator found
            if((*src == '<') || (*src == '>')) {                
                if(*(src+1) == *src) {
                    shift = true;
                }
                else {
                    error = true;
                }
            }
            break;
        }
        src++;
        length++;
    }

    if(!error) token->terminator = *src;
    else {
        token->terminator = '!';
    }
    if(*src) token->next = shift?src+2:src+1;
    else token->next = NULL;

    if(length) {
        *src-- = 0; // terminate early and revert one character
        while(isspace(*src)) { // remove trailing space(s)
            *src-- = 0; // terminate on trailing spaces
            if(length-- == 0) break;
        }
    }
    return length;
}
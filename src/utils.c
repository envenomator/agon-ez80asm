#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "globals.h"
#include "utils.h"

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


bool isempty(const char *str){
    return (str[0] == '\0');
}

bool notempty(const char *str) {
    return (str[0] != '\0');
}

void error(char* msg)
{
    printf("Error in line %d - %s\n", linenumber, msg);
    global_errors++;
}

void debugmsg(char *msg)
{
    if(pass == 1) {
        printf("DEBUG - Line %d - %s\n", linenumber, msg);
    }
}

void trimEdges(char *str) {
    uint8_t index = 0;
    char *target = str;
    // skip leading space
    while(*str) {
        if((*str == ' ') || (*str == '\n') || (*str == '\r') || (*str == '\t')) {
            str++;
        }
        else break;
    }
    // start copying data back
    while(*str) {
        *target++ = *str++;
        index++;
    }
    // remove trailing spaces
    while(index--) {
        target--;
        if((*target != ' ') && (*target != '\n') && (*target != '\r') && (*target != '\t')) {
            target++;
            break;
        }
    }
    *target = 0;     // close out token
}

void parse_comment(char *comment, char *src) {
    char *target;
    uint8_t index = 0;

    target = src;
    // remove leading space
    while(*src) {
        if((*src == ' ') || (*src == '\n') || (*src == '\r') || (*src == '\t')) {
            src++;
        }
        else break;
    }
    while(*src) {
        *target++ = *src++;
        index++;
    }
    // remove trailing space
    while(index--) {
        target--;
        if((*target != ' ') && (*target != '\n') && (*target != '\r') && (*target != '\t')) {
            target++;
            break;
        }
    }
    *target = 0;     // close out token
}
// Find a token, ending with delimiter character
// token is copied over and species are trimmed from it
// Source string is not altered in any way
//
// A token is parses as a literal string, when the first non-space character is a double quote
//
// Returns:
// Pointer to next token in source or NULL
char *parse_token(char *token, char  *src, char delimiter, bool required) {
    char *target;
    uint8_t index = 0;
    bool found = false;
    bool escaped = false;
    bool string = false;

    target = token;
    // remove leading space
    while(*src) {
        if((*src == ' ') || (*src == '\n') || (*src == '\r') || (*src == '\t')) {
            src++;
        }
        else break;
    }
    if(*src == '\"') {
        string = true;
        *target++ = *src++;
    }
    // copy potential token
    while(*src) {
        if(string) {
            switch(*src) {
                case '\\':
                    escaped = !escaped;
                    break;
                case '\"':
                    if(!escaped) {
                        found = true;
                        string = false;
                    }
                    escaped = false;
                    break;
                default:
                    escaped = false;
                    break;
            }
        }
        else {
            if(*src == delimiter) {
                found = true;
                break;
            }
            if(*src == ';') break;
        }
        *target++ = *src++;
        index++;
        if(found == true) break;
    }
    // finalize found or remaining token
    if(found || !required) {
        // remove trailing space
        while(index--) {
            target--;
            if((*target != ' ') && (*target != '\n') && (*target != '\r') && (*target != '\t')) {
                target++;
                break;
            }
        }
        *target = 0;     // close out token
        if(found) {
            if(*src) return src+1;
            return NULL;
        }
        else return NULL;
    }
    // no result
    *token = 0; // empty token
    return NULL;
}

typedef enum {
    TOKEN_REGULAR,
    TOKEN_STRING,
    TOKEN_BRACKET
} tokenclass;

// Find a token, ending with delimiter character
// token is copied over and species are trimmed from it
// Source string is not altered in any way
//
// Returns:
// number of bytes copied into token, excluding the 0 terminator
static inline bool isTerminator(char t) {
    return ((t == ' ') || (t == ',') || (t == ':') || (t == ';') || (t == '='));
}

static inline bool isNonSpaceTerminator(char t) {
    return ((t == ',') || (t == ':') || (t == ';') || (t == '='));
}

// split a 'command.suffix' token in two parts 
void split_suffix(char *mnemonic, char *suffix, char *buffer) {
    bool cmd = true;

    while(*buffer) {
        if(cmd) {
            *mnemonic = *buffer;
            if(*buffer == '.') cmd = false;
            else mnemonic++;
        }
        else *suffix++ = *buffer;
        buffer++;
    }
    *suffix = 0;
    *mnemonic = 0;
}

uint8_t get_token(tokentype *token, char *src) {
    char *target;
    uint8_t index = 0;
    bool escaped = false;
    bool terminated;
    tokenclass state;

    // remove leading space
    while(*src) {
        if(isspace(*src) != 0) src++;
        else break;
    }
    if(*src == 0) { // empty string
        token->terminator = 0;
        token->start[0] = 0;
        token->next = NULL;
        return 0;
    }
    if(isNonSpaceTerminator(*src)) {
        token->terminator = *src;
        token->start[0] = 0;
        token->next = src+1;
        return 0;
    }
    // copy over the token itself, taking care of the character state within the token
    state = TOKEN_REGULAR;
    target = token->start;
    while(true) {
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
            case TOKEN_BRACKET:
                if(*src == ')') state = TOKEN_REGULAR;
                break;
            case TOKEN_REGULAR:
                if(*src == '\"') state = TOKEN_STRING;
                if(*src == '(') state = TOKEN_BRACKET;
                terminated = isTerminator(*src);
                break;            
        }
        terminated |= (*src == 0);
        if(terminated) {
            token->terminator = *src;
            break;
        }
        *target++ = *src++;
        index++;
    }
    if(*src == ' ') { // Are we terminated at a space between tokens, or still another terminator?
        while(isspace(*src) != 0) src++;
        if(*src == 0) token->terminator = 0;
        else {
            if(isNonSpaceTerminator(*src)) {
                if(*src == ':') { // ':' can't have spaces before, so can't terminate here
                    token->terminator = ' ';
                    src--; // at a ':' after a few spaces
                }
                else token->terminator = *src;
            }
            else src--; // at a regular character
        }
    }
    // remove trailing space
    while(index--) {
        target--;
        if(isspace(*target) == 0) {
            target++;
            index++;
            break;
        }
    }
    *target = 0;
    if(*src == 0) token->next = NULL;
    else token->next = src+1;
    return index;
}

#include <stdio.h>
#include <string.h>
#include "globals.h"

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
        if(found) return src+1;
        else return NULL;
    }
    // no result
    *token = 0; // empty token
    return NULL;
}

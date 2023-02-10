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

void strstripleft(const char *source_str, char *dest_str)
{
    int index = 0, j, k = 0;
 
    while (source_str[index] == ' '
        || source_str[index] == '\t'
        || source_str[index] == '\n'){
        index++;
    }
    for (j = index; source_str[j] != '\0'; j++){
        dest_str[k] = source_str[j];
        k++;
    } 
    dest_str[k] = '\0'; 
}

bool isempty(const char *str){
    return (str[0] == '\0');
}

bool notempty(const char *str) {
    return (str[0] != '\0');
}

void error(char* msg)
{
    if(pass == 1) {
        printf("asm - line %d - %s\n", linenumber, msg);
        global_errors++;
    }
}

void debugmsg(char *msg)
{
    if(pass == 1) {
        printf("DEBUG - Line %d - %s\n", linenumber, msg);
    }
}
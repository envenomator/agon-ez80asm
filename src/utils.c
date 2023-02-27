#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "globals.h"
#include "utils.h"
#include "str2num.h"
#include "label.h"
#include "instruction.h"

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


bool isEmpty(const char *str){
    return (str[0] == '\0');
}

bool notEmpty(const char *str) {
    return (str[0] != '\0');
}

void error(char* msg)
{
    printf("\"%s\" - line %d - %s\n", currentInputFilename, linenumber, msg);
    global_errors++;
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
    TOKEN_BRACKET
} tokenclass;

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

uint8_t getLineToken(tokentype *token, char *src, char terminator) {
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
        token->length = 0;
        token->next = NULL;
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
                terminated = ((*src == ';') || (*src == terminator));
                break;            
        }
        terminated = terminated || (*src == 0);
        if(terminated) {
            token->terminator = *src;
            break;
        }
        *target++ = *src++;
        index++;
    }
    // remove trailing space
    while(index) {
        target--;
        if(isspace(*target) == 0) {
            target++;
            break;
        }
        index--;
    }
    *target = 0;
    if(*src == 0) token->next = NULL;
    else token->next = src+1;
    token->length = index;
    return index;
}

uint8_t getOperatorToken(tokentype *token, char *src) {
    char *target;
    uint8_t index = 0;

    // remove leading space
    while(*src) {
        if(isspace(*src) != 0) src++;
        else break;
    }
    if(*src == 0) { // empty string
        token->terminator = 0;
        token->start[0] = 0;
        token->length = 0;
        token->next = NULL;
        return 0;
    }
    // copy content
    target = token->start;
    while(true) {
        if((*src == 0) || (*src == '+') || (*src == '-') || (*src == '<') || (*src == '>')) {
            if(((*src == '<') || (*src == '>'))) {
                if((*(src+1) == *src)) src += 1;
                else {
                    token->terminator = '!'; // ERROR
                    break;
                }
            }
            token->terminator = *src;
            break;
        }
        *target++ = *src++;
        index++;
    }
    // remove trailing space
    while(index) {
        target--;
        if(isspace(*target) == 0) {
            target++;
            break;
        }
        index--;
    }
    *target = 0;
    if(*src == 0) token->next = NULL;
    else token->next = src+1;
    token->length = index;
    return index;
}


bool openFile(FILE **file, char *name, char *mode) {
    *file = fopen(name, mode);

    if(*file) return true;
    printf("Error opening \"%s\"\n", name);
    return false;
}

bool reOpenFile(uint8_t number, char *mode) {
    if(filehandle[number]) fclose(filehandle[number]);
    return openFile(&filehandle[number], filename[number], mode);
}

void prepare_filenames(char *input_filename) {
    // prepare filenames
    strcpy(filename[FILE_INPUT], input_filename);
    strcpy(filename[FILE_OUTPUT], input_filename);
    remove_ext(filename[FILE_OUTPUT], '.', '/');
    strcpy(filename[FILE_LOCAL_LABELS], filename[FILE_OUTPUT]);
    strcpy(filename[FILE_ANONYMOUS_LABELS],filename[FILE_OUTPUT]);
    strcpy(filename[FILE_LISTING],filename[FILE_OUTPUT]);
    strcat(filename[FILE_OUTPUT], ".bin");
    strcat(filename[FILE_LOCAL_LABELS], ".lcllbls");
    strcat(filename[FILE_ANONYMOUS_LABELS], ".anonlbls");
    strcat(filename[FILE_LISTING], ".lst");
}

void closeAllFiles() {
   // Cleanup
    if(filehandle[FILE_INPUT]) fclose(filehandle[FILE_INPUT]);
    if(filehandle[FILE_OUTPUT]) fclose(filehandle[FILE_OUTPUT]);
    if(filehandle[FILE_LOCAL_LABELS]) fclose(filehandle[FILE_LOCAL_LABELS]);
    if(filehandle[FILE_ANONYMOUS_LABELS]) fclose(filehandle[FILE_ANONYMOUS_LABELS]);
    if(filehandle[FILE_LISTING]) fclose(filehandle[FILE_LISTING]);
    remove(filename[FILE_LOCAL_LABELS]);
    remove(filename[FILE_ANONYMOUS_LABELS]);
}

bool openfiles(void) {
    bool status = true;

    status = status && openFile(&filehandle[FILE_INPUT], filename[FILE_INPUT], "r");
    status = status && openFile(&filehandle[FILE_OUTPUT], filename[FILE_OUTPUT], "wb+");
    status = status && openFile(&filehandle[FILE_LOCAL_LABELS], filename[FILE_LOCAL_LABELS], "wb+");
    status = status && openFile(&filehandle[FILE_ANONYMOUS_LABELS], filename[FILE_ANONYMOUS_LABELS], "wb+");
    status = status && openFile(&filehandle[FILE_LISTING], filename[FILE_LISTING], "wb+");
    if(!status) closeAllFiles();
    return status;
}


char *agon_fgets(char *s, int size, uint8_t fileid) {
	int c;
	char *cs;

	cs = s;
	while (--size > 0 && (c = getc(filehandle[fileid])) != EOF)
		if ((*cs++ = c) == '\n')
			break;
	*cs = '\0';
	return (c == EOF && cs == s) ? NULL : s;
}

size_t agon_fwrite(void *ptr, size_t size, size_t nmemb, uint8_t fileid) {
    size_t n, s, result = 0;

    for(n = 0; n < nmemb; n++) {
        for(s = 0; s < size; s++) {
            if(putc((*(char *)ptr),filehandle[fileid]) == EOF) return result;
            ptr++;
            result++;
        }
    }
    return result;
}

size_t agon_fread(void *ptr, size_t size, size_t nmemb, uint8_t fileid) {
    size_t n, s, result = 0;

    for(n = 0; n < nmemb; n++) {
        for(s = 0; s < size; s++) {
            if((*(char *)ptr = getc(filehandle[fileid])) == EOF) {
                *(char *)ptr = 0;
                return result;
            }
            ptr++;
            result++;
        }
    }
    return result;
}
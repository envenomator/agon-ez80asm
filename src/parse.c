#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

void removeSpaces(char *str)
{
    // To keep track of non-space character count
    int count = 0;
 
    // Traverse the given string. If current character
    // is not space, then place it at index 'count++'
    for (int i = 0; str[i]; i++)
        if (str[i] != ' ')
            str[count++] = str[i]; // here count is
                                   // incremented
    str[count] = '\0';
}

void trimTrailing(char *str)
{
    int i;

	i=strlen(str)-1;
	while(i>-1)
	{
	  if(str[i]==' '||str[i]=='\t') i--;
	  else break;
	}
	str[i+1]='\0';
}

void trimLeading(char * str)
{
    int index, i;

    index = 0;

    /* Find last index of whitespace character */
    while(str[index] == ' ' || str[index] == '\t' || str[index] == '\n') index++;

    if(index != 0)
    {
        /* Shift all trailing characters to its left */
        i = 0;
        while(str[i + index] != '\0')
        {
            str[i] = str[i + index];
            i++;
        }
        str[i] = '\0';
    }
}

void trimEdges(char *str) {
    trimLeading(str);
    trimTrailing(str);
}
// Find a token, ending with delimiter character
// token is copied over and species are trimmed from it
// Source string is not altered in any way
//
// Returns:
// Pointer to next token in source or NULL
char *parse_token(char *token, char  *src, char delimiter, bool required) {
    char *target;
    uint8_t index = 0;
    bool found = false;

    target = token;
    // remove leading space
    while(*src) {
        if((*src == ' ') || (*src == '\n') || (*src == '\r') || (*src == '\t')) {
            src++;
        }
        else break;
    }
    // copy potential token
    while(*src) {
        if(*src == delimiter) {
            found = true;
            break;
        }
        *target++ = *src++;
        index++;
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

int main () {
    //char src[] = ":test.is  (ar g1)  , arg2  ,arg3, arg4  , arg5\n";
    //char src[] = "  label  :   test.is  ar g1  , arg2  ";
    char src[] = "label::test.is  (ar g1)  , arg2  ,arg3, arg4  , arg5\n";
    char buffer[32];

    char *next;

    next = parse_token(buffer, src, ':',true);
    if(buffer[0]) printf("Label: <<%s>>\n",buffer);
    else printf("No label found\n");

    if(next) next = parse_token(buffer, next, ' ', true);
    else next = parse_token(buffer, src, ' ', true);
    if(buffer[0]) {    
        printf("Command: <<%s>>\n", buffer);

        while(next) {
            next = parse_token(buffer, next, ',', false);
            if(buffer[0]) printf("Arg: <<%s>>\n",buffer);
        }

    }
    else printf("No command found\n");
    return(0);
}
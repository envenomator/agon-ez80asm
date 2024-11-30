#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include "globals.h"
#include "console.h"
#include "utils.h"
#include "str2num.h"
#include "label.h"
#include "instruction.h"
#include "io.h"
#include "assemble.h"
#include <stdarg.h>

// memory allocate size bytes, raise error if not available
void *allocateMemory(size_t size) {
    void *ptr = malloc(size);
    if(ptr == NULL) {
        error(message[ERROR_MEMORY],0);
    }
    return ptr;
}

// memory allocate a string, copy content and return pointer, or NULL if no memory
char *allocateString(char *name) {
    char *ptr = (char *)allocateMemory(strlen(name) + 1);
    if(ptr) {
        strcpy(ptr, name);
    }
    return ptr;
}

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

void displayerror(const char *msg, const char *context, uint8_t level) {
    struct contentitem *ci = currentContent();

    if((global_errors == 1) || (level == LEVEL_WARNING)) {
        if(ci) {
            if(currentExpandedMacro) {
                printf("Macro [%s] in \"%s\" line %d - ",currentExpandedMacro->name, currentExpandedMacro->originfilename, currentExpandedMacro->originlinenumber+macrolinenumber);
            }
            else {
                printf("File \"%s\" line %d - ", ci->name, ci->currentlinenumber);
            }
        }
        printf("%s", msg);
        if(strlen(context)) {
            if(level == LEVEL_WARNING)
                vdp_set_text_colour(BRIGHT_WHITE);
            else
                vdp_set_text_colour(DARK_YELLOW);
            printf(" \'%s\'", context);
        }
        printf("\r\n");
    }
    vdp_set_text_colour(BRIGHT_WHITE);
}

void error(const char *msg, const char *contextformat, ...) {
    char context[LINEMAX+1];

    if(contextformat) {
        va_list args;
        va_start(args, contextformat);
        vsprintf(context, contextformat, args);
        va_end(args);
    }
    else context[0] = 0;

    vdp_set_text_colour(DARK_RED);
    global_errors++;
    errorreportlevel = currentStackLevel();

    displayerror(msg, context, LEVEL_ERROR);
}

void warning(const char *msg, const char *contextformat, ...) {
    char context[LINEMAX+1];

    if(contextformat) {
        va_list args;
        va_start(args, contextformat);
        vsprintf(context, contextformat, args);
        va_end(args);
    }
    else context[0] = 0;

    vdp_set_text_colour(DARK_YELLOW);
    issue_warning = true;

    displayerror(msg, context, LEVEL_WARNING);
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
    bool escaped = false;

    // skip leading space
    while(*src && (isspace(*src))) src++;
    if(*src == 0) {
        memset(token, 0, sizeof(streamtoken_t));
        return 0;
    }
    token->start = src;

    // hunt for end-character (0 , or ; in normal non-literal mode)
    while(*src) {
        if(*src == '\'') escaped = !escaped;
        if(!escaped && ((*src == ',') || (*src == ';'))) break;
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

// Returns a bracketed token
// Example '[token]' -> returns 'token'0
// Assumes start on the '[' character
// Ends the token string in the stream on the matching ']' character, by zeroing it
uint8_t getBracketToken(streamtoken_t *token, char *src) {
    uint8_t length = 0;
    uint8_t opencount = 1;

    token->start = src+1;
    src++;
    while(*src) {
        if(*src == '[') opencount++;
        if(*src == ']') opencount--;
        if(opencount == 0) {
            token->next = src+1;
            *src = 0;
            break;
        }
        src++;
        length++;
    }
    if(opencount) return 0;
    else return length;
}

uint8_t getDefineValueToken(streamtoken_t *token, char *src) {
    uint8_t length = 0;
    tokenclass state;
    bool escaped = false;
    bool terminated;

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
            length++;
            break;
        case '\'':
            state = TOKEN_LITERAL;
            src++;
            length++;
            break;
        case '(':
            state = TOKEN_BRACKET;
            src++;
            length++;
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
    if(state == TOKEN_STRING) error(message[ERROR_STRING_NOTTERMINATED],0);
    return length;
}

void validateRange8bit(int32_t value, const char *name) {
    if(!(ignore_truncation_warnings)) {
        if((value > 0xff) || (value < -128)) {
            warning(message[WARNING_TRUNCATED_8BIT],"%s",name);
        }
    }
}

void validateRange16bit(int32_t value, const char *name) {
    if(!(ignore_truncation_warnings)) {
        if((value > 0xffff) || (value < -32768)) {
            warning(message[WARNING_TRUNCATED_16BIT],"%s",name);
        }
    }
}

void validateRange24bit(int32_t value, const char *name) {
    if(!(ignore_truncation_warnings)) {
        if((value > 0xffffff) || (value < -8388608)) {
            warning(message[WARNING_TRUNCATED_24BIT],"%s",name);
        }
    }
}

// Returns the value of an escaped character \c, or 255 if illegal
uint8_t getEscapedChar(char c) {
    switch(c) {
        case 'a':
            return(0x07); // Alert, beep
        case 'b':
            return(0x08); // Backspace
        case 'e':
            return(0x1b); // Escape
        case 'f':
            return(0x0c); // Formfeed
        case 'n':
            return(0x0a); // Newline
        case 'r':
            return(0x0d); // Carriage return
        case 't':
            return(0x09); // Horizontab tab
        case 'v':
            return(0x0b); // Vertical tab
        case '\\':
            return('\\'); // Backslash
        case '\'':
            return('\''); // Single quotation mark
        case '\"':
            return('\"'); // Double quotation mark
        case '?':
            return('?');  // Question mark
        default:
            return(0xff);
    }
}

// Get the ascii value from a single 'x' token.
uint8_t getLiteralValue(char *string) {
    uint8_t len = strlen(string);
    if((len == 3) && (string[2] == '\'')) {
        return string[1];
    }

    if((len == 4) && (string[3] == '\'')) {
        uint8_t c = getEscapedChar(string[2]);
        if(c == 0xff) {
            error(message[ERROR_ILLEGAL_ESCAPELITERAL],0);
            return 0;
        }
        return c;
    }

    error(message[ERROR_ASCIIFORMAT],0);
    return 0;
}

// Resolves a number from a string in this order:
// 1) Check if a label exists with this name
// 2) If not, try converting it to a number with str2num
int32_t resolveNumber(char *str, uint8_t length, bool req_firstpass) {
    int32_t number;
    label_t *lbl = findLabel(str);

    if((pass == 1) && !req_firstpass) return 0;

    if(lbl) number = lbl->address;
    else {
        if(*str == '\'') number = getLiteralValue(str);
        else {
            number = str2num(str, length?length:strlen(str));
            if(err_str2num) {
                if(pass == 1) {
                    // Yet unknown label, number incorrect
                    // We only get here if req_firstpass is true, so error
                    error(message[ERROR_INVALIDNUMBER],"%s", str);
                    return 0;
                }
                else {
                    // Unknown label and number incorrect
                    error(message[ERROR_INVALIDLABELORNUMBER], "%s", str);                            
                    return 0;
                }
            }
        }
    }
    return number;
}

// copy a variable-length literal from a literal string, starting with \', ending with the non-escaped \' character
// Examples tokenized:
// ''
// ' '
// '\n'
// '\''
// '   \'  ' -> this will copy, getLiteralValue will error out later
// '       0 -> this will copy, getLiteralValue will error out later
uint8_t copyLiteralToken(const char *from, char *to) {
    uint8_t length = 0;
    bool escaped = false;

    if(*from == '\'') {
        while(*from) {
            *to++ = *from++;
            length++;
            if(*from == '\\') escaped = !escaped;
            if(!escaped && (*from == '\'')) {
                length++;
                break;
            }
        }
        *to++ = *from;
    }
    *to = 0;
    return length;
}

enum getValueState {
    START,
    OP,
    UNARY,
    NUMBER,
};

// Gets the value from an expression, possible consisting of values, labels and operators
int32_t getExpressionValue(char *str, bool req_firstpass) {
    uint8_t tmplength;
    streamtoken_t token;
    char buffer[LINEMAX+1];
    char *bufptr;
    char operator, unaryoperator;
    int32_t tmp = 0;
    int32_t total = 0;
    enum getValueState state;;

    if((pass == 1) && !req_firstpass) return 0;

    while(isspace(*str)) str++; // eat all spaces

    operator = 0; // first implicit operator
    unaryoperator = 0;
    state = START;

    while(true) {
        switch(state) {
            case UNARY:
                unaryoperator = *str++;
                while(isspace(*str)) str++; // eat all spaces
                if(*str == 0) {
                    error(message[ERROR_MISSINGLABELORNUMBER],0);
                    return 0;
                }
                if(strchr("+-*/<>&|^~", *str)) {
                    error(message[ERROR_UNARYOPERATOR],0);
                    return 0;
                }
                state = NUMBER;
                break;
            case OP:
                if(operator) { // check 'dual-character' operator
                    error(message[ERROR_MISSINGLABELORNUMBER],0);
                    return 0;
                }

                if((*str == '<') || (*str == '>')) {                
                    if(*(str+1) != *str) {
                        error(message[ERROR_OPERATOR], 0);
                        return 0;
                    }
                    str++; // advance the double operator
                }

                unaryoperator = 0;
                operator = *str++;
                while(isspace(*str)) str++; // eat all spaces
                if(*str == 0) {
                    error(message[ERROR_MISSINGLABELORNUMBER],0);
                    return 0;
                }
                if(strchr("*/<>&|^", *str)) { // illegal unary
                    error(message[ERROR_UNARYOPERATOR], 0);
                    return 0;
                }
                state = START;
                // implicit fall-through for performance
            case START:
                if(strchr("+-*/<>&|^~", *str)) {
                    if((*str == '-') || (*str == '~') || (*str == '+')) {
                        state = UNARY;
                        break;
                    }
                    else {
                        error(message[ERROR_UNARYOPERATOR],0);
                        return 0;
                    }
                }
                else state = NUMBER;
                // implicit fall-through for performance
            case NUMBER:
                bufptr = buffer;
                switch(*str) {
                    case '\'':
                        tmplength = copyLiteralToken(str, buffer);
                        str += tmplength;
                        tmp = resolveNumber(buffer, tmplength, req_firstpass);
                        break;
                    case '[':
                        if(getBracketToken(&token, str) == 0) {
                            error(message[ERROR_BRACKETFORMAT],0);
                            return 0;
                        }
                        tmp = getExpressionValue(token.start, req_firstpass);
                        str = token.next;
                        break;
                    default:
                        while(!strchr("+-*/<>&|^~\t ", *str)) *bufptr++ = *str++;
                        *bufptr = 0; // terminate string in buffer
                        tmp = resolveNumber(buffer, bufptr - buffer, req_firstpass);
                        break;
                }
                
                if(unaryoperator) {
                    if(unaryoperator == '-') tmp = -tmp;
                    if(unaryoperator == '~') tmp = ~tmp;
                }

                switch(operator) {
                    case 0:
                    case '+': total += tmp; break;
                    case '-': total -= tmp; break;
                    case '*': total *= tmp; break;
                    case '<': total = total << tmp; break;
                    case '>': total = total >> tmp; break;
                    case '&': total = total & tmp;  break;
                    case '|': total = total | tmp;  break;
                    case '^': total = total ^ tmp;  break;
                    case '~': total = total + ~tmp; break;
                    case '/': total = total / tmp;  break;
                    default:
                        error(message[ERROR_OPERATOR],"%c",operator);
                        return 0;
                }
                // operation complete, reset operators 
                operator = 0;
                unaryoperator = 0;

                while(isspace(*str)) str++; // eat all spaces
                if(*str) state = OP;
                else return total;
                break;
        }
    }
    return total;
}

// efficient strcpy/strcat compound function
uint8_t strcompound(char *dest, const char *src1, const char *src2) {
    uint8_t len = 0;

    while(*src1) {
        *dest++ = *src1++;
        len++;
    }
    while(*src2) {
        *dest++ = *src2++;
        len++;
    }
    *dest = 0;
    return len;
}

char * _nextline_ptr;

uint16_t getnextline(char *dst) {
    uint16_t len = 0;

    while(*_nextline_ptr) {
        *dst++ = *_nextline_ptr;
        len++;
        if(*_nextline_ptr++ == '\n') {
            break;
        }
    }
    *dst = 0;
    return len;
}

void resetnextline(char *src) {
    _nextline_ptr = src;
}

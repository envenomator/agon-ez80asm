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

char _macro_VAL_buffer[MACROARGLENGTH + 1];// replacement buffer for values during macro expansion

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

void validateRange8bit(int32_t value) {
    if((value > 0xff) || (value < -128)) {
        error(message[ERROR_8BITRANGE]);
    }
}

void validateRange16bit(int32_t value) {
    if(value > 0xffff) {
        if(adlmode) {
            error(message[ERROR_16BITRANGE]);
        }
        else {
            error(message[ERROR_ADLWORDSIZE]);
        }
    }
    if(value < -32768) {
        error(message[ERROR_16BITRANGE]);
    }
}

void validateRange24bit(int32_t value) {
    if((value > 0xffffff) || (value < -8388608)) {
        error(message[ERROR_24BITRANGE]);
    }
}

// Returns the value of an escaped character \c, or 255 if illegal
uint8_t get_escaped_char(char c) {
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
        uint8_t c = get_escaped_char(string[2]);
        if(c == 0xff) {
            error(message[ERROR_ILLEGAL_ESCAPELITERAL]);
            return 0;
        }
        return c;
    }

    error(message[ERROR_ASCIIFORMAT]);
    return 0;
}

// Get the value from a sequence of 0-n labels and values, separated by +/- operators
// Examples:
// labela+5
// labelb-1
// labela+labelb+offset1-1
// The string should not contain any spaces, needs to be a single token
int32_t getValue(char *str, bool req_firstpass) {
    streamtoken_t token;
    label_t *lbl;
    int32_t total, tmp;
    uint8_t length, substlength;
    char prev_op = '+', unary = 0;
    bool expect = true;

    if((pass == 1) && !req_firstpass) return 0;

    total = 0;
    while(str) {
        length = getOperatorToken(&token, str);
        if(currentExpandedMacro) {
            substlength = macroExpandArg(_macro_VAL_buffer, token.start, currentExpandedMacro);
            if(substlength) {
                token.start = _macro_VAL_buffer;
                length = substlength;
            }
        }
        if(length == 0) { // at begin, or middle, OK. Expect catch at end
            expect = true;
            unary = token.terminator;
        }
        else { // normal processing
            lbl = findLabel(token.start);
            if(lbl) {
                tmp = lbl->address;
            }
            else {
                if(token.start[0] == '\'') tmp = getLiteralValue(token.start);
                else {
                    tmp = str2num(token.start, length);
                    if(err_str2num) {
                        if(pass == 1) {
                            // Yet unknown label, number incorrect
                            // We only get here if req_firstpass is true, so error
                            error(message[ERROR_INVALIDNUMBER]);
                            return 0;
                        }
                        else {
                            // Unknown label and number incorrect
                            error(message[ERROR_INVALIDLABELORNUMBER]);
                            return 0;
                        }
                    }
                }
            }
            if(unary) {
                switch(unary) {
                    case '-': tmp = -tmp; break;
                    case '~': tmp = ~tmp; break;
                    case '+': break;
                    default:
                        error(message[ERROR_UNARYOPERATOR]);
                        return 0;
                }
                unary = 0; // reset
                expect = false;
            }
            switch(prev_op) {
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
                case '!':
                default:
                    error(message[ERROR_OPERATOR]);
                    return total;
            }
            prev_op = token.terminator;
            expect = false;
        }
        str = token.next;
    }
    if(expect) {
        error(message[ERROR_MISSINGOPERAND]);
        return 0;
    }
    return total;
}

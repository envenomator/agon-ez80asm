#include <string.h>
#include "str2num.h"
#include "globals.h"
#include "utils.h"
#include "stdint.h"

bool err_str2num;

enum {
    BASESELECT,
    FIND_PREFIX,
    FIND_END,
    LAST_ITEM,
    DONE,
    ERROR
};

// transform a binary string to a uint32_t number
// string must end with 0 and contain only valid characters (0..1)
int32_t str2bin(char *string) {
    int32_t result = 0;
    uint8_t x = 0;

    while(*string) {
        if((*string == '0') || (*string == '1')) {x = *string - '0';}
        else err_str2num = true;
        result = (result << 1) | x;
        string++;
    }
    return result;
}

// transform a hex string to a uint32_t number
// string must end with 0 and contain only valid characters (0..9,a..f,A..F)
int32_t str2hex(char *string) {
    int32_t result = 0;
    char c;
    uint8_t x = 0;

    while(*string) {
        c = *string;
        if((c >= '0') && (c <= '9')) { x = c - '0'; }
        else {
            c = c & 0xDF ; // toupper();
            if((c >= 'A') && (c <= 'F')) { x = c - 'A' + 10; }
        else err_str2num = true;
        }
        result = (result << 4) | x;
        string++;
    }
    return result;
}

// transform a hex string to a uint32_t number
// string must end with 0 and contain only valid characters (0..9)
int32_t str2dec(char *string) {
    int32_t result = 0;
    uint8_t x = 0;

    while(*string) {
        if((*string >= '0') && (*string <= '9')) { x = *string - '0'; }
        else err_str2num = true;
        result = ((result << 1) + (result << 3)) + x;
        string++;
    }
    return result;
}

// Transforms a binary/hexadecimal/decimal string to an uint32_t number
// Valid strings are
// BINARY:  0%..., %..., ...b
// HEX:     0x..., ...h, $...
// DECIMAL ...
int32_t str2num(char *string, bool errorhalt) {
    char *ptr = string;
    char *start = string;
    int32_t result = 0;
    uint8_t state = BASESELECT;

    err_str2num = false;

    while(1) {
        switch(state) {
            case(DONE):
                return result;
                break;
            case(BASESELECT):
                switch(*ptr) {
                    case '$':
                    case '#':
                        if(strlen(ptr+1)) {
                            result = str2hex(ptr+1);
                            if(err_str2num && errorhalt) error(message[ERROR_INVALIDNUMBER]);
                            return result;
                        }
                        else {
                            state = ERROR;
                            errorhalt = true;
                        }
                        break;
                    case '%':
                        if(strlen(ptr+1)) {
                            result = str2bin(ptr+1);
                            if(err_str2num && errorhalt) error(message[ERROR_INVALIDBITNUMBER]);
                            return result;
                        }
                        else {
                            state = ERROR;
                            errorhalt = true;
                        }
                        break;
                    case '0':
                        state = FIND_PREFIX;
                        ptr++;
                        break;
                    case 0: // empty string
                        state = DONE;
                        break;
                    default:
                        if(isxdigit(*ptr)) state = FIND_END;
                        else state = ERROR;
                }
                break;
            case(FIND_PREFIX):
                switch(tolower(*ptr)) {
                    case 'x':
                        if(strlen(ptr+1)) {
                            result = str2hex(ptr+1);
                            if(err_str2num && errorhalt) error(message[ERROR_INVALIDNUMBER]);
                            return result;
                        }
                        else { // labels shouldn't begin with 0x, so halt always
                            errorhalt = true;
                            state = ERROR;
                        }
                        break;
                    case 'b': // also takes care of 0b, which is 0
                        result = str2bin(ptr+1);
                        if(err_str2num && errorhalt) error(message[ERROR_INVALIDNUMBER]);
                        return result;
                        break;
                    case 'h':
                    case 0:
                        return 0; // special case 0h / 0
                    default:
                        if(isxdigit(*ptr)) {
                            state = FIND_END;
                            start = ptr;
                        }
                        else state = ERROR;
                }
                break;
            case(FIND_END):
                switch(*ptr) {
                    case 0:
                        state = DONE; // empty string
                        break;
                    default:
                    if(ptr[1] == 0) state = LAST_ITEM;
                    else ptr++;
                }
                break;
            case(LAST_ITEM):
                switch(tolower(*ptr)) {
                    case 'b':
                        *ptr = 0; // terminate string
                        if(strlen(start)) {
                            result = str2bin(start);
                            if(err_str2num && errorhalt) error(message[ERROR_INVALIDNUMBER]);
                            return result;
                        }
                        else {  // just a 'b' given as part of an operand, not a register
                            state = ERROR;
                            errorhalt = true;
                        }
                        break;
                    case 'h':
                        *ptr = 0; // terminate string
                        if(strlen(start)) {
                            result = str2hex(start);
                            if(err_str2num && errorhalt) error(message[ERROR_INVALIDNUMBER]);
                            return result;
                        }
                        else {  // just a 'h' given as part of an operand, not a register
                            state = ERROR;
                            errorhalt = true;
                        }
                        break;
                    default:
                        if(isdigit(*ptr)) {
                            result = str2dec(start);
                            if(err_str2num && errorhalt) error(message[ERROR_INVALIDNUMBER]);
                            return result;
                        }
                        else state = ERROR;
                }
                break;
            case(ERROR):
                err_str2num = true;
                if(err_str2num && errorhalt) error(message[ERROR_INVALIDNUMBER]);
                state = DONE;
                break;
        }
    }
}
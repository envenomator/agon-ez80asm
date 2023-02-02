#include "str2num.h"
#include "globals.h"
#include "utils.h"

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
uint32_t str2bin(char *string) {
    uint32_t result = 0;
    uint8_t x;

    while(*string) {
        if((*string == '0') || (*string == '1')) {x = *string - '0';}
        else error(message[ERROR_INVALIDNUMBER]);
        result = (result << 1) | x;
        string++;
    }
    return result;
}

// transform a hex string to a uint32_t number
// string must end with 0 and contain only valid characters (0..9,a..f,A..F)
uint32_t str2hex(char *string) {
    uint32_t result = 0;
    char c;
    uint8_t x;

    while(*string) {
        c = *string;
        if((c >= '0') && (c <= '9')) { x = c - '0'; }
        else {
            c = c & 0xDF ; // toupper();
            if((c >= 'A') && (c <= 'F')) { x = c - 'A' + 10; }
        else error(message[ERROR_INVALIDNUMBER]);
        }
        result = (result << 4) | x;
        string++;
    }
    return result;
}

// transform a hex string to a uint32_t number
// string must end with 0 and contain only valid characters (0..9)
uint32_t str2dec(char *string) {
    uint32_t result = 0;
    uint8_t x;

    while(*string) {
        if((*string >= '0') && (*string <= '9')) { x = *string - '0'; }
        else error(message[ERROR_INVALIDNUMBER]);
        result = ((result << 1) + (result << 3)) + x;
        string++;
    }
    return result;
}

// Transforms a binary/hexadecimal/decimal string to an uint32_t number
// Valid strings are
// BINARY:  0b..., ...b
// HEX:     0x..., ...h, $...
// DECIMAL ...
uint32_t str2num(char *string) {
    char *ptr = string;
    char *start = string;
    uint32_t result = 0;
    uint8_t state = BASESELECT;

    while(1) {
        switch(state) {
            case(DONE):
                return result;
                break;
            case(BASESELECT):
                switch(*ptr) {
                    case '$':
                        return str2hex(ptr+1);
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
                switch(*ptr) {
                    case 'x':
                        return str2hex(ptr+1);
                        break;
                    case 'b':
                        //printf("DEBUG: %s\n",ptr+1);
                        return str2bin(ptr+1);
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
                switch(*ptr) {
                    case 'b':
                        *ptr = 0; // terminate string
                        return str2bin(start);
                        break;
                    case 'h':
                        *ptr = 0; // terminate string
                        return str2hex(start);
                        break;
                    default:
                        if(isdigit(*ptr)) return str2dec(start);
                        else state = ERROR;
                }
                break;
            case(ERROR):
                error(message[ERROR_INVALIDNUMBER]);
                state = DONE;
                break;
        }
    }
}
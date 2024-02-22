#include "str2num.h"
#include "globals.h"

bool err_str2num;

// transform a binary string to a uint24_t number
// string must end with 0 and contain only valid characters (0..1)
int24_t str2bin(char *string) {
    int24_t result = 0;
    uint8_t x = 0;

    while(*string) {
        if((*string == '0') || (*string == '1')) {x = *string - '0';}
        else err_str2num = true;
        result = (result << 1) | x;
        string++;
    }
    return result;
}

// transform a hex string to a int24_t number
// string must end with 0 and contain only valid characters (0..9,a..f,A..F)
int24_t str2hex(char *string) {
    int24_t result = 0;
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

// transform a hex string to a int24_t number
// string must end with 0 and contain only valid characters (0..9)
int24_t str2dec(char *string) {
    int24_t result = 0;
    uint8_t x = 0;

    while(*string) {
        if((*string >= '0') && (*string <= '9')) { x = *string - '0'; }
        else err_str2num = true;
        result = ((result << 1) + (result << 3)) + x;
        string++;
    }
    return result;
}

// Transforms a binary/hexadecimal/decimal string to an uint24_t number
// Valid strings are
// BINARY:  %..., , 0b..., ...b, capital letters allowed
// HEX:     0x..., ...h, $..., capital letters allowed
// DECIMAL ...
// Returns current program counter with just '$'
int24_t str2num(char *string, uint8_t length) {
    char buffer[TOKEN_MAX];
    char lastchar;
    int24_t result = 0;
    err_str2num = false;

    if(*string == '$') {
        if(*(string+1) == 0) return address;

        result = str2hex(string+1);
        if(err_str2num) error(message[ERROR_INVALIDNUMBER]);
        return result;
    }
    if(*string == '#') {
        result = str2hex(string+1);
        if(err_str2num) error(message[ERROR_INVALIDNUMBER]);
        return result;
    }
    if(*string == '%') {
        result = str2bin(string+1);
        if(err_str2num) error(message[ERROR_INVALIDNUMBER]);
        return result;
    }

    if(length == 1) {
        result = str2dec(string);
        if(err_str2num) error(message[ERROR_INVALIDNUMBER]);
        return result;
    }

    lastchar = tolower(string[length-1]);
    
    if(lastchar == 'h') {
        strcpy(buffer, string);
        buffer[length-1] = 0;
        result = str2hex(buffer);
        if(err_str2num) error(message[ERROR_INVALIDNUMBER]);
        return result;
    }

    if((*string == '0') && (length >= 2)) {
        if(tolower(*(string+1)) == 'x') {
            result = str2hex(string+2);
            if(err_str2num) error(message[ERROR_INVALIDNUMBER]);
            return result;
        }
        if(tolower(*(string+1)) == 'b') {
            result = str2bin(string+2);
            if(err_str2num) error(message[ERROR_INVALIDNUMBER]);
            return result;
        }
    }

    if(lastchar == 'b') {
        strcpy(buffer, string);
        buffer[length-1] = 0;
        result = str2bin(buffer);
        if(err_str2num) error(message[ERROR_INVALIDNUMBER]);
        return result;
    }

    result = str2dec(string);
    if(err_str2num) error(message[ERROR_INVALIDNUMBER]);
    return result;
}
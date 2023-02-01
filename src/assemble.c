#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <stdarg.h>
#include "assemble.h"
#include "globals.h"
#include "utils.h"
#include "label.h"

// parses the given string to the operand, or throws errors along the way
// will destruct parts of the original string during the process
void parse_operand(char *string, operand *operand) {
    char *ptr = string;

    // sensible defaults
    operand->displacement = 0;
    // direct or indirect
    if(*ptr == '(') {
        operand->indirect = true;
        // find closing bracket or error out
        while((*ptr) && (*ptr != ')')) ptr++;
        *ptr = 0; // terminate on closing bracket, or overwrite the existing 0
        ptr = &string[1];
    }
    else operand->indirect = false;

    switch(*ptr++) {
        case 0: // empty operand
            break;
        case 'a':
            switch(*ptr++) {
                case 0:
                    operand->type = OP_R;
                    operand->reg = R_A;
                    return;
                case 'f':
                    switch(*ptr++) {
                        case 0:
                        case '\'':
                            operand->type = OP_RR;
                            operand->reg = RR_AF;
                            return;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 'b':
            switch(*ptr++) {
                case 0:
                    operand->type = OP_R;
                    operand->reg = R_B;
                    return;
                case 'c':
                    if(*ptr == 0) {
                        operand->type = OP_RR;
                        operand->reg = RR_BC;
                        return;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 'c':
            switch(*ptr++) {
                case 0:
                    operand->type = OP_R;
                    operand->reg = R_C;
                    return;
                default:
                    break;
            }
            break;
        case 'd':
            switch(*ptr++) {
                case 0:
                    operand->type = OP_R;
                    operand->reg = R_D;
                    return;
                case 'e':
                    switch(*ptr++) {
                        case 0:
                            operand->type = OP_RR;
                            operand->reg = RR_DE;
                            return;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 'e':
            switch(*ptr++) {
                case 0:
                    operand->type = OP_R;
                    operand->reg = R_E;
                    return;
                default:
                    break;
            }
            break;
        case 'h':
            switch(*ptr++) {
                case 0:
                    operand->type = OP_R;
                    operand->reg = R_H;
                    return;
                case 'l':
                    if(*ptr == 0) {
                        operand->type = OP_RR;
                        operand->reg = RR_HL;
                        return;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 'i':
            switch(*ptr++) {
                case 0:
                    operand->type = OP_I;
                    operand->reg = R_I;
                    return;
                case 'x':
                    switch(*ptr++) {
                        case 0:
                            operand->type = OP_IX;
                            operand->reg = RR_IX;
                            return;
                        case 'h':
                            operand->type = OP_IXH;
                            operand->reg = RR_IXH;
                            return;
                        case 'l':
                            operand->type = OP_IXL;
                            operand->reg = RR_IXL;
                            return;
                        case '+':
                            if(isdigit(*ptr)) {
                                operand->displacement = (uint8_t) immediate(ptr);
                                return;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case 'y':
                    switch(*ptr++) {
                        case 0:
                            operand->type = OP_IY;
                            operand->reg = RR_IY;
                            return;
                        case 'h':
                            operand->type = OP_IYH;
                            operand->reg = RR_IYH;
                            return;
                        case 'l':
                            operand->type = OP_IYL;
                            operand->reg = RR_IYL;
                            return;
                        case '+':
                            if(isdigit(*ptr)) {
                                operand->displacement = (uint8_t) immediate(ptr);
                                return;
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 'l':
            switch(*ptr++) {
                case 0:
                    operand->type = OP_R;
                    operand->reg = R_L;
                    return;
                default:
                    break;
            }
            break;
        case 'm':
            if((*ptr == 'b') && ptr[1] == 0) {
                operand->type = OP_MB;
                operand->reg = RR_MB;
                return;
            }
            break;
        case 'r':
            if(*ptr == 0) {
                operand->type = OP_R;
                operand->reg = R_R;
                return;
            }
        case 's':
            if((*ptr == 'p') && ptr[1] == 0) {
                operand->type = OP_RR;
                operand->reg = RR_SP;
                return;
            }
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '$':
        case 'f':
            operand->immediate = immediate(ptr-1);
            return;
        default:
            break;
    }
    // check for hex string that ends with 'h'
    if(string[strlen(string)-1] == 'h') operand->immediate = immediate(string);
    else error("Invalid register");
}

void parse(char *line){
    char *s,*c;

    currentline.label[0] = 0;
    currentline.mnemonic[0] = 0;
    currentline.suffix_present = false;
    currentline.suffix[0] = 0;
    currentline.operand1[0] = 0;
    currentline.operand2[0] = 0;
    currentline.comment[0] = 0;
    currentline.size = 0;

    s = line;
    if((isspace(*s) == 0) && (*s != ';')) { // first char is not a space and not a ';'
        // label found at column 0
        c = currentline.label;
        while(*s){
            if(*s == ':') break;
            *c++ = *s++;
        }
        *c = 0; // terminate label
        s++;    // advance scanner beyond ':'
        if(*s == 0) {
            error("Invalid label definition");
            return;
        }
    }    
    while(isspace(*s) != 0) s++; // skip over whitespace
    if(isalpha(*s)){
        // potential mnemonic found
        c = currentline.mnemonic;
        while(*s && (isspace(*s) == 0) && (*s != '.') && (*s != ';')) {
            *c++ = tolower(*s++);
        }
        *c = 0; // terminate mnemonic string
        while(isspace(*s) != 0) s++; // skip over whitespace
        if(*s == '.') {
            // extension found
            s++;
            c = currentline.suffix;
            while(*s && (isspace(*s) == 0)) {
                *c++ = tolower(*s++);
            }
            currentline.suffix_present = true;
            *c = 0; // terminate extension string
            while(isspace(*s) != 0) s++; // skip over whitespace
        }
        if(*s != ';') {
            // potential first operand found
            c = currentline.operand1;
            while(*s && (*s != ',') && (*s != ';')){
                *c++ = tolower(*s++);
                while(isspace(*s) != 0) s++; // skip whitespace in argument
            }
            *c = 0; // terminate argument string
            parse_operand(currentline.operand1, &operand1);
            while(isspace(*s) != 0) s++; // skip over whitespace
            if(*s == ','){
                // potential second operand found
                s++;
                c = currentline.operand2;
                while(isspace(*s) != 0) s++; // skip over whitespace
                while(*s && (*s != ';')){
                    *c++ = tolower(*s++);
                    while(isspace(*s) != 0) s++; // skip whitespace in argument
                }
                *c = 0; // terminate second argument
                parse_operand(currentline.operand2, &operand2);
                if(currentline.operand2[0] == 0){
                    error("Missing 2nd argument");
                    return;
                }
            }
            while(isspace(*s) != 0) s++; // skip over whitespace
        }
    }
    if(*s == ';'){
        // comment found
        c = currentline.comment;
        s++;
        while(isspace(*s) != 0) s++; // skip over whitespace
        while(*s && (*s != '\n') && (*s != '\r')){
            *c++ = *s++;
        }
        *c = 0; // terminate string
    }
    return;
}

void definelabel(uint8_t size){
    // add label to label if defined
    if(strlen(currentline.label)) {
        if(label_table_insert(currentline.label, address) == false){
            error("Out of label space");
        }
    }
    address += size;
}

// return immediate 32-bit value
// when 8bit values are expected, cast appropriately by caller
// HEX: starts with $,0x, or ends with h
// BIN: starts with %,0%,0b, or ends with b
uint32_t immediate(char *arg)
{
    uint8_t base = 0;
    char *ptr,*chk;
    uint8_t len = strlen(arg);
    bool err;

    // if the string starts with '('
    if(arg[0] == '(') {
        arg++;
        len--;
    }    
    // if the string ends with ')'
    if(arg[len-1] == ')') {
        arg[len-1] = 0;
        len--;
    }
    // select correct base
    if(arg[len-1] == 'h') {
        arg[len-1] = 0; // only keep xdigit characters
        len--;
        base = 16;
        if(arg[0] == '$') ptr = arg+1; // allow for typos like $ffh
        else ptr = arg;
    }
    if((base == 0) && (arg[len-1]) == 'b') {
        arg[len-1] = 0; // only keep binary characters
        len--;
        base = 2;
        ptr = arg;
    }
    if((base == 0) && (arg[0] == '$')) {
        base = 16;
        ptr = &arg[1];
    }
    if((base == 0) && (strncmp("0x", arg, 2) == 0)) {
        base = 16;
        ptr = &arg[2];
    }
    if((base == 0) && ((strncmp("0b", arg, 2) == 0) || (strncmp("0\%", arg, 2)) == 0)) {
        base = 2;
        ptr = &arg[2];
    }
    if(base == 0) {
        base = 10;
        ptr = arg;
    }
    // verify character according to base
    chk = ptr;
    err = false;
    while(*chk) {
        if(base == 2) err = !((*chk == '0') || (*chk == '1'));
        if(base == 10) err = !isdigit(*chk);
        if(base == 16) err = !isxdigit(*chk);
        if(err) {
            error("Invalid number format");
            return 0;
        } 
        chk++;
    }
    return strtol(ptr, 0, base);
}

// return ADL prefix code, or 0 if none present
uint8_t getADLsuffix(adltype allowed) {
    uint8_t code=0;

    if(currentline.suffix_present) {
        switch(strlen(currentline.suffix)) {
            case 1: // .s or .l
                switch(currentline.suffix[0]) {
                    case 's':
                        if(adlmode) code=0x52;    // SIL
                        else code=0x40;           // SIS
                        break;
                    case 'l':
                        if(adlmode) code=0x5B;    // LIL
                        else code=0x49;           // LIS
                        break;
                    default: // illegal suffix
                        break;
                }
                break;
            case 2: // .is or .il
                if(currentline.suffix[0] != 'i') break; // illegal suffix
                switch(currentline.suffix[1]) {
                    case 's':
                        if(adlmode) code=0x49;    // LIS
                        else code=0x40;           // SIS
                        break;
                    case 'l':
                        if(adlmode) code=0x5B;    // LIL
                        else code=0x52;           // SIL
                        break;
                    default: // illegal suffix
                        break;
                }
                break;
            case 3:
                if(currentline.suffix[1] != 'i') break; // illegal suffix
                switch(currentline.suffix[0]) {
                    case 's':
                        if(currentline.suffix[2] == 's') code=0x40;
                        if(currentline.suffix[2] == 'l') code=0x52;
                        // illegal suffix
                        break;
                    case 'l':
                        if(currentline.suffix[2] == 's') code=0x49;
                        if(currentline.suffix[2] == 'l') code=0x5B;
                        // illegal suffix
                        break;
                    default: // illegal suffix
                        break;
                }
                break;
            default: // illegal suffix
                break;
        }
        if(code == 0) error("Illegal ADL suffix");
        // check for allowed suffixes
        switch(allowed) {
            case NONE:
                error("No suffix allowed");
                break;
            case L_ONLY:
                if((code == 0x40) || (code == 0x52)) error("Only .L allowed");
                break;
            case SL_ONLY:
                if((code == 0x40) || (code == 0x5B)) error("No SIS or LIL allowed");
                if((code == 0x49) && adlmode) error("L only allowed in z80 mode");
                if((code == 0x52) && !adlmode) error("S only allowed in ADL mode");
                break;
            case SISLIL:
                if((code == 0x49) || (code == 0x52)) error("No SIL or LIS allowed");
                if((code == 0x40) && (!adlmode)) error("SIS only allowed in ADL mode");
                if((code == 0x5B) && adlmode) error("LIL only allowed in z80 mode");
                break;
            case ANY:
                break;
        }
    }
    return code;
}

void emit_ld_from_immediate(uint8_t prefix, uint8_t opcode, char *valstring) {
    uint8_t suffix;
    uint8_t immsize;
    uint32_t tmp32 = immediate(valstring);

    suffix = getADLsuffix(ANY); // only takes care of illegal suffixes
    if(adlmode) {
        switch(suffix) {
            case 0: // According to mode
                immsize = 3;
                break;
            case 0x40: // SIS
                immsize = 2;
                break;
            default:
                immsize = 0;
                break;
        }
    }
    else {
        switch(suffix) {
            case 0: // According to mode
                immsize = 2;
                break;
            case 0x5B: // LIL
                immsize = 3;
                break;
            default:
                immsize = 0;
                break;
        }
    }
    if(immsize) {
        emit_instruction(immsize, suffix, prefix, opcode);
        if(immsize == 2) emit_16bit(tmp32);
        if(immsize == 3) emit_24bit(tmp32);
    }
    else error("Illegal ADL suffix");
}

void adl_action() {
    if(strcmp(currentline.operand1, "0") == 0) adlmode = false;
    if(strcmp(currentline.operand1, "1") == 0) adlmode = true;
    if(pass == 1) {
        if(debug_enabled) {
            if(adlmode) printf("ADLmode: 1\n");
            else printf("ADLmode: 0\n");
        }
    }
}

void emit_instruction(uint8_t immsize, uint8_t suffix, uint8_t prefix, uint8_t opcode) {
    uint8_t size;

    if(pass == 1) {
        size = immsize + (suffix>0)?1:0 + (prefix>0)?1:0 + 1;
        definelabel(size);
    }
    if(pass == 2) {
        if(suffix) printf("0x%02x-",suffix);
        if(prefix) printf("0x%02x-",prefix);
        printf("0x%02x-",opcode);
    }
}

void emit_8bit(uint8_t value) {
    if(pass == 2) printf("0x%02x\n",value);
}

void emit_16bit(uint16_t value) {
    if(pass == 2) {
        printf("0x%02x-0x%02x\n",value&0xFF, (value>>8)&0xFF);
    }
}

void emit_24bit(uint32_t value) {
    if(pass == 2) {
        printf("0x%02x-0x%02x-0x%02x\n", value&0xFF, (value>>8)&0xFF, (value>>16)&0xFF);
    }
}

void argcheck(bool passed)
{
    if (passed == false)
        error("arguments not correct for mnemonic");
}

void extcheck(bool passed)
{
    if (passed == false)
        error("extension not correct for mnemonic");
}


bool process(void){
    instruction *i;

    // return on empty lines
    if(strlen(currentline.mnemonic) == 0) {
        // check if there is a single label on a line in during pass 1
        if(pass == 1) definelabel(0);
        return true; // valid line, but empty
    }

    i = instruction_table_lookup(currentline.mnemonic);
    if(i == NULL) {
        error("Illegal opcode");
        return false;
    }

    if(i->type == ASSEMBLER)
    {
        adl_action();
    }
    // NEW PROCESS FUNCTION GOES HERE
    //i->function();
    return true;
}

void print_linelisting(void) {
    /*
    printf("Line %04d - ", linenumber);
    if(currentline.label[0]) printf("%s:",currentline.label);
    if(currentline.mnemonic[0]) printf("\t%s",currentline.mnemonic);
    if(currentline.suffix_present) printf(".%s",currentline.suffix);
    if(currentline.operand1[0]) printf("\t%s",currentline.operand1);
    if(currentline.operand2[0]) printf(", %s",currentline.operand2);
    if(currentline.comment[0]) printf("\t; %s",currentline.comment);
    printf("\n");
    */
    printf("Line       %04d\n", linenumber);
    printf("Operand1:\n");
    printf("Type:        %02x\n", operand1.type);
    printf("Register:    %02x\n", operand1.reg);
    printf("Indirect:    %02x\n", operand1.indirect);
    printf("d:           %02x\n", operand1.displacement);
    printf("Immediate: %04x\n", operand1.immediate);
    printf("Operand2:\n");
    printf("Type:        %02x\n", operand2.type);
    printf("Register:    %02x\n", operand2.reg);
    printf("Indirect:    %02x\n", operand2.indirect);
    printf("d:           %02x\n", operand2.displacement);
    printf("Immediate: %04x\n", operand2.immediate);

    printf("\n");
}


void parsed_listing(void) {
    printf("Line       %04d - ", linenumber);
    printf("Operand1:\n");
    printf("Type:        %02x\n", operand1.type);
    printf("Register:    %02x\n", operand1.reg);
    printf("Indirect:    %02x\n", operand1.indirect);
    printf("d:           %02x\n", operand1.displacement);
    printf("Immediate: %04x\n", operand1.immediate);
    printf("Operand2:\n");
    printf("Type:        %02x\n", operand2.type);
    printf("Register:    %02x\n", operand2.reg);
    printf("Indirect:    %02x\n", operand2.indirect);
    printf("d:           %02x\n", operand2.displacement);
    printf("Immediate: %04x\n", operand2.immediate);

}
bool assemble(FILE *infile, FILE *outfile){
    char line[LINEMAX];

    adlmode = true;
    global_errors = 0;

    // Assemble in two passes
    printf("Pass 1...\n");
    pass = 1;
    linenumber = 1;
    address = START_ADDRESS;
    while (fgets(line, sizeof(line), infile)){
        parse(line);

        if(listing_enabled) print_linelisting();

        process();
        linenumber++;
    }
    if(debug_enabled) print_label_table();
    printf("%d lines\n", linenumber);
    printf("%d labels\n", label_table_count());
    //print_bufferspace();

    if(global_errors) return false;

    printf("Pass 2...\n");
    rewind(infile);
    pass = 2;
    linenumber = 1;
    address = START_ADDRESS;
    while (fgets(line, sizeof(line), infile)){
        parse(line);
        process();
        linenumber++;
    }
    return true;
}


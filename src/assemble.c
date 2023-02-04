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
#include "str2num.h"

// parses the given string to the operand, or throws errors along the way
// will destruct parts of the original string during the process
void parse_operand(char *string, operand *operand) {
    char *ptr = string;
    uint8_t len = strlen(string);

    // defaults
    operand->reg = R_NONE;
    operand->reg_index = 0;
    operand->cc = false;
    operand->cc_index = 0;
    operand->displacement = 0;
    operand->immediate = 0;
    operand->immediate_provided = false;
    // direct or indirect
    if(*ptr == '(') {
        operand->indirect = true;
        // find closing bracket or error out
        if(string[len-1] == ')') string[len-1] = 0; // terminate on closing bracket
        else error(message[ERROR_CLOSINGBRACKET]);
        ptr = &string[1];
    }
    else {
        operand->indirect = false;
        // should not find a closing bracket
        if(string[len-1] == ')') error(message[ERROR_OPENINGBRACKET]);
    }

    switch(*ptr++) {
        case 0: // empty operand
            break;
        case 'a':
            switch(*ptr++) {
                case 0:
                    operand->reg = R_A;
                    operand->reg_index = R_INDEX_A;
                    return;
                case 'f':
                    switch(*ptr++) {
                        case 0:
                        case '\'':
                            operand->reg = R_AF;
                            operand->reg_index = R_INDEX_AF;
                            if(operand->indirect) error(message[ERROR_INVALIDREGISTER]);
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
                    operand->reg = R_B;
                    operand->reg_index = R_INDEX_B;
                    return;
                case 'c':
                    if(*ptr == 0) {
                        operand->reg = R_BC;
                        operand->reg_index = R_INDEX_BC;
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
                    operand->reg = R_C;
                    operand->reg_index = R_INDEX_C;
                    operand->cc = true;
                    operand->cc_index = CC_INDEX_C;
                    return;
                default:
                    break;
            }
            break;
        case 'd':
            switch(*ptr++) {
                case 0:
                    operand->reg = R_D;
                    operand->reg_index = R_INDEX_D;
                    return;
                case 'e':
                    switch(*ptr++) {
                        case 0:
                            operand->reg = R_DE;
                            operand->reg_index = R_INDEX_DE;
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
                    operand->reg = R_E;
                    operand->reg_index = R_INDEX_E;
                    return;
                default:
                    break;
            }
            break;
        case 'h':
            switch(*ptr++) {
                case 0:
                    operand->reg = R_H;
                    operand->reg_index = R_INDEX_H;
                    return;
                case 'l':
                    if(*ptr == 0) {
                        operand->reg = R_HL;
                        operand->reg_index = R_INDEX_HL;
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
                    operand->reg = R_I;
                    operand->reg_index = R_INDEX_I;
                    return;
                case 'x':
                    switch(*ptr++) {
                        case 0:
                            if(operand->indirect) error(message[ERROR_INVALIDREGISTER]);
                            operand->reg = R_IX;
                            return;
                        case 'h':
                            operand->reg = R_IXH;
                            return;
                        case 'l':
                            operand->reg = R_IXL;
                            return;
                        case '+':
                            if(isdigit(*ptr)) {
                                operand->reg = R_IX;
                                operand->displacement = (uint8_t) str2num(ptr);
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
                            if(operand->indirect) error(message[ERROR_INVALIDREGISTER]);
                            operand->reg = R_IY;
                            return;
                        case 'h':
                            operand->reg = R_IYH;
                            return;
                        case 'l':
                            operand->reg = R_IYL;
                            return;
                        case '+':
                            if(isdigit(*ptr)) {
                                operand->reg = R_IY;
                                operand->displacement = (uint8_t) str2num(ptr);
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
                    operand->reg = R_L;
                    operand->reg_index = R_INDEX_L;
                    return;
                default:
                    break;
            }
            break;
        case 'm':
            if((*ptr == 'b') && ptr[1] == 0) {
                operand->reg = R_MB;
                operand->reg_index = R_INDEX_MB;
                return;
            }
            if((*ptr == 0)) {
                operand->cc = true;
                operand->cc_index = CC_INDEX_M;
                return;
            }
            break;
        case 'n':
            switch(*ptr++) {
                case 'c':   // NC
                    operand->cc = true;
                    operand->cc_index = CC_INDEX_NC;
                    return;
                case 'z':   // NZ
                    operand->cc = true;
                    operand->cc_index = CC_INDEX_NZ;
                    return;
                default:
                    break;
            }
            break;
        case 'p':
            switch(*ptr++) {
                case '0':
                    operand->cc = true;
                    operand->cc_index = CC_INDEX_P;
                    return;
                case 'e':
                    operand->cc = true;
                    operand->cc_index = CC_INDEX_PE;
                    return;
                case 'o':
                    operand->cc = true;
                    operand->cc_index = CC_INDEX_PO;
                    return;
                default:
                    break;
            }
            break;
        case 'r':
            if(*ptr == 0) {
                operand->reg = R_R;
                operand->reg_index = R_INDEX_R;
                return;
            }
            break;
        case 's':
            if((*ptr == 'p') && ptr[1] == 0) {
                operand->reg = R_SP;
                operand->reg_index = R_INDEX_SP;
                return;
            }
            break;
        case 'z':
            if(*ptr == 0) {
                operand->cc = true;
                operand->cc_index = CC_INDEX_Z;
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
            operand->immediate = str2num(ptr-1);
            operand->immediate_provided = true;
            return;
        default:
            break;
    }
    if(*string) { // not on empty lines
        // check for hex string that ends with 'h'
        if(string[strlen(string)-1] == 'h') {
            operand->immediate = str2num(string);
            operand->immediate_provided = true;
        }
        else error(message[ERROR_INVALIDREGISTER]);
    }
}

enum {
    STATE_LINESTART,
    STATE_MNEMONIC,
    STATE_SUFFIX,
    STATE_OPERAND1,
    STATE_OPERAND2,
    STATE_COMMENT,
    STATE_DONE,
    STATE_MISSINGOPERAND
};

// converts everything to lower case, except comments
void convertLower(char *line) {
    char *ptr = line;
    while((*ptr) && (*ptr != ';')) {
        *ptr = tolower(*ptr);
        ptr++;
    }
}

void parse(char *line) {
    uint8_t state;
    char *ptr;
    char *token;
    char *tstart;

    currentline.label[0] = 0;
    currentline.mnemonic[0] = 0;
    currentline.suffix_present = false;
    currentline.suffix[0] = 0;
    currentline.operand1[0] = 0;
    currentline.operand2[0] = 0;
    currentline.comment[0] = 0;
    currentline.size = 0;

    state = STATE_LINESTART;

    while(1) {
        switch(state) {
            case STATE_LINESTART:
                ptr = line;
                while(isspace(*ptr) != 0) ptr++; // skip over whitespace
                tstart = ptr;
                if(isalnum(*ptr)) {
                    state = STATE_MNEMONIC;
                    token = currentline.mnemonic;
                    break;
                }
                if(*ptr == ';') {
                    token = currentline.comment;
                    state = STATE_COMMENT;
                    ptr++;
                    break;
                }
                state = STATE_DONE; // empty line
                break;
            case STATE_MNEMONIC:
                switch(*ptr) {
                    case 0:
                        *token = 0;
                        state = STATE_DONE;
                        break;
                    case ';':
                        *token = 0;
                        token = currentline.comment;
                        state = STATE_COMMENT;
                        ptr++;
                        break;
                    case ':':
                        // token proves to be a label after all
                        *token = 0; // close string
                        *ptr = 0;
                        strcpy(currentline.label, tstart);
                        currentline.mnemonic[0] = 0; // empty mnemonic again
                        ptr++;
                        while(isspace(*ptr) != 0) ptr++; // skip over whitespace
                        token = currentline.mnemonic;
                        // no change in state - MNEMONIC expected
                        break;
                    case '.':
                        *token = 0; // terminate token string
                        state = STATE_SUFFIX;
                        ptr++;
                        while(isspace(*ptr) != 0) ptr++; // skip over whitespace
                        token = currentline.suffix;
                        break;
                    case ',':
                        state = STATE_MISSINGOPERAND;
                        break;
                    default:
                        if(isspace(*ptr) == 0) *token++ = *ptr++;
                        else {
                            // close out token
                            *token = 0;
                            ptr++;
                            while(isspace(*ptr) != 0) ptr++; // skip over whitespace
                            token = currentline.operand1;
                            state = STATE_OPERAND1;
                        }
                }
                break;
            case STATE_SUFFIX:
                if(*ptr == ',') {
                    state = STATE_MISSINGOPERAND;
                    break;
                }
                if(isspace(*ptr) == 0) {
                    *token++ = *ptr++;
                }
                else {
                    // close out token
                    *token = 0;
                    ptr++;
                    while(isspace(*ptr) != 0) ptr++; // skip over whitespace
                    token = currentline.operand1;
                    state = STATE_OPERAND1;
                    currentline.suffix_present = true;
                }
                break;
            case STATE_OPERAND1:
                switch(*ptr) {
                    case 0:
                        *token = 0;
                        state = STATE_DONE;
                        break;
                    case '.':
                    case ':':
                        state = ERROR_INVALIDOPERAND;
                        break;
                    case ',':
                        *token = 0;
                        ptr++;
                        while(isspace(*ptr) != 0) ptr++; // skip over whitespace
                        token = currentline.operand2;
                        state = STATE_OPERAND2;
                        break;
                    case ';':
                        *token = 0;
                        ptr++;
                        token = currentline.comment;
                        state = STATE_COMMENT;
                        break;
                    default:
                        *token++ = *ptr++;
                        while(isspace(*ptr) != 0) ptr++; // skip over whitespace
                }
                break;
            case STATE_OPERAND2:
                switch(*ptr) {
                    case 0:
                        *token = 0;
                        state = STATE_DONE;
                        break;
                    case '.':
                    case ':':
                    case ',':
                        state = ERROR_INVALIDOPERAND;
                        break;
                    case ';':
                        *token = 0;
                        ptr++;
                        token = currentline.comment;
                        state = STATE_COMMENT;
                        break;
                    default:
                        *token++ = *ptr++;
                        while(isspace(*ptr) != 0) ptr++; // skip over whitespace
                }
                break;
            case STATE_COMMENT:
                if((*ptr != 0) && (*ptr != '\r') && (*ptr != '\n')) *token++ = *ptr++;
                else {
                    *token = 0;
                    state = STATE_DONE;
                }
                break;
            case STATE_DONE:
                parse_operand(currentline.operand1, &operand1);
                parse_operand(currentline.operand2, &operand2);
                return;
            case STATE_MISSINGOPERAND:
                error(message[ERROR_MISSINGOPERAND]);
                state = STATE_DONE;
                break;
        }
    }
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
                        if(currentline.suffix[2] == 's') code=0x40; // SIS
                        if(currentline.suffix[2] == 'l') code=0x52; // SIL
                        // illegal suffix
                        break;
                    case 'l':
                        if(currentline.suffix[2] == 's') code=0x49; // LIS
                        if(currentline.suffix[2] == 'l') code=0x5B; // LIL
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

/*
void emit_ld_from_immediate(uint8_t prefix, uint8_t opcode, char *valstring) {
    uint8_t suffix;
    uint8_t immsize;
    uint32_t tmp32 = str2num(valstring);

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
*/

void adl_action() {
    if(strcmp(currentline.operand1, "0") == 0) adlmode = false;
    if(strcmp(currentline.operand1, "1") == 0) adlmode = true;
    if(pass == 1) {
        if(debug_enabled) {
            if(adlmode) printf("DEBUG - Line %d - ADLmode: 1\n", linenumber);
            else printf("DEBUG - Line %d - ADLmode: 0\n", linenumber);
        }
    }
}

/*
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
*/

void emit_instruction(operandlist *list) {
    uint8_t size = 1; // There is always 1 opcode to output
    uint8_t suffix = getADLsuffix(list->adl);

    // Transform necessary prefix/opcode in output, according to given list and operands
    output.prefix1 = list->prefix1;
    output.prefix2 = list->prefix2;
    output.opcode = list->opcode;
    if(list->transformA != TRANSFORM_NONE) operandtype_matchlist[list->operandA].transform(list->transformA, &operand1);
    if(list->transformB != TRANSFORM_NONE) operandtype_matchlist[list->operandB].transform(list->transformB, &operand2);

    size += output.prefix1?1:0 + output.prefix2?1:0 + suffix?1:0;
    if((list->operandA == OPTYPE_INDIRECT_IXYd) || (list->operandB == OPTYPE_INDIRECT_IXYd)) size++; // Displacement byte
    if((list->operandA == OPTYPE_N) || (list->operandB == OPTYPE_N)) size++; // n == 1byte
    if((list->operandA == OPTYPE_MMN) || (list->operandB == OPTYPE_MMN) || (list->operandA == OPTYPE_INDIRECT_MMN) || (list->operandB == OPTYPE_INDIRECT_MMN)) {
        // add 2 or 3 bytes, according to adl mode and suffix
    }

    if(pass == 1) {
        if(debug_enabled) printf("DEBUG - Line %d - instruction size %d\n", linenumber, size);
        if((list->operandA == OPTYPE_N) && (operand1.immediate > 0xFF)) error(message[WARNING_N_TOOLARGE]);
        if((list->operandB == OPTYPE_N) && (operand2.immediate > 0xFF)) error(message[WARNING_N_TOOLARGE]);
        printf("DEBUG: ADL code %d\n", suffix);
        printf("DEBUG: ADL string %s\n",currentline.suffix);
        printf("DEBUG: ADL suffix present: %d\n", currentline.suffix_present);
        definelabel(size);
    }
    if(pass == 2) {
        if(suffix) printf("0x%02x:",suffix);
        if(output.prefix1) printf("0x%02x:",output.prefix1);
        if(output.prefix2) printf("0x%02x:",output.prefix2);
        printf("0x%02x",output.opcode);
        if(list->operandA == OPTYPE_N) printf(":0x%02x", operand1.immediate & 0xFF);
        if(list->operandB == OPTYPE_N) printf(":0x%02x", operand2.immediate & 0xFF);
        if(list->operandA == OPTYPE_INDIRECT_IXYd) printf(":0x%02x", operand1.displacement & 0xFF);
        if(list->operandB == OPTYPE_INDIRECT_IXYd) printf(":0x%02x", operand2.displacement & 0xFF);

        printf("\n");
    //    if(suffix) printf("0x%02x-",suffix);
    //    if(prefix) printf("0x%02x-",prefix);
    //    printf("0x%02x-",opcode);
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


void process(void){
    instruction *current_instruction;
    operandlist *list;
    uint8_t listitem;
    bool match;

    // return on empty lines
    if((currentline.mnemonic[0]) == 0) {
        // check if there is a single label on a line in during pass 1
        if(pass == 1) definelabel(0);
        return; // valid line, but empty
    }

    current_instruction = instruction_table_lookup(currentline.mnemonic);
    if(current_instruction == NULL) {
        error(message[ERROR_INVALIDMNEMONIC]);
        return;
    }
    if(current_instruction->type == EZ80) {
        // process this mnemonic by applying the instruction list as a filter to the operand-set
        list = current_instruction->list;
        if(debug_enabled && pass == 1) {
            printf("DEBUG - Line %d - Mmemonic \'%s\'\n", linenumber, currentline.mnemonic);
            printf("DEBUG - Line %d - regA %02x regB %02x\n", linenumber, operand1.reg, operand2.reg);
            printf("DEBUG - Line %d - indirectA %02x\n", linenumber, operand1.indirect);
            printf("DEBUG - Line %d - indirectB %02x\n", linenumber, operand2.indirect);
        }
        match = false;
        for(listitem = 0; listitem < current_instruction->listnumber; listitem++) {
            if(debug_enabled && pass == 1) printf("DEBUG - Line %d - %02x %02x %02x %02x %02x %02x %02x %02x\n", linenumber, list->operandA, list->operandB, list->transformA, list->transformB, list->prefix1, list->prefix2, list->opcode, list->adl);
            if(operandtype_matchlist[list->operandA].match(&operand1) && operandtype_matchlist[list->operandB].match(&operand2)) {
                match = true;
                if((debug_enabled) && pass == 1) printf("DEBUG - Line %d - match found on ^last^ filter list tuple\n", linenumber);
                //printf("Line %d - match found\n",linenumber);
                emit_instruction(list);
                break;
            }
            list++;
        }
        if(!match) error(message[ERROR_OPERANDSNOTMATCHING]);
        return;
    }
    if(current_instruction->type == ASSEMBLER)
    {
        adl_action();
        return;
    }
    return;
}

void print_linelisting_old(void) {

    printf("Line %04d - \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\"", linenumber, currentline.label, currentline.mnemonic, currentline.suffix, currentline.operand1, currentline.operand2, currentline.comment);
    printf("\n");

    printf("Line %04d - %10s %4s - %d %d %d %d - %d %d %d %d\n",linenumber, currentline.mnemonic, currentline.suffix, operand1.reg, operand1.indirect, operand1.displacement, operand1.immediate, operand2.reg, operand2.indirect, operand2.displacement, operand2.immediate);
    /*
    printf("Line       %04d\n", linenumber);
    printf("Operand1:\n");
    printf("Register:    %02x\n", operand1.reg);
    printf("Indirect:    %02x\n", operand1.indirect);
    printf("d:           %02x\n", operand1.displacement);
    printf("Immediate: %04x\n", operand1.immediate);
    printf("Operand2:\n");
    printf("Register:    %02x\n", operand2.reg);
    printf("Indirect:    %02x\n", operand2.indirect);
    printf("d:           %02x\n", operand2.displacement);
    printf("Immediate: %04x\n", operand2.immediate);
    */
}


void print_linelisting(void) {
    printf("Line       %04d - ", linenumber);
    printf("Operand1:\n");
    printf("Register:    %02x\n", operand1.reg);
    printf("Indirect:    %02x\n", operand1.indirect);
    printf("d:           %02x\n", operand1.displacement);
    printf("Immediate: %04x\n", operand1.immediate);
    printf("Operand2:\n");
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
    // Pass 1
    printf("Pass 1...\n");
    pass = 1;
    linenumber = 1;
    address = START_ADDRESS;
    while (fgets(line, sizeof(line), infile)){
        convertLower(line);
        parse(line);
        process();
        if(listing_enabled) print_linelisting();
        linenumber++;
    }
    if(debug_enabled) print_label_table();
    printf("%d lines\n", linenumber);
    printf("%d labels\n", label_table_count());
    if(global_errors) return false;

    // Pass 2
    printf("Pass 2...\n");
    rewind(infile);
    pass = 2;
    linenumber = 1;
    address = START_ADDRESS;
    while (fgets(line, sizeof(line), infile)){
        convertLower(line);
        parse(line);
        process();
        linenumber++;
    }
    return true;
}


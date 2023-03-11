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
#include "listing.h"
#include "filestack.h"
#include "./stdint.h"
#include "mos-interface.h"
#include "macro.h"

void empty_operand(operand *op) {
    op->reg = R_NONE;
    op->reg_index = 0;
    op->cc = false;
    op->cc_index = 0;
    op->displacement = 0;
    op->displacement_provided = false;
    op->immediate = 0;
    op->immediate_provided = false;
    op->wasLabel = false;
}

void advanceLocalLabel(void) {
    if(currentline.label[0] == '@') {
        if(currentline.label[1] == '@') {
            readAnonymousLabel();
        }
    }
}

// Get the ascii value from a single 'x' token.
uint8_t getAsciiValue(char *string) {
    uint8_t len = strlen(string);

    if((len == 3) || (len == 4)) {
        if(*string == '\'') {
            if((len == 4) && (string[3] == '\'') && (string[1] == '\\')) {
                switch(string[2]) {
                    case 'n': return '\n';
                    case 'r': return '\r';
                    case 't': return '\t';
                    case 'b': return '\b';
                    case '\\': return '\\';
                    case '\"': return '\"';
                    case '\'': return '\'';
                }
            }
            if((len == 3) && (string[2] == '\'')) {
                return string[1];
            }
        }
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
int24_t getLabelValue(char *string) {
    int24_t total, tmp;
    char operator, *ptr;
    label *lbl;
    tokentype token;

    ptr = string;
    total = 0;
    operator = '+'; // previous operand in case of single value/label
    while(ptr) {
        tmp = 0;
        getOperatorToken(&token, ptr);
        if(notEmpty(token.start)) {
            lbl = findLabel(token.start);
            if(lbl) tmp = lbl->address;
            else {
                if(token.start[0] == '\'') tmp = getAsciiValue(token.start);
                else {
                    tmp = str2num(token.start, false);
                    if(err_str2num && (pass == 2)) error(message[ERROR_INVALIDLABEL]);
                }
            }
        }
        if(operator == '!') error(message[ERROR_OPERATOR]);
        if(operator == '+') total += tmp;
        if(operator == '-') total -= tmp;
        if(operator == '<') total = total << tmp;
        if(operator == '>') total = total >> tmp;
        operator = token.terminator;

        if(operator) ptr = token.next;
        else ptr = NULL;
    }
    return total;
}

// parses the given string to the operand, or throws errors along the way
// will destruct parts of the original string during the process
void parse_operand(char *string, operand *operand) {
    char *ptr = string;
    uint8_t len = strlen(string);

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

    switch(tolower(*ptr++)) {
        case 0: // empty operand
            break;
        case 'a':
            switch(tolower(*ptr++)) {
                case 0:
                    operand->reg = R_A;
                    operand->reg_index = R_INDEX_A;
                    return;
                case 'f':
                    switch(tolower(*ptr++)) {
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
            switch(tolower(*ptr++)) {
                case 0:
                    operand->reg = R_B;
                    operand->reg_index = R_INDEX_B;
                    return;
                case 'c':
                    if(tolower(*ptr == 0)) {
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
            switch(tolower(*ptr++)) {
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
            switch(tolower(*ptr++)) {
                case 0:
                    operand->reg = R_D;
                    operand->reg_index = R_INDEX_D;
                    return;
                case 'e':
                    switch(tolower(*ptr++)) {
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
            switch(tolower(*ptr++)) {
                case 0:
                    operand->reg = R_E;
                    operand->reg_index = R_INDEX_E;
                    return;
                default:
                    break;
            }
            break;
        case 'h':
            switch(tolower(*ptr++)) {
                case 0:
                    operand->reg = R_H;
                    operand->reg_index = R_INDEX_H;
                    return;
                case 'l':
                    if(tolower(*ptr == 0)) {
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
            switch(tolower(*ptr++)) {
                case 0:
                    operand->reg = R_I;
                    operand->reg_index = R_INDEX_I;
                    return;
                case 'x':
                    switch(tolower(*ptr++)) {
                        case 0:
                            operand->reg = R_IX;
                            operand->reg_index = R_INDEX_IX;
                            return;
                        case 'h':
                            operand->reg = R_IXH;
                            return;
                        case 'l':
                            operand->reg = R_IXL;
                            return;
                        case '+':
                        case '-':
                            //if(isdigit(*ptr)) {
                                operand->reg = R_IX;
                                operand->displacement_provided = true;
                                //if(*(ptr-1) == '-') operand->displacement = -1 * (int16_t) str2num(ptr,true);
                                //else operand->displacement = (int16_t) str2num(ptr,true);
                                if(*(ptr-1) == '-') operand->displacement = -1 * (int16_t) getLabelValue(ptr);
                                else operand->displacement = (int16_t) getLabelValue(ptr);
                                return;
                            //}
                            break;
                        default:
                            break;
                    }
                    break;
                case 'y':
                    switch(tolower(*ptr++)) {
                        case 0:
                            operand->reg = R_IY;
                            operand->reg_index = R_INDEX_IY;
                            return;
                        case 'h':
                            operand->reg = R_IYH;
                            return;
                        case 'l':
                            operand->reg = R_IYL;
                            return;
                        case '+':
                        case '-':
                            //if(isdigit(*ptr)) {
                                operand->reg = R_IY;
                                operand->displacement_provided = true;
                                //if(*(ptr-1) == '-') operand->displacement = -1 * (int16_t) str2num(ptr,true);
                                //else operand->displacement = (int16_t) str2num(ptr,true);
                                if(*(ptr-1) == '-') operand->displacement = -1 * (int16_t) getLabelValue(ptr);
                                else operand->displacement = (int16_t) getLabelValue(ptr);
                                return;
                            //}
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
            switch(tolower(*ptr++)) {
                case 0:
                    operand->reg = R_L;
                    operand->reg_index = R_INDEX_L;
                    return;
                default:
                    break;
            }
            break;
        case 'm':
            if((tolower(*ptr) == 'b') && ptr[1] == 0) {
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
            switch(tolower(*ptr++)) {
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
            switch(tolower(*ptr++)) {
                case 0:
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
            if((tolower(*ptr) == 'p') && ptr[1] == 0) {
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
        default:
            break;
    }
    if(*string) {
        if(operand->indirect) operand->immediate = getLabelValue(string + 1);
        else operand->immediate = getLabelValue(string);
        operand->immediate_provided = true;
        operand->wasLabel = true;
    }
}

void parseLine(char *src) {
    uint8_t x;
    bool done;
    uint8_t state;
    uint8_t argcount = 0;
    tokentype token;

    // default current line items
    currentline.current_instruction = NULL;
    currentline.next = NULL;
    currentline.label[0] = 0;
    currentline.mnemonic[0] = 0;
    currentline.suffix[0] = 0;
    currentline.operand1[0] = 0;
    currentline.operand2[0] = 0;
    currentline.comment[0] = 0;
    currentline.size = 0;

    empty_operand(&operand1);
    empty_operand(&operand2);

    state = PS_START;
    done = false;
    while(!done) {
        switch(state) {
            case PS_START:
                if((isspace(*src) == 0) && (*src) != '.') {
                    getLineToken(&token, src, ':');
                    switch(token.terminator) {
                        case ':':
                            state = PS_LABEL;
                            break;
                        case ';':
                            state = PS_COMMENT;
                            break;
                        default:
                            error(message[ERROR_INVALIDLABEL]);
                            state = PS_ERROR;                        
                            break;
                    }
                    break;
                }
                x = getLineToken(&token, src,' ');
                if(x) state = PS_COMMAND;
                else {
                    if(token.terminator == 0) {
                        state = PS_DONE;
                        break;
                    }
                    if(token.terminator == ';') {
                        state = PS_COMMENT;
                        break;
                    }
                }
                break;
            case PS_LABEL:
                strcpy(currentline.label,token.start);
                advanceLocalLabel();
                x = getLineToken(&token, token.next, ' ');
                if(x) state = PS_COMMAND;
                else {
                    if(token.terminator == 0) {
                        state = PS_DONE;
                        break;
                    }
                    if(token.terminator == ';') {
                        state = PS_COMMENT;
                        break;
                    }
                }
                break;
            case PS_COMMAND:
                if(token.start[0] == '.') strcpy(currentline.mnemonic, token.start+1);
                else split_suffix(currentline.mnemonic, currentline.suffix, token.start);

                currentline.current_instruction = instruction_table_lookup(currentline.mnemonic);
                if(currentline.current_instruction == NULL) {
                    error(message[ERROR_INVALIDMNEMONIC]);
                    state = PS_ERROR;
                    break;
                }
                if(currentline.current_instruction->type == ASSEMBLER) {
                    currentline.next = token.next;
                    state = PS_DONE;
                    break;
                }
                if(token.start[0] == '.') {
                    error(message[ERROR_INVALIDMNEMONIC]);
                    currentline.mnemonic[0] = 0;
                    state = PS_ERROR;                    
                    break;
                }
                // Valid EZ80 instruction
                if(!inmacro) {
                    switch(token.terminator) {
                        case ';':
                            getLineToken(&token, token.next, 0);
                            state = PS_COMMENT;
                            break;
                        case 0:
                            currentline.next = NULL;
                            state = PS_DONE;
                            break;
                        default:
                            getLineToken(&token, token.next, ',');
                            state = PS_OP1;
                            break;
                    }
                } 
                else state = PS_DONE;
                break;
            case PS_OP1:
                argcount++;
                if(argcount == 1) {
                    strcpy(currentline.operand1, token.start);
                    parse_operand(currentline.operand1, &operand1);
                }
                else {
                    strcpy(currentline.operand2, token.start);
                    parse_operand(currentline.operand2, &operand2);
                }
                switch(token.terminator) {
                    case ';':
                        getLineToken(&token, token.next, 0);
                        state = PS_COMMENT;
                        break;
                    case 0:
                        currentline.next = NULL;
                        state = PS_DONE;
                        break;
                    case ',':
                        if(argcount == 2) {
                            error(message[ERROR_TOOMANYARGUMENTS]);
                            state = PS_ERROR;
                            break;
                        }
                        getLineToken(&token, token.next, ',');
                        break;
                }
                break;
            case PS_COMMENT:
                strcpy(currentline.comment,token.start);
                state = PS_DONE;
                break;
            case PS_ERROR:
                currentline.next = NULL;
                state = PS_DONE;
                break;
            case PS_DONE:
                done = true;
                break;
        }
    }
}

void definelabel(int24_t num){
    if(strlen(currentline.label)) {
        if(currentline.label[0] == '@') {
            if(currentline.label[1] == '@') {
                writeAnonymousLabel(num);
                return;
            }
            if(insertLocalLabel(currentline.label, num) == false) {
                error(message[ERROR_CREATINGLABEL]);
                return;
            }
        }
        else {
            if(insertGlobalLabel(currentline.label, num) == false){
                error(message[ERROR_CREATINGLABEL]);
                return;
            }
            writeLocalLabels();
            clearLocalLabels();
        }
    }
}

void refreshlocalLabels(void) {
    if(pass == 2) {
        if(notEmpty(currentline.label)) {
            if(currentline.label[0] != '@') {
                clearLocalLabels();
                readLocalLabels();
            }
        }
    }
}

// return ADL prefix bitfield, or 0 if none present
uint8_t getADLsuffix(void) {
    if(notEmpty(currentline.suffix)) {
        switch(strlen(currentline.suffix)) {
            case 1: // .s or .l
                switch(tolower(currentline.suffix[0])) {
                    case 's':
                        if(adlmode) return S_SIL;  // SIL
                        else return S_SIS;         // SIS
                        break;
                    case 'l':
                        if(adlmode) return S_LIL;  // LIL
                        else return S_LIS;         // LIS
                        break;
                    default: // illegal suffix
                        break;
                }
                break;
            case 2: // .is or .il
                if(tolower(currentline.suffix[0]) != 'i') break; // illegal suffix
                switch(tolower(currentline.suffix[1])) {
                    case 's':
                        if(adlmode) return S_LIS;  // LIS
                        else return S_SIS;         // SIS
                        break;
                    case 'l':
                        if(adlmode) return S_LIL;  // LIL
                        else return S_SIL;         // SIL
                        break;
                    default: // illegal suffix
                        break;
                }
                break;
            case 3:
                if(tolower(currentline.suffix[1]) != 'i') break; // illegal suffix
                switch(tolower(currentline.suffix[0])) {
                    case 's':
                        if(currentline.suffix[2] == 's') return S_SIS; // SIS
                        if(currentline.suffix[2] == 'l') return S_SIL; // SIL
                        // illegal suffix
                        break;
                    case 'l':
                        if(currentline.suffix[2] == 's') return S_LIS; // LIS
                        if(currentline.suffix[2] == 'l') return S_LIL; // LIL
                        // illegal suffix
                        break;
                    default: // illegal suffix
                        break;
                }
                break;
            default: // illegal suffix
                break;
        }
        error(message[ERROR_INVALIDSUFFIX]);
    }
    return 0;
}

void adl_action() {
    if(strcmp(currentline.operand1, "0") == 0) adlmode = false;
    if(strcmp(currentline.operand1, "1") == 0) adlmode = true;
}

// get the number of bytes to emit from an immediate
uint8_t get_immediate_size(operand *op, uint8_t suffix) {
    uint8_t num;
    switch(suffix) {
        case S_SIS:
        case S_LIS:
            num = 2;
            break;
        case S_SIL:
        case S_LIL:
            num = 3;
            break;
        case 0: // Use current ADL mode to determine 16/24 bit
            if(adlmode) num = 3;
            else num = 2;
            break;
        default:
            error(message[ERROR_INVALIDMNEMONIC]);
            return 0;
    }
    if((num == 2) && (op->immediate > 0xFFFF)) error(message[ERROR_MMN_TOOLARGE]);
    return num;
}
// Emit a 16 or 24 bit immediate number, according to
// given suffix bit, or in lack of it, the current ADL mode
void emit_immediate(operand *op, uint8_t suffix) {
    uint8_t num;

    num = get_immediate_size(op, suffix);
    emit_8bit(op->immediate & 0xFF);
    emit_8bit((op->immediate >> 8) & 0xFF);
    if(num == 3) emit_8bit((op->immediate >> 16) & 0xFF);
}

void emit_adlsuffix_code(uint8_t suffix) {
    uint8_t code;
    switch(suffix) {
        case S_SIS:
            code = CODE_SIS;
            break;
        case S_LIS:
            code = CODE_LIS;
            break;
        case S_SIL:
            code = CODE_SIL;
            break;
        case S_LIL:
            code = CODE_LIL;
            break;
        default:
            error(message[ERROR_INVALIDSUFFIX]);
            return;
    }
    emit_8bit(code);
}

uint8_t get_ddfd_prefix(cpuregister reg) {
    switch(reg) {
        case R_IX:
        case R_IXH:
        case R_IXL:
            return 0xDD;
        case R_IY:
        case R_IYH:
        case R_IYL:
            return 0xFD;
        default:
            break;
    }
    return 0;    
}

void prefix_ddfd_suffix(operandlist *op) {
    uint8_t prefix1, prefix2;

    if(op->ddfdpermitted) {
        prefix1 = get_ddfd_prefix(operand1.reg);
        prefix2 = get_ddfd_prefix(operand2.reg);


        // prefix in either of these two cases
        if(prefix1) {
            if(prefix2) {
                // both prefixes set
                if(operand1.indirect) {
                    output.prefix1 = prefix1;
                }
                else {
                    output.prefix1 = prefix2;
                }
            }
            else {
                output.prefix1 = prefix1;
            }
        }
        else {
            if(prefix2) {
                output.prefix1 = prefix2;
            }
        }
    }
}

void transform_instruction(operand *op, permittype type) {
    uint8_t y;
    int24_t rel;

    switch(type) {
        case TRANSFORM_IR0:
            if((op->reg == R_IXL) || (op->reg == R_IYL)) output.opcode |= 0x01;
            break;
        case TRANSFORM_IR3:
            if((op->reg == R_IXL) || (op->reg == R_IYL)) output.opcode |= 0x08;
            break;
        case TRANSFORM_Z:
            output.opcode |= op->reg_index;
            break;
        case TRANSFORM_Y:
            if(op->immediate_provided) output.opcode |= (op->immediate << 3);
            else output.opcode |= (op->reg_index << 3);
            break;
        case TRANSFORM_P:
            output.opcode |= (op->reg_index << 4);
            break;
        case TRANSFORM_CC:
            output.opcode |= (op->cc_index << 3);
            break;
        case TRANSFORM_N:
            output.opcode |= op->immediate;
            op->immediate_provided = false; // no separate output for this transform
            break;
        case TRANSFORM_BIT:
            output.opcode |= (op->immediate << 3);
            op->immediate_provided = false;
            break;
        case TRANSFORM_SELECT:
            switch(op->immediate) {
                case 0:
                    y = 0;
                    break;
                case 1:
                    y = 2;
                    break;
                case 2:
                    y = 3;
                    break;
                default:
                    y = 0;
            }
            output.opcode |= (y << 3); // shift 3 lower bits 3 to the left
            op->immediate_provided = false; // no separate output for this transform
            break;
        case TRANSFORM_REL:
            if(pass == 2) {
                // label still potentially unknown in pass 1, so output the existing '0' in pass 1
                if(op->wasLabel) rel = op->immediate - address - 2;
                else rel = op->immediate; // user asked for specific offset
                if((rel > 127) || (rel < -128)) error(message[ERROR_RELATIVEJUMPTOOLARGE]);
                op->immediate = ((int8_t)(rel & 0xFF));
                op->immediate_provided = true;
            }
            break;
        case TRANSFORM_NONE:
            break;
        default:
            error(message[ERROR_TRANSFORMATION]);
            break;
    }
    return;
}

void emit_instruction(operandlist *list) {
    bool ddbeforeopcode; // determine position of displacement byte in case of DDCBdd/DDFDdd

    // Transform necessary prefix/opcode in output, according to given list and operands
    output.suffix = getADLsuffix();
    output.prefix1 = 0;
    output.prefix2 = list->prefix;
    output.opcode = list->opcode;

    if(pass == 1) definelabel(address);

    // issue any errors here
    if((list->transformA != TRANSFORM_REL) && (list->transformB != TRANSFORM_REL)) { // TRANSFORM_REL will mask to 0xFF
        if(((list->operandA == OPTYPE_N) || (list->operandA == OPTYPE_INDIRECT_N)) && ((operand1.immediate > 0xFF) || (operand1.immediate < -128))) error(message[WARNING_N_8BITRANGE]);
        if(((list->operandB == OPTYPE_N) || (list->operandB == OPTYPE_INDIRECT_N)) && ((operand2.immediate > 0xFF) || (operand2.immediate < -128))) error(message[WARNING_N_8BITRANGE]);
    }
    if((output.suffix) && ((list->adl & output.suffix) == 0)) error(message[ERROR_ILLEGAL_SUFFIXMODE]);
    if((operand2.displacement_provided) && ((operand2.displacement < -128) || (operand2.displacement > 127))) error(message[ERROR_DISPLACEMENT_RANGE]);

    // Specific checks
    if((list->operandA == OPTYPE_BIT) && (operand1.immediate > 7)) error(message[ERROR_INVALIDBITNUMBER]);
    if((list->operandA == OPTYPE_NSELECT) && (operand1.immediate > 2)) error(message[ERROR_ILLEGALINTERRUPTMODE]);
    if((list->transformA == TRANSFORM_N) && (operand1.immediate & 0x47)) error(message[ERROR_ILLEGALRESTARTADDRESS]);

    // prepare extra DD/FD suffix if needed
    prefix_ddfd_suffix(list);
    // Transform the opcode and potential immediate values, according to the current ruleset
    transform_instruction(&operand1, list->transformA);
    transform_instruction(&operand2, list->transformB);
    // determine position of dd
    ddbeforeopcode = (((output.prefix1 == 0xDD) || (output.prefix1 == 0xFD)) && (output.prefix2 == 0xCB) &&
                ((operand1.displacement_provided) || (operand2.displacement_provided)));
    
    // output adl suffix and any prefixes
    if(output.suffix > 0) emit_adlsuffix_code(output.suffix);
    if(output.prefix1) emit_8bit(output.prefix1);
    if(output.prefix2) emit_8bit(output.prefix2);

    // opcode in normal position
    if(!ddbeforeopcode) emit_8bit(output.opcode);
    
    // output displacement
    if(operand1.displacement_provided) emit_8bit(operand1.displacement & 0xFF);
    if(operand2.displacement_provided) emit_8bit(operand2.displacement & 0xFF);
    // output n
    if((operand1.immediate_provided) && ((list->operandA == OPTYPE_N) || (list->operandA == OPTYPE_INDIRECT_N))) emit_8bit(operand1.immediate & 0xFF);
    if((operand2.immediate_provided) && ((list->operandB == OPTYPE_N) || (list->operandB == OPTYPE_INDIRECT_N))) emit_8bit(operand2.immediate & 0xFF);


    // opcode in DDCBdd/DFCBdd position
    if(ddbeforeopcode) emit_8bit(output.opcode);

    //output remaining immediate bytes
    if((list->operandA == OPTYPE_MMN) || (list->operandA == OPTYPE_INDIRECT_MMN)) emit_immediate(&operand1, output.suffix);
    if((list->operandB == OPTYPE_MMN) || (list->operandB == OPTYPE_INDIRECT_MMN)) emit_immediate(&operand2, output.suffix);
}

void emit_8bit(uint8_t value) {
    if(pass == 2) {
        listEmit8bit(value);
        agon_fwrite(&value, sizeof(char), 1, FILE_OUTPUT);
    }
    address++;
    totalsize++;
}

void emit_16bit(uint16_t value) {
    emit_8bit(value&0xFF);
    emit_8bit((value>>8)&0xFF);
}

void emit_24bit(uint24_t value) {
    emit_8bit(value&0xFF);
    emit_8bit((value>>8)&0xFF);
    emit_8bit((value>>16)&0xFF);
}

void emit_32bit(uint32_t value) {
    emit_8bit(value&0xFF);
    emit_8bit((value>>8)&0xFF);
    emit_8bit((value>>16)&0xFF);
    emit_8bit((value>>24)&0xFF);
}

// return the value of a previously escaped character with backslash
uint8_t get_escaped_char(char n) {
    switch(n) {
        case 'n':
            return(0x0a);
        case 'r':
            return(0x0d);
        case 't':
            return(0x09);
        case 'b':
            return(0x08);
        case 'e':
            return(0x1b);
        case '\"':
            return('\"');
        case '\'':
            return('\'');
        default:
            return(n);
    }
}

// emits a string surrounded by literal string quotes, as the token gets in from a file
void emit_quotedstring(char *str) {
    bool escaped = false;

    if(*str == '\"') {
        str++;
        while(*str) {
            switch(*str) {
                case '\\':
                    if(escaped) {
                        emit_8bit('\\');
                        escaped = false;
                    }
                    else escaped = true;
                    break;
                case 'n':
                case 'r':
                case 't':
                case 'b':
                case 'e':
                case '\'':
                    if(escaped) emit_8bit(get_escaped_char(*str));
                    else emit_8bit(*str); // the normal character
                    escaped = false;
                    break;
                case '\"':
                    if(escaped) {
                        emit_8bit('\"');
                        escaped = false;
                    }
                    else {
                        if(*(str+1) != 0) error(message[ERROR_STRINGFORMAT]);
                        return; // end of quoted string
                    }
                    break;
                default:
                    emit_8bit(*str);
            }
            str++;
        }
        // we missed an end-quote to this string, we shouldn't reach this
        error(message[ERROR_STRINGFORMAT]);
    }
    else error(message[ERROR_STRINGFORMAT]);
}

void parse_asm_single_immediate(void) {
    tokentype token;

    if(currentline.next) {
        getLineToken(&token, currentline.next,0);
        if(notEmpty(token.start)) {
            operand1.immediate = str2num(token.start,true);
            operand1.immediate_provided = true;
            if(token.terminator != 0) error(message[ERROR_TOOMANYARGUMENTS]);
        }
        else error(message[ERROR_MISSINGOPERAND]);
    }
    else error(message[ERROR_MISSINGOPERAND]);
}

void parse_asm_keyval_pair(void) {
    tokentype token;

    if(currentline.next) {
        getLineToken(&token, currentline.next, '=');
        strcpy(currentline.operand1, token.start);
        if(token.terminator == '=') {
            getLineToken(&token, token.next, 0);
            if(notEmpty(token.start)) {
                operand2.immediate = str2num(token.start,true);
                operand2.immediate_provided = true;
            }
            else error(message[ERROR_MISSINGOPERAND]);
        }        
        else error(message[ERROR_MISSINGOPERAND]);
    }
    else error(message[ERROR_MISSINGOPERAND]);
}

void handle_asm_db(void) {
    tokentype token;

    if(pass == 1) {
        // Output label at this address
        definelabel(address);
    }
    if(currentline.next) {
        while(currentline.next) {
            getLineToken(&token, currentline.next, ',');
            if(notEmpty(token.start)) {
                switch(token.start[0]) {
                    case '\"':
                        emit_quotedstring(token.start);
                        break;
                    default:
                        operand1.immediate = getLabelValue(token.start);
                        if(operand1.immediate > 0xff) error(message[WARNING_N_TOOLARGE]);
                        emit_8bit(operand1.immediate);
                        break;
                        
                }
            }
            if(token.terminator == ',') currentline.next = token.next;
            else {
                if((token.terminator != 0) &&(token.terminator != ';')) error(message[ERROR_LISTFORMAT]);
                currentline.next = NULL; 
            }
        }
    }
    else error(message[ERROR_MISSINGOPERAND]); // we need at least one value
}

void handle_asm_dw(bool longword) {
    label *lbl;
    tokentype token;
    if(pass == 1) {
        // Output label at this address
        definelabel(address);
    }
    if(currentline.next) {
        while(currentline.next) {
            getLineToken(&token, currentline.next, ',');
            if(notEmpty(token.start)) {
                lbl = findLabel(token.start);
                if(lbl) operand1.immediate = lbl->address;
                else operand1.immediate = str2num(token.start,true);
                
                if(longword) {
                    emit_24bit(operand1.immediate);
                }
                else {
                    if(operand1.immediate > 0xffffff) error(message[ERROR_ADLWORDSIZE]);
                    emit_16bit(operand1.immediate);
                }
            }
            if(token.terminator == ',') currentline.next = token.next;
            else {
                if((token.terminator != 0) && (token.terminator != ';')) error(message[ERROR_LISTFORMAT]);
                currentline.next = NULL; 
            }
        }
    }
    else error(message[ERROR_MISSINGOPERAND]); // we need at least one value
}

void handle_asm_equ(void) {
    tokentype token;

    if(currentline.next) {
        getLineToken(&token, currentline.next, 0);
        if(notEmpty(token.start)) {
            if((token.terminator != 0) && (token.terminator != ';')) error(message[ERROR_TOOMANYARGUMENTS]);
            if(pass == 1) definelabel(getLabelValue(token.start));
            /*
            if(pass == 1) definelabel(0);
            if(pass == 2) {
                lbl = findLabel(currentline.label);
                if(lbl) lbl->address = getLabelValue(token.start);
                else error(message[ERROR_MISSINGLABEL]);
            }
            */
        }
        else error(message[ERROR_MISSINGOPERAND]);
    }
    else error(message[ERROR_MISSINGOPERAND]);
}

void handle_asm_adl(void) {
    parse_asm_keyval_pair();
    if(strcasecmp(currentline.operand1, "adl") == 0) {
        if((operand2.immediate == 0) || (operand2.immediate == 1)) {
            adlmode = operand2.immediate;
        }
        else error(message[ERROR_INVALID_ADLMODE]);
    }
    else error(message[ERROR_INVALIDOPERAND]);
}

void handle_asm_org(void) {
    uint24_t newaddress;
    
    parse_asm_single_immediate(); // get address from next token
    newaddress = operand1.immediate;
    if((adlmode == 0) && (newaddress > 0xffff)) error(message[ERROR_ADDRESSRANGE]); 
    if(newaddress >= address) {
        if(pass == 1) {
            // Output label at this address
            definelabel(address); // set address to current line
        }
        address = newaddress;
    }
    else error(message[ERROR_ADDRESSLOWER]);
}

void handle_asm_include(void) {
    tokentype token;
    filestackitem fsi;
    if(currentline.next) {
        getLineToken(&token, currentline.next, 0);
        if(token.start[0] == '\"') {
            token.start[strlen(token.start)-1] = 0;
            fsi.linenumber = linenumber;
            fsi.fp = filehandle[FILE_CURRENT];
            strcpy(fsi.filename, filename[FILE_CURRENT]);
            filestackPush(&fsi);
            filehandle[FILE_CURRENT] = mos_fopen(token.start+1, fa_read);
            strcpy(filename[FILE_CURRENT], token.start+1);
            if(filehandle[FILE_CURRENT] == 0) {
                filestackPop(&fsi);
                error(message[ERROR_INCLUDEFILE]);
            }
            lineNumberNeedsReset = true;
        }
        else error(message[ERROR_STRINGFORMAT]);
        if(token.terminator != 0) error(message[ERROR_TOOMANYARGUMENTS]);
    }
    else error(message[ERROR_MISSINGOPERAND]);
}



void handle_asm_blk(uint8_t width) {
    uint16_t num;
    int24_t val = 0;
    tokentype token;

    if(pass == 1) {
        // Output label at this address
        definelabel(address);
    }

    if(currentline.next) {
        getLineToken(&token, currentline.next, ',');
        if(notEmpty(token.start)) {
            //num = str2num(token.start,true);
            num = getLabelValue(token.start);

            if(token.terminator == ',') {
                getLineToken(&token, token.next, 0);
                if(notEmpty(token.start)) {
                    if(token.start[0] == '\'') val = getAsciiValue(token.start);
                    //else val = str2num(token.start,true);
                    else val = getLabelValue(token.start);
                }
                else error(message[ERROR_MISSINGOPERAND]);
            }
            else if((token.terminator != 0)  && (token.terminator != ';')) error(message[ERROR_LISTFORMAT]);
            while(num--) {
                switch(width) {
                    case 1:
                        emit_8bit(val);
                        break;
                    case 2:
                        emit_16bit(val);
                        break;
                    case 3:
                        emit_24bit(val);
                        break;
                    case 4:
                        emit_32bit(val);
                        break;
                }
            }
        }
        else error(message[ERROR_MISSINGOPERAND]); // we need at least one value
    }
    else error(message[ERROR_MISSINGOPERAND]); // we need at least one value
}

void handle_asm_align(void) {
uint24_t alignment;
uint24_t base;
uint24_t delta;

    if(pass == 1) {
        // Output label at this address
        definelabel(address);
    }

    parse_asm_single_immediate();
    if(operand1.immediate > 0) {
        if((operand1.immediate & (operand1.immediate - 1)) == 0) {
            alignment = operand1.immediate;
            base = (~(operand1.immediate - 1) & address);

            if(address & (operand1.immediate -1)) base += alignment;
            delta = base - address;
            while(delta--) emit_8bit(FILLBYTE);

            address = base;
            if(pass == 1) {
                definelabel(address); // set address to current line
            }
        }
        else error(message[ERROR_POWER2]); 
    }
    else error(message[ERROR_INVALIDNUMBER]);
}

void handle_asm_endmacro(void) {
    if(pass == 1) {
        mos_fclose(filehandle[FILE_MACRO]);
    }
    inmacro = false;
}

void handle_asm_definemacro(void) {
    
    tokentype token;
    uint8_t argcount = 0;
    char arglist[MACROMAXARGS][MACROARGLENGTH];
    
    if(pass == 1) {
        // Output any label at this address
        definelabel(address);

        // parse arguments into array
        if(currentline.next) {
            getLineToken(&token, currentline.next, ' ');
            if(notEmpty(token.start)) {
                if(findMacro(currentline.mnemonic) != 0) {
                    error(message[ERROR_MACRODEFINED]);
                    return;
                }
                strcpy(currentline.mnemonic, token.start);
                currentline.next = token.next;
                if((token.terminator == ' ') || (token.terminator == '\t')) {
                    while(currentline.next) {
                        if(argcount == MACROMAXARGS) error(message[ERROR_MACROARGCOUNT]);
                        getLineToken(&token, currentline.next, ',');
                        if(notEmpty(token.start)) {
                            strcpy(arglist[argcount], token.start);
                            argcount++;
                        }
                        if(token.terminator == ',') currentline.next = token.next;
                        else {
                            if((token.terminator != 0) &&(token.terminator != ';')) error(message[ERROR_LISTFORMAT]);
                            currentline.next = NULL; 
                        }
                    }
                }
                // record the macro to memory
                defineMacro(currentline.mnemonic, argcount, (char *)arglist);
                // define macro filename
                getMacroFilename(filename[FILE_MACRO], currentline.mnemonic);
                if(openFile(&filehandle[FILE_MACRO], filename[FILE_MACRO], fa_write | fa_create_always)) {
                    // start writing macro lines to file, keep file open until 'endmacro'
                    //printf("Writing to file <<%s>>\n",filename[FILE_MACRO]);
                }
                else error("Error writing macro file");
            }
            else error(message[ERROR_MACRONAME]);
        }
        else error(message[ERROR_MACRONAME]);
    }
    /*
    if(pass == 1) {
        printf("Macro name: <<%s>>\n",currentline.mnemonic);
        printf("Marco args: %d\n",argcount);
        for(i = 0; i < argcount; i++) {
            printf("Macro  arg: <<%s>>\n",arglist[i]);
        }
    
        macro *test = findMacro(currentline.mnemonic);
        if(test) {
            printf("Macro name: [[%s]]\n",test->name);
            for(i = 0; i < test->argcount; i++) {
                printf("macro  arg: [[%s]]\n",test->arguments[i]);
            }
        }
        else printf("Not found in memory\n");
    }
    */    
    inmacro = true;
}

void handle_assembler_command(void) {
    switch(currentline.current_instruction->asmtype) {
    case(ASM_ADL):
        handle_asm_adl();
        break;
    case(ASM_ORG):
        handle_asm_org();
        break;
    case(ASM_DB):
        handle_asm_db();
        break;
    case(ASM_DS):
        handle_asm_blk(1);
        break;
    case(ASM_DW):
        handle_asm_dw(false);
        break;
    case(ASM_DW24):
        handle_asm_dw(true);
        break;
    case(ASM_ASCIZ):
        handle_asm_db();
        emit_8bit(0);
        break;
    case(ASM_EQU):
        handle_asm_equ();
        break;
    case(ASM_INCLUDE):
        handle_asm_include();
        break;
    case(ASM_BLKB):
        handle_asm_blk(1);
        break;
    case(ASM_BLKW):
        handle_asm_blk(2);
        break;
    case(ASM_BLKP):
        handle_asm_blk(3);
        break;
    case(ASM_BLKL):
        handle_asm_blk(4);
        break;
    case(ASM_ALIGN):
        handle_asm_align();
        break;
    case(ASM_MACRO_START):
        handle_asm_definemacro();
        break;
    case(ASM_MACRO_END):
        handle_asm_endmacro();
        break;
    }
    return;
}

void processInstructions(char *line){
    operandlist *list;
    uint8_t listitem;
    bool match;

    if(!inmacro) {
        // return on empty lines
        if(isEmpty(currentline.mnemonic)) {
            // check if there is a single label on a line in during pass 1
            if(pass == 1) definelabel(address);
            return; // valid line, but empty
        }
    }

    if(currentline.current_instruction) {
        if(currentline.current_instruction->type == EZ80) {
            if(!inmacro) {
                // process this mnemonic by applying the instruction list as a filter to the operand-set
                list = currentline.current_instruction->list;
                match = false;
                for(listitem = 0; listitem < currentline.current_instruction->listnumber; listitem++) {
                    if(permittype_matchlist[list->operandA].match(&operand1) && permittype_matchlist[list->operandB].match(&operand2)) {
                        match = true;
                        emit_instruction(list);
                        break;
                    }
                    list++;
                }
                if(!match) error(message[ERROR_OPERANDSNOTMATCHING]);
                return;
            }
            else {
                if(pass == 1) agon_fputs(line, filehandle[FILE_MACRO]); // emit entire line to macro file
            }
        }
        else handle_assembler_command();
    }
    return;
}

void passInitialize(uint8_t passnumber) {
    pass = passnumber;
    adlmode = ADLMODE_START;
    linenumber = 0;
    address = START_ADDRESS;
    totalsize = 0;
    inmacro = false;
    // init the file stack and push the primary input file
    filestackInit();
}

// Assembler directives may demand a late reset of the linenumber, after the listing has been done
void processDelayedLineNumberReset(void) {
    if(lineNumberNeedsReset) {
        lineNumberNeedsReset = false;
        linenumber = 0;
    }
}

bool assemble(void){
    char line[LINEMAX];
    filestackitem fsitem;
    bool incfiles;

    global_errors = 0;

    filehandle[FILE_CURRENT] = filehandle[FILE_INPUT];
    strcpy(filename[FILE_CURRENT], filename[FILE_INPUT]);

    // Assemble in two passes
    // Pass 1
    printf("Pass 1...\n\r");
    passInitialize(1);
    do {
        while (agon_fgets(line, sizeof(line), FILE_CURRENT)){
            linenumber++;
            parseLine(line);
            processInstructions(line);
            processDelayedLineNumberReset();

            //printLocalLabelTable();

        }
        if(filestackCount()) {
            mos_fclose(filehandle[FILE_CURRENT]);
            incfiles = filestackPop(&fsitem);
            linenumber = fsitem.linenumber;
            filehandle[FILE_CURRENT] = fsitem.fp;
            strcpy(filename[FILE_CURRENT], fsitem.filename);
        }
        else incfiles = false;
    }
    while(incfiles);
    writeLocalLabels();
    if(global_errors) return false;

    // Pass 2
    printf("Pass 2...\n\r");
    //rewind(filehandle[FILE_INPUT]);
    reOpenFile(FILE_INPUT, fa_read);
    //rewind(filehandle[FILE_LOCAL_LABELS]);
    reOpenFile(FILE_LOCAL_LABELS, fa_read);
    //rewind(filehandle[FILE_ANONYMOUS_LABELS]);
    reOpenFile(FILE_ANONYMOUS_LABELS, fa_read);
    passInitialize(2);
    listInit(consolelist_enabled);
    readLocalLabels();
    readAnonymousLabel();
    
    filehandle[FILE_CURRENT] = filehandle[FILE_INPUT];
    do {
        while (agon_fgets(line, sizeof(line), FILE_CURRENT)){
            linenumber++;
            listStartLine(line);
            parseLine(line);
            refreshlocalLabels();
            processInstructions(line);
            listEndLine(consolelist_enabled);
            processDelayedLineNumberReset();

            //printLocalLabelTable();
        }
        if(filestackCount()) {
            mos_fclose(filehandle[FILE_CURRENT]);
            incfiles = filestackPop(&fsitem);
            linenumber = fsitem.linenumber;
            filehandle[FILE_CURRENT] = fsitem.fp;
            strcpy(filename[FILE_CURRENT], fsitem.filename);
        }
        else incfiles = false;
    }
    while(incfiles);
    
    return true;
}


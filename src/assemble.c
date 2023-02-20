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

void empty_operand(operand *op) {
    op->position = 0;
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

// parses the given string to the operand, or throws errors along the way
// will destruct parts of the original string during the process
void parse_operand(operand_position pos, char *string, operand *operand) {
    char *ptr = string;
    uint8_t len = strlen(string);
    label *lbl;

    operand->position = pos;
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
                            if(isdigit(*ptr)) {
                                operand->reg = R_IX;
                                operand->displacement_provided = true;
                                if(*(ptr-1) == '-') operand->displacement = -1 * (int16_t) str2num(ptr);
                                else operand->displacement = (int16_t) str2num(ptr);
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
                            if(isdigit(*ptr)) {
                                operand->reg = R_IY;
                                operand->displacement_provided = true;
                                if(*(ptr-1) == '-') operand->displacement = -1 * (int16_t) str2num(ptr);
                                else operand->displacement = (int16_t) str2num(ptr);
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
            return;
        }
        if(operand->indirect) lbl = findLabel(string+1);
        else lbl = findLabel(string);
        if(lbl) {
            operand->immediate = lbl->address;
            operand->immediate_provided = true;
            //printf("Label found: %s, %u\n",lbl->name, lbl->address);
            operand->wasLabel = true;
            return;
        }
        else {
            //printf("Label not found\n");
            if(pass == 1) {
                // might be a lable that isn't defined yet. will see in pass 2
                operand->immediate = 0;
                operand->immediate_provided = true;
                operand->wasLabel = true;
            }
            else error(message[ERROR_INVALIDLABEL]); // pass 2, not a label, error
        }
    }
}

bool parseCombinedLabel(uint32_t *value, char *token) {
    label *lbl1;
    label *lbl2;
    uint32_t part2;
    char op = 0;
    char *s = token;

    lbl1 = findLabel(token);
    if(lbl1) { // single label found
        *value = lbl1->address;
        return true;
    }

    while(*s) {
        if((*s == '+') || (*s == '-')) {
            op = *s;
            break;
        }
        s++;
    }
    *s = 0; // split or re-terminate

    if((op != '+') && (op != '-')) {
        if(pass == 1) *value = 0;
        else *value = str2num(token);
        return true; // future single label or value
    }
    s++;
    printf("Part1: <<%s>>\n",token);
    printf("Part2: <<%s>>\n",s);

    lbl1 = findLabel(token);
    lbl2 = findLabel(s);

    if(lbl1) *value = lbl1->address;
    else *value = str2num(token);
    if(lbl2) part2 = lbl2->address;
    else part2 = str2num(s);
    printf("Part1: %ld\n",*value);
    printf("Part2: %ld\n",part2);
    if(op == '+') *value += part2;
    if(op == '-') *value -= part2;
    return true;
}

// converts everything to lower case, except comments
void convertLower(char *line) {
    char *ptr = line;
    while((*ptr) && (*ptr != ';')) {
        *ptr = tolower(*ptr);
        ptr++;
    }
}

typedef enum {
    PS_START,
    PS_LABEL,
    PS_COMMAND,
    PS_OP1,
    PS_OP2,
    PS_COMMENT,
    PS_DONE,
    PS_ERROR
} parsestate;

void parse(char *src) {
    uint8_t x;
    bool done;
    uint8_t state;

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
    currentline.buffer[0] = 0;

    empty_operand(&operand1);
    empty_operand(&operand2);

    state = PS_START;
    done = false;
    while(!done) {
        switch(state) {
            case PS_START:
                x = get_token(&token, src);
                switch(token.terminator) {
                    case ':':
                        state = PS_LABEL;
                        break;
                    case ';':
                        state = PS_COMMENT;
                        break;
                    case 0:
                    case ' ':
                    case '\t':
                        if(x) state = PS_COMMAND;
                        else state = PS_DONE;
                        break;
                    default:
                        state = PS_ERROR;
                        break;
                }
                break;
            case PS_LABEL:
                strcpy(currentline.label,token.start);
                x = get_token(&token,token.next);
                switch(token.terminator) {
                    case ';':
                        state = PS_COMMENT;
                        if(pass == 2) advanceLocalLabel();
                        break;
                    case 0:
                    case ' ':
                    case '\t':
                        if(x) state = PS_COMMAND;
                        else state = PS_DONE;
                        if(pass == 2) advanceLocalLabel();
                        break;
                    default:
                        if(pass == 2) advanceLocalLabel();
                        state = PS_ERROR;
                        break;
                }                
                break;
            case PS_COMMAND:
                split_suffix(currentline.mnemonic, currentline.suffix, token.start);
                //printf("cmd: <<%s>> suffix <<%s>>\n", currentline.mnemonic,currentline.suffix);
                currentline.current_instruction = instruction_table_lookup(currentline.mnemonic);
                if(currentline.current_instruction == NULL) {
                    // check if 'suffix' part is actually a assembly command
                    currentline.current_instruction = instruction_table_lookup(currentline.suffix);
                    if(currentline.current_instruction == NULL) {
                        error(message[ERROR_INVALIDMNEMONIC]);
                        state = PS_DONE;
                        break;
                    }
                    // instruction is an assembly command, copy it to mnemonic and clear out suffix part
                    strcpy(currentline.mnemonic, currentline.suffix);
                    currentline.suffix[0] = 0;
                }
                // resume parsing the original line
                switch(token.terminator) {
                    case ';':
                        state = PS_COMMENT;
                        break;
                    case ' ':
                    case '\t':
                        if(currentline.current_instruction->type == EZ80) {
                            x = get_token(&token,token.next);
                            state = PS_OP1;
                        }
                        else {
                            currentline.next = token.next;
                            state = PS_DONE; // parse assembly later
                        }
                        break;
                    case 0:
                        state = PS_DONE;
                        break;
                    default:
                        state = PS_ERROR;
                        break;
                }                
                break;
            case PS_OP1:
                strcpy(currentline.operand1,token.start);
                switch(token.terminator) {
                    case ';':
                        parse_operand(POS_SOURCE, currentline.operand1, &operand1);
                        state = PS_COMMENT;
                        break;
                    case 0:
                        parse_operand(POS_SOURCE, currentline.operand1, &operand1);
                        state = PS_DONE;
                        break;
                    case ',':
                        parse_operand(POS_SOURCE, currentline.operand1, &operand1);
                        x = get_token(&token,token.next);
                        state = PS_OP2;
                        break;
                    default:
                        state = PS_ERROR;
                        break;
                }                
                break;
            case PS_OP2:
                strcpy(currentline.operand2,token.start);
                switch(token.terminator) {
                    case ';':
                    parse_operand(POS_DESTINATION, currentline.operand2, &operand2);
                        state = PS_COMMENT;
                        break;
                    case 0:
                    parse_operand(POS_DESTINATION, currentline.operand2, &operand2);
                        state = PS_DONE;
                        break;
                    default:
                        state = PS_ERROR;
                        break;
                }                
                break;
            case PS_COMMENT:
                strcpy(currentline.comment,token.next);
                state = PS_DONE;
                break;
            case PS_ERROR:
                printf("Error during parsing\n");
                state = PS_DONE;
                break;
            case PS_DONE:
                done = true;
                break;
        }
    }
}

void definelabel(uint32_t num){
    if(strlen(currentline.label)) {
        //printf("Inserting label %s, %08x\n",currentline.label, num);
        if(currentline.label[0] == '@') {
            if(currentline.label[1] == '@') {
                //printf("Line %d - Writing anon label\n",linenumber);
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
            //printf("Line %d Global label found - writing local labels\n", linenumber);
            writeLocalLabels(file_locals);
            clearLocalLabels();
        }
    }
}

void refreshlocalLabels(void) {
    if(pass == 2) {
        if(currentline.label[0]) {
            if(currentline.label[0] != '@') {
                //printf("Line %d Global label found - reading local labels\n", linenumber);
                clearLocalLabels();
                readLocalLabels(file_locals);
                //printLocalLabels();
            }
        }
    }
}

// return ADL prefix bitfield, or 0 if none present
uint8_t getADLsuffix(void) {
    if(currentline.suffix[0]) {
        switch(strlen(currentline.suffix)) {
            case 1: // .s or .l
                switch(currentline.suffix[0]) {
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
                if(currentline.suffix[0] != 'i') break; // illegal suffix
                switch(currentline.suffix[1]) {
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
                if(currentline.suffix[1] != 'i') break; // illegal suffix
                switch(currentline.suffix[0]) {
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
                    printf("both - setting op1\n");
                    output.prefix1 = prefix1;
                }
                else {
                    printf("both - Setting op2\n");
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
    int32_t rel;

    switch(type) {
        case TRANSFORM_IR:
            if((op->reg == R_IXL) || (op->reg == R_IYL)) {
                if(op->position == POS_DESTINATION) output.opcode |= 0x01; // bit 0
                else output.opcode |= 0x08; // bit 3
            }
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

    if(pass == 1) {
        // issue any errors here
        if((list->transformA != TRANSFORM_REL) && (list->transformB != TRANSFORM_REL)) { // TRANSFORM_REL will mask to 0xFF
            if(((list->operandA == OPTYPE_N) || (list->operandA == OPTYPE_INDIRECT_N)) && (operand1.immediate > 0xFF)) error(message[WARNING_N_TOOLARGE]);
            if(((list->operandB == OPTYPE_N) || (list->operandB == OPTYPE_INDIRECT_N)) && (operand2.immediate > 0xFF)) error(message[WARNING_N_TOOLARGE]);
        }
        if((output.suffix) && ((list->adl & output.suffix) == 0)) error(message[ERROR_ILLEGAL_SUFFIXMODE]);
        if((operand2.displacement_provided) && ((operand2.displacement < -128) || (operand2.displacement > 127))) error(message[ERROR_DISPLACEMENT_RANGE]);

        // Specific checks
        if((list->operandA == OPTYPE_BIT) && (operand1.immediate > 7)) error(message[ERROR_INVALIDBITNUMBER]);
        if((list->operandA == OPTYPE_NSELECT) && (operand1.immediate > 2)) error(message[ERROR_ILLEGALINTERRUPTMODE]);
        if((list->transformA == TRANSFORM_N) && (operand1.immediate & 0b1000111)) error(message[ERROR_ILLEGALRESTARTADDRESS]);
        // Define label at this address
        definelabel(address);
    }

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
        //printf("%02x:",value);
        listEmit8bit(value);
        fwrite(&value, sizeof(char), 1, file_bin);
    }
    address++;
    totalsize++;
}

void emit_16bit(uint16_t value) {
    //printf("%02x:%02x\n",value&0xFF, (value>>8)&0xFF);
    emit_8bit(value&0xFF);
    emit_8bit((value>>8)&0xFF);
}

void emit_24bit(uint32_t value) {
    //printf("0x%02x-0x%02x-0x%02x\n", value&0xFF, (value>>8)&0xFF, (value>>16)&0xFF);
    emit_8bit(value&0xFF);
    emit_8bit((value>>8)&0xFF);
    emit_8bit((value>>16)&0xFF);
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
                    else return; // end of quoted string
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

void emit_quotedvalue(char *str) {
    str++;
    if(*(str+1) != '\'') error(message[ERROR_VALUEFORMAT]);
    if(*str == '\'') emit_8bit(0);
    else emit_8bit(*str);
}

void parse_asm_single_immediate(void) {
    tokentype token;

    if(currentline.next) {
        get_token(&token, currentline.next);
        if(token.start[0]) {
            operand1.immediate = str2num(token.start);
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
        get_token(&token, currentline.next);
        strcpy(currentline.operand1, token.start);
        if(token.terminator == '=') {
            get_token(&token, token.next);
            if(token.start[0]) {
                operand2.immediate = str2num(token.start);
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
            get_token(&token, currentline.next);
            if(token.start[0]) {
                //printf("DEBUG db <<%s>>\n",token.start);
                switch(token.start[0]) {
                    case '\"':
                        emit_quotedstring(token.start);
                        break;
                    case '\'':
                        emit_quotedvalue(token.start);
                        break;
                    default:
                        operand1.immediate = str2num(token.start);
                        if(operand1.immediate > 0xff) error(message[WARNING_N_TOOLARGE]);
                        emit_8bit(operand1.immediate);
                        break;
                }
            }
            if(token.terminator == ',') currentline.next = token.next;
            else {
                if(token.terminator != 0) error(message[ERROR_LISTFORMAT]);
                currentline.next = NULL; 
            }
        }
    }
    else error(message[ERROR_MISSINGOPERAND]); // we need at least one value
}

void handle_asm_dw(void) {
    label *lbl;
    tokentype token;
    if(pass == 1) {
        // Output label at this address
        definelabel(address);
    }
    if(currentline.next) {
        while(currentline.next) {
            get_token(&token, currentline.next);
            if(token.start[0]) {
                lbl = findLabel(token.start);
                if(lbl) operand1.immediate = lbl->address;
                else operand1.immediate = str2num(token.start);
                
                if(adlmode) {
                    emit_24bit(operand1.immediate);
                }
                else {
                    if(operand1.immediate > 0xffffff) error(message[ERROR_ADLWORDSIZE]);
                    emit_16bit(operand1.immediate);
                }
            }
            if(token.terminator == ',') currentline.next = token.next;
            else {
                if(token.terminator != 0) error(message[ERROR_LISTFORMAT]);
                currentline.next = NULL; 
            }
        }
    }
    else error(message[ERROR_MISSINGOPERAND]); // we need at least one value
}

void handle_asm_ds(void) {
    uint16_t num;
    uint8_t val = 0;
    tokentype token;

    if(pass == 1) {
        // Output label at this address
        definelabel(address);
    }
    if(currentline.next) {
        get_token(&token, currentline.next);
        if(token.start[0]) {
            num = str2num(token.start);

            if(token.terminator == ',') {
                get_token(&token, token.next);
                if(token.start[0]) val = str2num(token.start);
                else error(message[ERROR_MISSINGOPERAND]);
            }
            else if(token.terminator != 0) error(message[ERROR_LISTFORMAT]);
            while(num--) emit_8bit(val);
        }
        else error(message[ERROR_MISSINGOPERAND]); // we need at least one value
    }
    else error(message[ERROR_MISSINGOPERAND]); // we need at least one value
}

void handle_asm_ascii(bool terminate) {
    tokentype token;

    if(pass == 1) {
        // Output label at this address
        definelabel(address);
    }
    if(currentline.next) {
        get_token(&token, currentline.next);
        if(token.start[0] == '\"') {
            emit_quotedstring(token.start);
            if(terminate) emit_8bit(0);
        }
        else error(message[ERROR_STRINGFORMAT]);
        if(token.terminator != 0) error(message[ERROR_TOOMANYARGUMENTS]);
    }
    else error(message[ERROR_MISSINGOPERAND]);
}

void handle_asm_equ(void) {
    label *lbl;
    uint32_t value = 0;
    uint32_t value2 = 0;
    tokentype token;
    tokentype val1,val2;

    val2.terminator = 0;

    if(currentline.label[0]) {
        if(currentline.label[0] != '@') {
            if(currentline.next) {
                get_token(&token, currentline.next);
                if(token.start[0]) {
                    if(pass == 1) {
                        get_ValueToken(&val1, token.start);
                        if(val1.start[0]) {
                            printf("Part 1: <<%s>>\n",val1.start);
                            lbl = findLabel(val1.start);
                            if(lbl) value = lbl->address;
                            else value = str2num(val1.start);
                            printf("Val1 terminator: %x\n",val1.terminator);
                            
                            if(val1.terminator != 0) {
                                get_ValueToken(&val2, val1.next);
                                if(val2.start[0]) {
                                    printf("Part 2: <<%s>>\n",val2.start);
                                    lbl = findLabel(val2.start);
                                    if(lbl) value2 = lbl->address;
                                    else value2 = str2num(val2.start);
                                }
                            }
                        }
                        if(val1.terminator == '+') value += value2;
                        if(val1.terminator == '-') value -= value2;
                        definelabel(value);
                    }
                    if(token.terminator != 0) error(message[ERROR_TOOMANYARGUMENTS]);
                }
                else error(message[ERROR_MISSINGOPERAND]);
            }
            else error(message[ERROR_MISSINGOPERAND]);
        }
        else error(message[ERROR_INVALIDLABEL]);
    }
    else error(message[ERROR_MISSINGLABEL]);
}

void handle_asm_adl(void) {
    parse_asm_keyval_pair();
    if(strcmp(currentline.operand1, "adl") == 0) {
        if((operand2.immediate == 0) || (operand2.immediate == 1)) {
            adlmode = operand2.immediate;
            //printf("Set ADL mode to %d\n",adlmode);
        }
        else error(message[ERROR_INVALID_ADLMODE]);
    }
    else error(message[ERROR_INVALIDOPERAND]);
}

void handle_asm_org(void) {
    uint32_t i;
    uint32_t size;
    uint32_t newaddress;
    
    parse_asm_single_immediate(); // get address from next token
    newaddress = operand1.immediate;
    if((adlmode == 0) && (newaddress > 0xffff)) error(message[ERROR_ADDRESSRANGE]); 
    //printf("DEBUG - setting address %08x, pass %d\n",newaddress, pass);
    if(newaddress >= address) {
        size = newaddress-address;
        if(pass == 1) {
            // Output label at this address
            definelabel(address); // set address to current line
        }
        if(totalsize > 0) {
            //printf("DEBUG - Output %d GAP bytes\n", size);
            for(i = 0; i < (size); i++) emit_8bit(FILLBYTE);
            totalsize += size;
        }
        address = newaddress;
    }
    else error(message[ERROR_ADDRESSLOWER]);
}

void handle_asm_include(void) {
    tokentype token;
    filestackitem fsi;
    if(currentline.next) {
        get_token(&token, currentline.next);
        if(token.start[0] == '\"') {
            token.start[strlen(token.start)-1] = 0;
            //printf("Include: <<%s>>\n",token.start+1);
            fsi.linenumber = linenumber;
            fsi.fp = file_input;
            strcpy(fsi.filename, currentInputFilename);
            filestackPush(&fsi);
            file_input = fopen(token.start+1, "r");
            strcpy(currentInputFilename, token.start+1);
            if(file_input == NULL) {
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
        handle_asm_ds();
        break;
    case(ASM_DW):
        handle_asm_dw();
        break;
    case(ASM_ASCII):
        handle_asm_ascii(false);
        break;
    case(ASM_ASCIIZ):
        handle_asm_ascii(true);
        break;
    case(ASM_EQU):
        handle_asm_equ();
        break;
    case(ASM_INCLUDE):
        handle_asm_include();
        break;
    }
    return;
}

void processInstructions(void){
    operandlist *list;
    uint8_t listitem;
    bool match;

    // return on empty lines
    if((currentline.mnemonic[0]) == 0) {
        // check if there is a single label on a line in during pass 1
        if(pass == 1) definelabel(address);
        return; // valid line, but empty
    }

    if(currentline.current_instruction) {
        if(currentline.current_instruction->type == EZ80) {
            // process this mnemonic by applying the instruction list as a filter to the operand-set
            list = currentline.current_instruction->list;
            if(debug_enabled && pass == 1) {
                printf("DEBUG - Line %d - Mmemonic \'%s\'\n", linenumber, currentline.mnemonic);
                printf("DEBUG - Line %d - regA %02x regB %02x\n", linenumber, operand1.reg, operand2.reg);
                printf("DEBUG - Line %d - indirectA %02x\n", linenumber, operand1.indirect);
                printf("DEBUG - Line %d - indirectB %02x\n", linenumber, operand2.indirect);
            }
            match = false;
            for(listitem = 0; listitem < currentline.current_instruction->listnumber; listitem++) {
                if(debug_enabled && pass == 1) printf("DEBUG - Line %d - %02x %02x %02x %02x %02x %02x %02x\n", linenumber, list->operandA, list->operandB, list->transformA, list->transformB, list->prefix, list->opcode, list->adl);
                if(permittype_matchlist[list->operandA].match(&operand1) && permittype_matchlist[list->operandB].match(&operand2)) {
                    match = true;
                    if((debug_enabled) && pass == 1) printf("DEBUG - Line %d - match found on ^last^ filter list tuple\n", linenumber);
                    emit_instruction(list);
                    break;
                }
                list++;
            }
            if(!match) error(message[ERROR_OPERANDSNOTMATCHING]);
            return;
        }
        else handle_assembler_command();
    }
    return;
}

void passInitialize(uint8_t passnumber) {
    pass = passnumber;
    adlmode = true;
    linenumber = 0;
    address = START_ADDRESS;
    totalsize = 0;
    
    // init the file stack and push the primary input file
    filestackInit();
}

// Assembler directives may demand a late reset of the linenumber, after the listing has been done
static inline void processDelayedLineNumberReset(void) {
    if(lineNumberNeedsReset) {
        lineNumberNeedsReset = false;
        linenumber = 0;
    }
}

bool assemble(FILE *fp, char *filename){
    char line[LINEMAX];
    global_errors = 0;
    filestackitem fsitem;
    bool incfiles;

    file_input = fp;
    strcpy(currentInputFilename, filename);

    // Assemble in two passes
    // Pass 1
    printf("Pass 1...\n");
    passInitialize(1);
    do {
        while (fgets(line, sizeof(line), file_input)){
            linenumber++;
            convertLower(line);
            parse(line);
            processInstructions();
            processDelayedLineNumberReset();
        }
        if(filestackCount()) {
            fclose(file_input);
            incfiles = filestackPop(&fsitem);
            linenumber = fsitem.linenumber;
            file_input = fsitem.fp;
            strcpy(currentInputFilename, fsitem.filename);
        }
        else incfiles = false;
    }
    while(incfiles);
    writeLocalLabels();
    if(global_errors) return false;

    printf("%d lines\n", linenumber);
    printf("%d labels\n", getGlobalLabelCount());

    // Pass 2
    printf("Pass 2...\n");
    rewind(file_input);
    rewind(file_locals);
    rewind(file_anon);
    passInitialize(2);
    listInit(consolelist_enabled);
    readLocalLabels();
    readAnonymousLabel();
    do {
        while (fgets(line, sizeof(line), file_input)){
            linenumber++;
            listStartLine(line);
            convertLower(line);
            parse(line);
            refreshlocalLabels();
            processInstructions();
            listEndLine(consolelist_enabled);
            processDelayedLineNumberReset();
        }
        if(filestackCount()) {
            fclose(file_input);
            incfiles = filestackPop(&fsitem);
            linenumber = fsitem.linenumber;
            file_input = fsitem.fp;
            strcpy(currentInputFilename, fsitem.filename);
        }
        else incfiles = false;
    }
    while(incfiles);
    return true;
}


#include "assemble.h"

// Local file buffer
char _buffer[FILE_BUFFERSIZE];
char _incbuffer[FILESTACK_MAXFILES][FILE_BUFFERSIZE];
char _macrobuffer[FILE_BUFFERSIZE];

char _macro_OP_buffer[MACROARGLENGTH + 1]; // replacement buffer for operands during macro expansion
char _macro_VAL_buffer[MACROARGLENGTH + 1];// replacement buffer for values during macro expansion
char _macro_ASM_buffer[MACROARGLENGTH + 1];// replacement buffer for values during ASSEMBLER macro expansion
void advanceLocalLabel(void) {
    if(currentline.label) {
        if(currentline.label[0] == '@') {
            if(currentline.label[1] == '@') {
                if(!recordingMacro) readAnonymousLabel();
            }
        }
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
uint8_t getAsciiValue(char *string) {
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
int24_t getValue(char *str, bool req_firstpass) {
    streamtoken_t token;
    label_t *lbl;
    uint24_t total, tmp;
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
                if(token.start[0] == '\'') tmp = getAsciiValue(token.start);
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

// parses the given string to the operand, or throws errors along the way
// will destruct parts of the original string during the process
void parse_operand(char *string, uint8_t len, operand_t *operand) {
    char *ptr = string;

    // direct or indirect
    if(*ptr == '(') {
        operand->indirect = true;
        // find closing bracket or error out
        if(string[len-1] == ')') string[len-1] = 0; // terminate on closing bracket
        else error(message[ERROR_CLOSINGBRACKET]);
        ptr = &string[1];
        while(isspace(*ptr)) ptr++; // eat spaces
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
        case 'A':
            switch(*ptr++) {
                case 0:
                    operand->reg = R_A;
                    operand->reg_index = R_INDEX_A;
                    return;
                case 'f':
                case 'F':
                    switch(*ptr++) {
                        case 0:
                        case '\'':
                            operand->reg = R_AF;
                            operand->reg_index = R_INDEX_AF;
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
        case 'B':
            switch(*ptr++) {
                case 0:
                    operand->reg = R_B;
                    operand->reg_index = R_INDEX_B;
                    return;
                case 'c':
                case 'C':
                    if((*ptr == 0) || isspace(*ptr)) {
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
        case 'C':
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
        case 'D':
            switch(*ptr++) {
                case 0:
                    operand->reg = R_D;
                    operand->reg_index = R_INDEX_D;
                    return;
                case 'e':
                case 'E':
                    if((*ptr == 0) || isspace(*ptr)) {
                        operand->reg = R_DE;
                        operand->reg_index = R_INDEX_DE;
                        return;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 'e':
        case 'E':
            if(*ptr++ == 0) {
                operand->reg = R_E;
                operand->reg_index = R_INDEX_E;
                return;
            }
            break;
        case 'h':
        case 'H':
            switch(*ptr++) {
                case 0:
                    operand->reg = R_H;
                    operand->reg_index = R_INDEX_H;
                    return;
                case 'l':
                case 'L':
                    if((*ptr == 0) || isspace(*ptr)) {
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
        case 'I':
            switch(*ptr++) {
                case 0:
                    operand->reg = R_I;
                    operand->reg_index = R_INDEX_I;
                    return;
                case 'x':
                case 'X':
                    while(isspace(*ptr)) ptr++; // eat spaces
                    switch(*ptr++) {
                        case 0:
                            operand->reg = R_IX;
                            operand->reg_index = R_INDEX_IX;
                            return;
                        case 'h':
                        case 'H':
                            if(*ptr == 0) {
                                operand->reg = R_IXH;
                                return;
                            }
                            break;
                        case 'l':
                        case 'L':
                            if(*ptr == 0) {
                                operand->reg = R_IXL;
                                return;
                            }
                            break;
                        case '+':
                        case '-':
                            operand->reg = R_IX;
                            operand->displacement_provided = true;
                            if(*(ptr-1) == '-') operand->displacement = -1 * (int16_t) getValue(ptr, false);
                            else operand->displacement = (int16_t) getValue(ptr, false);
                            return;
                            break;
                        default:
                            break;
                    }
                    break;
                case 'y':
                case 'Y':
                    while(isspace(*ptr)) ptr++; // eat spaces
                    switch(*ptr++) {
                        case 0:
                            operand->reg = R_IY;
                            operand->reg_index = R_INDEX_IY;
                            return;
                        case 'h':
                        case 'H':
                            if(*ptr == 0) {
                                operand->reg = R_IYH;
                                return;
                            }
                            break;
                        case 'l':
                        case 'L':
                            if(*ptr == 0) {
                                operand->reg = R_IYL;
                                return;
                            }
                            break;
                        case '+':
                        case '-':
                            operand->reg = R_IY;
                            operand->displacement_provided = true;
                            if(*(ptr-1) == '-') operand->displacement = -1 * (int16_t) getValue(ptr, false);
                            else operand->displacement = (int16_t) getValue(ptr, false);
                            return;
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
        case 'L':
            if(*ptr == 0) {
                operand->reg = R_L;
                operand->reg_index = R_INDEX_L;
                return;
            }
            break;
        case 'm':
        case 'M':
            if((tolower(*ptr) == 'b') && ptr[1] == 0) {
                operand->reg = R_MB;
                operand->reg_index = R_INDEX_MB;
                return;
            }
            if(*ptr == 0) {
                operand->cc = true;
                operand->cc_index = CC_INDEX_M;
                return;
            }
            break;
        case 'n':
        case 'N':
            switch(*ptr++) {
                case 'c':   // NC
                case 'C':
                    if(*ptr == 0) {
                        operand->cc = true;
                        operand->cc_index = CC_INDEX_NC;
                        return;
                    }
                    break;
                case 'z':   // NZ
                case 'Z':
                    if(*ptr == 0) {
                        operand->cc = true;
                        operand->cc_index = CC_INDEX_NZ;
                        return;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 'p':
        case 'P':
            switch(*ptr++) {
                case 0:
                    operand->cc = true;
                    operand->cc_index = CC_INDEX_P;
                    return;
                case 'e':
                case 'E':
                    if(*ptr == 0) {
                        operand->cc = true;
                        operand->cc_index = CC_INDEX_PE;
                        return;
                    }
                    break;
                case 'o':
                case 'O':
                    if(*ptr == 0) {
                        operand->cc = true;
                        operand->cc_index = CC_INDEX_PO;
                        return;
                    }
                    break;
                default:
                    break;
            }
            break;
        case 'r':
        case 'R':
            if(*ptr == 0) {
                operand->reg = R_R;
                operand->reg_index = R_INDEX_R;
                return;
            }
            break;
        case 's':
        case 'S':
            if((tolower(*ptr) == 'p') && ptr[1] == 0) {
                operand->reg = R_SP;
                operand->reg_index = R_INDEX_SP;
                return;
            }
            break;
        case 'z':
        case 'Z':
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
        if(operand->indirect) string++;
        operand->immediate = getValue(string, false);
        operand->immediate_provided = true;
    }
}

void parseLine(char *src) {
    uint8_t oplength = 0;
    uint8_t x;
    bool done;
    uint8_t state;
    uint8_t argcount = 0;
    streamtoken_t streamtoken;

    // default current line items
    memset(&currentline, 0, sizeof(currentline));
    memset(&operand1, 0, sizeof(operand_t));
    memset(&operand2, 0, sizeof(operand_t));

    state = PS_START;
    done = false;
    while(!done) {
        switch(state) {
            case PS_START:
                if((isspace(*src) == 0) && (*src) != '.') {
                    // LABEL or COMMENT
                    getLabelToken(&streamtoken, src);
                    switch(streamtoken.terminator) {
                        case ':':
                            state = PS_LABEL;
                            break;
                        case ';':
                            if(strlen(streamtoken.start) == 0) {
                                currentline.next = streamtoken.next;
                                state = PS_COMMENT;
                                break;
                            }
                            else {
                                error(message[ERROR_INVALIDLABEL]);
                                state = PS_ERROR;
                            }
                            break;
                        default:
                            error(message[ERROR_INVALIDLABEL]);
                            state = PS_ERROR;                        
                            break;
                    }
                    break;
                }
                // COMMAND or COMMENT
                x = getMnemonicToken(&streamtoken, src);
                if(x) {
                    state = PS_COMMAND;
                    break;
                }
                else {
                    if(streamtoken.terminator == 0) {
                        state = PS_DONE;
                        break;
                    }
                    if(streamtoken.terminator == ';') {
                        state = PS_COMMENT;
                        break;
                    }
                }
                break;
            case PS_LABEL:
                currentline.label = streamtoken.start;
                advanceLocalLabel();
                x = getMnemonicToken(&streamtoken, streamtoken.next);
                if(x) state = PS_COMMAND;
                else {
                    if(streamtoken.terminator == 0) {
                        state = PS_DONE;
                        break;
                    }
                    if(streamtoken.terminator == ';') {
                        state = PS_COMMENT;
                        currentline.next = streamtoken.next;
                        break;
                    }
                }
                break;
            case PS_COMMAND:
                if(streamtoken.start[0] == '.') {
                    // pseudo command
                    currentline.mnemonic = streamtoken.start + 1;
                }
                else parse_command(streamtoken.start);
                currentline.current_instruction = instruction_hashtable_lookup(currentline.mnemonic);
                if(currentline.current_instruction == NULL) {
                    // check if a defined macro exists, before erroring out
                    currentline.current_macro = findMacro(currentline.mnemonic);
                    if(currentline.current_macro) {
                        currentline.next = streamtoken.next;
                        state = PS_DONE;
                        break;
                    }
                    else {
                        error(message[ERROR_INVALIDMNEMONIC]);
                        state = PS_ERROR;
                        break;
                    }
                }
                if(currentline.current_instruction->type == ASSEMBLER) {
                    currentline.next = streamtoken.next;
                    state = PS_DONE;
                    break;
                }
                // Valid EZ80 instruction
                if(!recordingMacro) {
                    switch(streamtoken.terminator) {
                        case ';':
                            state = PS_COMMENT;
                            currentline.next = streamtoken.next;
                            break;
                        case 0:
                            currentline.next = NULL;
                            state = PS_DONE;
                            break;
                        default:
                            if(streamtoken.next) {
                                oplength = getOperandToken(&streamtoken, streamtoken.next);
                                if(oplength) {
                                    state = PS_OP1;
                                    break;
                                }
                            }
                            state = PS_DONE; // ignore any comments
                            break;
                    }
                } 
                else state = PS_DONE;
                break;
            case PS_OP1:
                argcount++;                
                if(currentExpandedMacro) {
                    oplength = macroExpandArg(_macro_OP_buffer, streamtoken.start, currentExpandedMacro);
                    if(oplength) {
                        streamtoken.start = _macro_OP_buffer;
                    }
                }
                if(argcount == 1) {
                    parse_operand(streamtoken.start, oplength, &operand1);
                }
                else {
                    parse_operand(streamtoken.start, oplength, &operand2);
                }
                switch(streamtoken.terminator) {
                    case ';':
                        currentline.next = streamtoken.next;
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
                        oplength = getOperandToken(&streamtoken, streamtoken.next);
                        if(oplength == 0) {
                            error(message[ERROR_MISSINGOPERAND]);
                            state = PS_ERROR;
                            break;
                        }
                        break;
                }
                break;
            case PS_COMMENT:
                currentline.comment = currentline.next;
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
    if(currentline.label == NULL) return;

    if(currentline.label[0] == '@') {
        if(currentline.label[1] == '@') {
            writeAnonymousLabel(num);
            return;
        }
        if(insertLocalLabel(currentline.label, num) == false) {
            error(message[ERROR_CREATINGLABEL]);
            return;
        }
        return;
    }
    if(currentline.label[0] == '$') {
        error(message[ERROR_INVALIDLABEL]);
        return;
    }
    str2num(currentline.label, strlen(currentline.label)); 
    if(!err_str2num) { // labels can't have a valid number format
        error(message[ERROR_INVALIDLABEL]);
        return;
    }
    if(insertGlobalLabel(currentline.label, num) == false){
        error(message[ERROR_CREATINGLABEL]);
        return;
    }

    writeLocalLabels();
    clearLocalLabels();
}

void refreshlocalLabels(void) {
    if((pass == 2) && (currentline.label) && (currentline.label[0] != '@')) {
        clearLocalLabels();
        readLocalLabels();
    }
}

// return ADL prefix bitfield, or 0 if none present
uint8_t getADLsuffix(void) {

    if(currentline.suffixpresent == false) return 0;

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
                    if(tolower(currentline.suffix[2]) == 's') return S_SIS; // SIS
                    if(tolower(currentline.suffix[2]) == 'l') return S_SIL; // SIL
                    // illegal suffix
                    break;
                case 'l':
                    if(tolower(currentline.suffix[2]) == 's') return S_LIS; // LIS
                    if(tolower(currentline.suffix[2]) == 'l') return S_LIL; // LIL
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
    return 0;
}

// get the number of bytes to emit from an immediate
uint8_t get_immediate_size(operand_t *op, uint8_t suffix) {
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
    if(num == 2) op->immediate &= 0xFFFF;
    return num;
}
// Emit a 16 or 24 bit immediate number, according to
// given suffix bit, or in lack of it, the current ADL mode
void emit_immediate(operand_t *op, uint8_t suffix) {
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

void prefix_ddfd_suffix(operandlist_t *op) {
    uint8_t prefix1, prefix2;

    if(!op->ddfdpermitted) return;

    prefix1 = get_ddfd_prefix(operand1.reg);
    prefix2 = get_ddfd_prefix(operand2.reg);

    // prefix in either of these two cases
    if(prefix1) {
        if(prefix2) {
            // both prefixes set
            if(operand1.indirect) output.prefix1 = prefix1;
            else output.prefix1 = prefix2;
        }
        else output.prefix1 = prefix1;
    }
    else if(prefix2) output.prefix1 = prefix2;
}

void transform_instruction(operand_t *op, permittype_t type) {
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
                rel = op->immediate - address - 2;
                if((rel > 127) || (rel < -128)) {
                    error(message[ERROR_RELATIVEJUMPTOOLARGE]);
                }
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

void emit_instruction(operandlist_t *list) {
    bool ddbeforeopcode; // determine position of displacement byte in case of DDCBdd/DDFDdd
    bool op1_displacement_required = false;
    bool op2_displacement_required = false;

    // Transform necessary prefix/opcode in output, according to given list and operands
    output.suffix = getADLsuffix();
    output.prefix1 = 0;
    output.prefix2 = list->prefix;
    output.opcode = list->opcode;

    if(pass == 1) definelabel(address);

    // Output displacement if needed, even when none is given (handles implicit cases)
    if(list->operandA > OPTYPE_R_AEONLY) op1_displacement_required = true;
    if(list->operandB > OPTYPE_R_AEONLY) op2_displacement_required = true;

    // issue any errors here
    if((list->transformA != TRANSFORM_REL) && (list->transformB != TRANSFORM_REL)) { // TRANSFORM_REL will mask to 0xFF
        if(((list->operandA == OPTYPE_N) || (list->operandA == OPTYPE_INDIRECT_N)) && ((operand1.immediate > 0xFF) || (operand1.immediate < -128))) error(message[WARNING_N_8BITRANGE]);
        if(((list->operandB == OPTYPE_N) || (list->operandB == OPTYPE_INDIRECT_N)) && ((operand2.immediate > 0xFF) || (operand2.immediate < -128))) error(message[WARNING_N_8BITRANGE]);
    }
    if((output.suffix) && ((list->adl & output.suffix) == 0)) error(message[ERROR_ILLEGAL_SUFFIXMODE]);
    if((op2_displacement_required) && ((operand2.displacement < -128) || (operand2.displacement > 127))) error(message[ERROR_DISPLACEMENT_RANGE]);

    // Specific checks
    if((list->operandA == OPTYPE_BIT) && (operand1.immediate > 7)) error(message[ERROR_INVALIDBITNUMBER]);
    if((list->operandA == OPTYPE_NSELECT) && (operand1.immediate > 2)) error(message[ERROR_ILLEGALINTERRUPTMODE]);
    if((list->transformA == TRANSFORM_N) && (operand1.immediate & 0x47)) error(message[ERROR_ILLEGALRESTARTADDRESS]);

    // prepare extra DD/FD suffix if needed
    prefix_ddfd_suffix(list);
    // Transform the opcode and potential immediate values, according to the current ruleset
    transform_instruction(&operand1, (permittype_t)list->transformA);
    transform_instruction(&operand2, (permittype_t)list->transformB);
    // determine position of dd
    ddbeforeopcode = (((output.prefix1 == 0xDD) || (output.prefix1 == 0xFD)) && (output.prefix2 == 0xCB) &&
                ((op1_displacement_required) || (op2_displacement_required)));
    
    // output adl suffix and any prefixes
    if(output.suffix > 0) emit_adlsuffix_code(output.suffix);
    if(output.prefix1) emit_8bit(output.prefix1);
    if(output.prefix2) emit_8bit(output.prefix2);

    // opcode in normal position
    if(!ddbeforeopcode) emit_8bit(output.opcode);
    
    // output displacement
    if(op1_displacement_required) emit_8bit(operand1.displacement & 0xFF);
    if(op2_displacement_required) emit_8bit(operand2.displacement & 0xFF);
    
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
        if(list_enabled || consolelist_enabled) listEmit8bit(value);
        io_outputc(value);
    }
    address++;
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

// emits a string surrounded by literal string quotes, as the token gets in from a file
// Only called when the first character is a double quote
void emit_quotedstring(char *str) {
    bool escaped = false;
    uint8_t escaped_char;

    str++; // skip past first "
    while(*str) {
        if(!escaped) {
            if(*str == '\\') { // escape character
                escaped = true;
            }
            else {
                if(*str == '\"') return;
                else emit_8bit(*str);
            }
        }
        else { // previously escaped
            escaped_char = get_escaped_char(*str);
            if(escaped_char == 0xff) {
                error(message[ERROR_ILLEGAL_ESCAPESEQUENCE]);
                return;
            }
            emit_8bit(escaped_char);
            escaped = false;
        }
        str++;
    }
    // we missed an end-quote to this string, we shouldn't reach this
    error(message[ERROR_STRING_NOTTERMINATED]);
}

void parse_asm_single_immediate(void) {
    streamtoken_t token;

    if(currentline.next) {
        if(getOperandToken(&token, currentline.next)) {
            operand1.immediate = getValue(token.start, true);
            operand1.immediate_provided = true;
            if((token.terminator != 0) && (token.terminator != ';')) error(message[ERROR_TOOMANYARGUMENTS]);
        }
        else error(message[ERROR_MISSINGOPERAND]);
    }
    else error(message[ERROR_MISSINGOPERAND]);
}

void handle_asm_db(void) {
    streamtoken_t token;
    bool expectarg = true;

    if(pass == 1) definelabel(address);

    while(currentline.next) {
        if(getDefineValueToken(&token, currentline.next)) {
            if(currentExpandedMacro) {
                if(macroExpandArg(_macro_ASM_buffer, token.start, currentExpandedMacro)) {
                    token.start = _macro_ASM_buffer;
                }
            }

            switch(token.start[0]) {
                case '\"':
                    emit_quotedstring(token.start);
                    break;
                default:
                    operand1.immediate = getValue(token.start, false); // not needed in pass 1
                    if(operand1.immediate > 0xff) error(message[WARNING_N_TOOLARGE]);
                    emit_8bit(operand1.immediate);
                    break;
            }
            expectarg = false;
        }
        if(token.terminator == ',') {
            currentline.next = token.next;
            expectarg = true;
        }
        else {
            if((token.terminator != 0) &&(token.terminator != ';')) error(message[ERROR_LISTFORMAT]);
            currentline.next = NULL; 
        }
    }
    if(expectarg) error(message[ERROR_MISSINGOPERAND]);
}

void handle_asm_dw(uint8_t wordtype) {
    label_t *lbl;
    streamtoken_t token;
    bool expectarg = true;

    if(pass == 1) definelabel(address);

    while(currentline.next) {
        if(getDefineValueToken(&token, currentline.next)) {

            if(currentExpandedMacro) {
                if(macroExpandArg(_macro_ASM_buffer, token.start, currentExpandedMacro)) {
                    token.start = _macro_ASM_buffer;
                }
            }
            lbl = findLabel(token.start);
            if(lbl) operand1.immediate = lbl->address;
            else operand1.immediate = getValue(token.start, false);
            switch(wordtype) {
                case ASM_DW:
                    if(operand1.immediate > 0xffffff) error(message[ERROR_ADLWORDSIZE]);
                    emit_16bit(operand1.immediate);
                    break;
                case ASM_DW24:
                    emit_24bit(operand1.immediate);
                    break;
                case ASM_DW32:
                    emit_8bit(operand1.immediate & 0xFF);
                    emit_24bit(operand1.immediate >> 8);
                    break;
                default:
                    error(message[ERROR_INTERNAL]);
                    break;
            }
            expectarg = false;
        }
        if(token.terminator == ',') {
            currentline.next = token.next;
            expectarg = true;
        }
        else {
            if((token.terminator != 0) && (token.terminator != ';')) error(message[ERROR_LISTFORMAT]);
            currentline.next = NULL;
        }
    }
    if(expectarg) error(message[ERROR_MISSINGOPERAND]);
}

void handle_asm_equ(void) {
    streamtoken_t token;

    if(currentline.next) {
        if(getDefineValueToken(&token, currentline.next)) {
            if((token.terminator != 0) && (token.terminator != ';')) error(message[ERROR_TOOMANYARGUMENTS]);
            if(pass == 1) {
                if(currentline.label) definelabel(getValue(token.start, true)); // needs to be defined in pass 1
                else error(message[ERROR_MISSINGLABEL]);
            }
        }
        else error(message[ERROR_MISSINGOPERAND]);
    }
    else error(message[ERROR_MISSINGOPERAND]);
}

void handle_asm_adl(void) {
    streamtoken_t token;

    if(currentline.next) {
        if(getDefineValueToken(&token, currentline.next) == 0) {
            error(message[ERROR_MISSINGOPERAND]);
            return;
        }
        if(currentExpandedMacro) {
            if(macroExpandArg(_macro_ASM_buffer, token.start, currentExpandedMacro)) {
                token.start = _macro_ASM_buffer;
            }
        }

        if(fast_strcasecmp(token.start, "adl")) {
            error(message[ERROR_INVALIDOPERAND]);
            return;
        }
        if(token.terminator == '=') {
            if(getDefineValueToken(&token, token.next)) {
                operand2.immediate = getValue(token.start, true); // needs to be defined in pass 1
                operand2.immediate_provided = true;
            }
            else error(message[ERROR_MISSINGOPERAND]);
        }        
        else error(message[ERROR_MISSINGOPERAND]);
    }
    else error(message[ERROR_MISSINGOPERAND]);


    if((operand2.immediate != 0) && (operand2.immediate != 1)) {
        error(message[ERROR_INVALID_ADLMODE]);
    }

    adlmode = operand2.immediate;
}

void handle_asm_org(void) {
    uint24_t newaddress;
    
    parse_asm_single_immediate(); // get address from next token
    // address needs to be given in pass 1
    newaddress = operand1.immediate;
    if((adlmode == 0) && (newaddress > 0xffff)) error(message[ERROR_ADDRESSRANGE]); 
    if(pass == 1) definelabel(address);
    address = newaddress;
}

void handle_asm_include(void) {
    streamtoken_t token;
    filestackitem fsi;
    uint8_t inclevel;
    
    if(!currentline.next) {
        error(message[ERROR_MISSINGOPERAND]);
        return;
    }
    inclevel = filestackCount();
    if(inclevel > FILESTACK_MAXFILES) {
        error(message[ERROR_MAXINCLUDEFILES]);
        return;
    }
    getDefineValueToken(&token, currentline.next);
    if(token.start[0] != '\"') {
        error(message[ERROR_STRINGFORMAT]);
        return;
    }

    token.start[strlen(token.start)-1] = 0;
    io_getCurrent(&fsi);
    filestackPush(&fsi);
    strcpy(fsi.filename, token.start+1);
    // set new file
    io_getFileDefaults(&fsi);
    fsi.fp = fopen(token.start+1, "rb");
    strncpy(fsi.filename, token.start+1, sizeof(fsi.filename));
    fsi.filename[sizeof(fsi.filename)-1] = '\0';
    fsi.bufferstart = &_incbuffer[inclevel][0];
    fsi.filebuffer = fsi.bufferstart;
    io_setCurrent(&fsi);
    
    if(filehandle[FILE_CURRENT] == 0) {
        error(message[ERROR_INCLUDEFILE]);
        filestackPop(&fsi);
    }
    lineNumberNeedsReset = true;
    if((token.terminator != 0) && (token.terminator != ';')) error(message[ERROR_TOOMANYARGUMENTS]);
}

void handle_asm_incbin(void) {
    streamtoken_t token;
    FILE* fh;
    uint24_t n, size = 0;

    if(recordingMacro) return;

    if(!currentline.next) {
        error(message[ERROR_MISSINGOPERAND]);
        return;
    }

    getDefineValueToken(&token, currentline.next);

    if(currentExpandedMacro) {
        if(macroExpandArg(_macro_ASM_buffer, token.start, currentExpandedMacro)) {
            token.start = _macro_ASM_buffer;
        }
    }

    if(token.start[0] != '\"') {
        error(message[ERROR_STRINGFORMAT]);
        return;
    }

    token.start[strlen(token.start)-1] = 0;
    fh = fopen(token.start+1, "rb");

    if(!fh) {
        error(message[ERROR_INCLUDEFILE]);            
        return;
    }

    if(pass == 1) {
        #ifdef CEDEV
            // Use optimal assembly routine in moscalls.asm
            size = getfilesize(fh->fhandle);
            address += size;
        #else
            while(1) {
                // Other non-agon compilers
                size = fread(_buffer, 1, FILE_BUFFERSIZE, fh);
                address += size;
                if(size < FILE_BUFFERSIZE) break;
            }
        #endif
    }

    if(pass == 2) {
        while(1) {
            size = fread(_buffer, 1, FILE_BUFFERSIZE, fh);
            if(list_enabled || consolelist_enabled) { // Output needs to pass to the listing through emit_8bit, performance-hit
                for(n = 0; n < size; n++) emit_8bit(_buffer[n]);
            }
            else {
                io_write(FILE_OUTPUT, _buffer, size);
                address += size;
            }
            if(size < FILE_BUFFERSIZE) break;
        }
        binfilecount++;
    }
    fclose(fh);
    if((token.terminator != 0) && (token.terminator != ';')) error(message[ERROR_TOOMANYARGUMENTS]);
}

void handle_asm_blk(uint8_t width) {
    uint24_t num;
    int24_t val = 0;
    streamtoken_t token;

    if(pass == 1) definelabel(address);

    if(!currentline.next) {
        error(message[ERROR_MISSINGOPERAND]);
        return;
    }

    if(getDefineValueToken(&token, currentline.next) == 0) {
        error(message[ERROR_MISSINGOPERAND]); // we need at least one value
        return;
    }

    if(currentExpandedMacro) {
        if(macroExpandArg(_macro_ASM_buffer, token.start, currentExpandedMacro)) {
            token.start = _macro_ASM_buffer;
        }
    }

    num = getValue(token.start, true); // <= needs a number of items during pass 1, otherwise addresses will be off later on

    if(token.terminator == ',') {
        if(getDefineValueToken(&token, token.next) == 0) {
            error(message[ERROR_MISSINGOPERAND]);
            return;
        }

        if(currentExpandedMacro) {
            if(macroExpandArg(_macro_ASM_buffer, token.start, currentExpandedMacro)) {
                token.start = _macro_ASM_buffer;
            }
        }
        val = getValue(token.start, false); // value not required in pass 1
    }
    else { // no value given
        if((token.terminator != 0)  && (token.terminator != ';'))
            error(message[ERROR_LISTFORMAT]);
        val = fillbyte;
    }
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

void handle_asm_align(void) {
uint24_t alignment;
uint24_t base;
uint24_t delta;

    parse_asm_single_immediate();
    if(operand1.immediate <= 0) {
        error(message[ERROR_INVALIDNUMBER]);
        return;
    }

    if((operand1.immediate & (operand1.immediate - 1)) != 0) {
        error(message[ERROR_POWER2]); 
        return;
    }
    
    alignment = operand1.immediate;
    base = (~(operand1.immediate - 1) & address);

    if(address & (operand1.immediate -1)) base += alignment;
    delta = base - address;
    while(delta--) emit_8bit(fillbyte);

    address = base;
    if(pass == 1) {
        definelabel(address); // set address to current line
    }
}

void handle_asm_endmacro(void) {
    if(pass == 1) {
        fclose(filehandle[FILE_MACRO]);
        filehandle[FILE_MACRO] = NULL;
    }
    recordingMacro = false;
}

void handle_asm_definemacro(void) {
    streamtoken_t token;
    uint8_t argcount = 0;
    char arglist[MACROMAXARGS][MACROARGLENGTH + 1];
    
    recordingMacro = true;

    if(pass == 2) return;

    // Only define macros in pass 1
    definelabel(address);

    // parse arguments into array
    if(!currentline.next) {
        error(message[ERROR_MACRONAME]);
        return;
    }
    if(getMnemonicToken(&token, currentline.next) == 0) { // terminate on space
        error(message[ERROR_MACRONAME]);
        return;
    }
    if(findMacro(currentline.mnemonic) != 0) {
        error(message[ERROR_MACRODEFINED]);
        return;
    }
    currentline.mnemonic = token.start;

    currentline.next = token.next;
    if((token.terminator == ' ') || (token.terminator == '\t')) {
        while(currentline.next) {
            if(argcount == MACROMAXARGS) error(message[ERROR_MACROARGCOUNT]);
            if(getDefineValueToken(&token, currentline.next)) {
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
    io_getMacroFilename(filename[FILE_MACRO], currentline.mnemonic);
    filehandle[FILE_MACRO] = fopen(filename[FILE_MACRO], "wb+");
    if(!filehandle[FILE_MACRO]) error("Error writing macro file");
}

void handle_asm_if(void) {
    streamtoken_t token;
    int24_t value;
    
    // No nested conditionals.
    if(inConditionalSection != 0) {
        error(message[ERROR_NESTEDCONDITIONALS]);
        return;
    }

    if(currentline.next) {
        if(getMnemonicToken(&token, currentline.next) == 0) { // terminate on space
            error(message[ERROR_CONDITIONALEXPRESSION]);
            return;
        }
        value = getValue(token.start, true);

        inConditionalSection = value ? 2 : 1;
    }
    else error(message[ERROR_MISSINGOPERAND]);
}

void handle_asm_else(void) {
    // No nested conditionals.
    if(inConditionalSection == 0) {
        error(message[ERROR_MISSINGIFCONDITION]);
        return;
    }
    inConditionalSection = inConditionalSection == 1 ? 2 : 1;
}

void handle_asm_endif(void) {
    if(inConditionalSection == 0) {
        error(message[ERROR_MISSINGIFCONDITION]);
        return;
    }
    inConditionalSection = 0;
}

void handle_asm_fillbyte(void) {

    parse_asm_single_immediate(); // get fillbyte from next token
    if((operand1.immediate < 0) || (operand1.immediate > 255)) error(message[ERROR_8BITRANGE]);
    fillbyte = operand1.immediate;
}

void handle_assembler_command(void) {
    if(!recordingMacro) {
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
            handle_asm_dw(ASM_DW);
            break;
        case(ASM_DW24):
            handle_asm_dw(ASM_DW24);
            break;
        case(ASM_DW32):
            handle_asm_dw(ASM_DW32);
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
        case(ASM_INCBIN):
            handle_asm_incbin();
            break;
        case(ASM_FILLBYTE):
            handle_asm_fillbyte();
            break;
        case(ASM_IF):
            handle_asm_if();
            break;
        case(ASM_ELSE):
            handle_asm_else();
            break;
        case(ASM_ENDIF):
            handle_asm_endif();
            break;
        }
        return;
    }
    if(currentline.current_instruction->asmtype == ASM_MACRO_END) handle_asm_endmacro();
    return;
}

void expandMacroStart(macro_t *exp) {    
    streamtoken_t token;
    uint8_t argcount = 0;
    filestackitem fsi;

    if(pass == 1) definelabel(address);

    currentExpandedMacro = currentline.current_macro;
    // parse arguments into given macro substitution space
    while(currentline.next) {
        if(getDefineValueToken(&token, currentline.next)) {
            argcount++;
            if(argcount > exp->argcount) {
                error(message[ERROR_MACROARGCOUNT]);
                return;
            }
            strcpy(exp->substitutions[argcount-1], token.start);
        }
        if(token.terminator == ',') currentline.next = token.next;
        else {
            if((token.terminator != 0) &&(token.terminator != ';')) error(message[ERROR_LISTFORMAT]);
            currentline.next = NULL; 
        }
    }
    if(argcount != exp->argcount) error(message[ERROR_MACROINCORRECTARG]);
    // push current file to the stack
    io_getCurrent(&fsi);
    filestackPush(&fsi);

    // set new file
    io_getFileDefaults(&fsi);
    io_getMacroFilename(fsi.filename, exp->name);    
    fsi.fp = fopen(fsi.filename, "rb");
    // set up temporary buffer for file reads from macro
    fsi.bufferstart = &_macrobuffer[0];
    fsi.filebuffer = fsi.bufferstart;
    io_setCurrent(&fsi);

    if(filehandle[FILE_CURRENT] == 0) {
        filestackPop(&fsi);
        error(message[ERROR_MACROFILEWRITE]);
    }
    lineNumberNeedsReset = true;
}

void processInstructions(char *macroline){
    operandlist_t *list;
    uint8_t listitem;
    bool match;

    if(pass == 1) {
        if(recordingMacro) {
            if((currentline.mnemonic == NULL) || fast_strcasecmp(currentline.mnemonic, "endmacro")) {

                io_puts(FILE_MACRO, macroline);
            }
            if((currentline.label) && (currentline.label[0] != '@')) error("No global labels allowed in macro definition");
        }
        else {
            if(currentline.mnemonic == NULL) {
                // check if there is a single label on a line in during pass 1
                if(pass == 1) definelabel(address);
                return;
            }
        }
    }

    if(currentline.current_instruction) {
        if(currentline.current_instruction->type == EZ80) {
            if(!recordingMacro) {
                if(inConditionalSection != 1) {
                    // process this mnemonic by applying the instruction list as a filter to the operand-set
                    list = currentline.current_instruction->list;
                    match = false;
                    for(listitem = 0; listitem < currentline.current_instruction->listnumber; listitem++) {
                        if(permittype_matchlist[list->operandA].match(&operand1) && permittype_matchlist[list->operandB].match(&operand2)) {
                        match = true;
                        // mnemonic index distribution optimization
                        emit_instruction(list);
                        break;
                        }
                        list++;
                    }
                    if(!match) error(message[ERROR_OPERANDSNOTMATCHING]);
                    return;
                }
            }
        }
        else handle_assembler_command();
    }
    if(currentline.current_macro) {
        expandMacroStart(currentline.current_macro);
    }
    return;
}

void passInitialize(uint8_t passnumber) {
    pass = passnumber;
    linenumber = 0;
    address = start_address;
    recordingMacro = false;
    currentExpandedMacro = NULL;
    inConditionalSection = 0;
    io_setpass(passnumber);
    filestackInit();
    initAnonymousLabelTable();
    io_resetCurrentInput();
}

// Assembler directives may demand a late reset of the linenumber, after the listing has been done
void processDelayedLineNumberReset(void) {
    if(lineNumberNeedsReset) {
        lineNumberNeedsReset = false;
        linenumber = 0;
    }
}

bool assemble(void){
    char line[LINEMAX]; // Temp line buffer, will be deconstructed during streamtoken_t parsing
    char errorline[LINEMAX]; // Full integrity copy of each line
    char macroline[LINEMAX]; // Temp line buffer for macro expansion
    filestackitem fsitem;
    bool incfileState;

    global_errors = 0;
    
    // Assemble in two passes
    // Pass 1
    printf("Pass 1...\r\n");
    passInitialize(1);
    do {
        while(io_getline(FILE_CURRENT, line)) {
            strcpy(errorline, line);
            linenumber++;
            if(recordingMacro) strcpy(macroline, line);
            parseLine(line);
            processInstructions(macroline);
            processDelayedLineNumberReset();
            if(global_errors) {
                vdp_set_text_colour(DARK_YELLOW);
                printf("%s\r\n",errorline);
                vdp_set_text_colour(BRIGHT_WHITE);
                return false;
            }
        }
        if(inConditionalSection != 0) error(message[ERROR_MISSINGENDIF]);

        if(filestackCount()) {
            currentExpandedMacro = NULL;
            fclose(filehandle[FILE_CURRENT]);
            incfileState = filestackPop(&fsitem);
            io_setCurrent(&fsitem);
        }
        else incfileState = false;
    } while(incfileState);
    writeLocalLabels();

    if(global_errors) return false;

    // Pass 2
    printf("Pass 2...\r\n");
    passInitialize(2);
    if(consolelist_enabled || list_enabled) {
        listInit();
    }
    readLocalLabels();
    readAnonymousLabel();
    
    do {
        while(io_getline(FILE_CURRENT, line)) {
            strcpy(errorline, line);
            linenumber++;
            if(consolelist_enabled || list_enabled) {
                listStartLine(line);
            }
            parseLine(line);
            refreshlocalLabels();
            processInstructions(macroline);
            if(consolelist_enabled || list_enabled) {
                listEndLine();
            }
            processDelayedLineNumberReset();
            if(global_errors) {
                vdp_set_text_colour(DARK_YELLOW);
                printf("%s\r\n",errorline);
                vdp_set_text_colour(BRIGHT_WHITE);
                return false;
            }    
        }
        sourcefilecount++;
        if(filestackCount()) {
            currentExpandedMacro = NULL;
            fclose(filehandle[FILE_CURRENT]);
            incfileState = filestackPop(&fsitem);
            io_setCurrent(&fsitem);
        }
        else incfileState = false;
    } while(incfileState);
    return true;
}

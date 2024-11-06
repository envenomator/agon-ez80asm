#include "assemble.h"
#include <time.h>

// Temp macro buffers
char _macrobuffer[MACRO_BUFFERSIZE];
char _macro_OP_buffer[LINEMAX]; // replacement buffer for operands during macro expansion
char _macro_ASM_buffer[LINEMAX];// replacement buffer for values during ASSEMBLER macro expansion

struct contentitem *filecontent[256]; // hash table with all file content items
struct contentitem *_contentstack[FILESTACK_MAXFILES];  // stacked content
uint8_t _contentstacklevel;

bool processContent(char *filename);
uint16_t getnextContentLine(struct contentitem *ci);
uint16_t getnextContentMacroLine(char *dst, struct contentitem *ci);
struct contentitem *findContent(char *filename);
struct contentitem *insertContent(char *filename);

// Parse a command-token string to currentline.mnemonic & currentline.suffix
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

// parses the given string to the operand, or throws errors along the way
// will destruct parts of the original string during the process
void parse_operand(char *string, uint8_t len, operand_t *operand) {
    char *ptr = string;

    operand->addressmode = NOREQ;
    operand->reg = R_NONE;

    // direct or indirect
    if(*ptr == '(') {
        operand->indirect = true;
        operand->addressmode |= INDIRECT;
        // find closing bracket or error out
        if(string[len-1] == ')') string[len-1] = 0; // terminate on closing bracket
        else error(message[ERROR_CLOSINGBRACKET],0);
        ptr = &string[1];
        while(isspace(*ptr)) ptr++; // eat spaces
    }
    else {
        operand->indirect = false;
        // should not find a closing bracket
        if(string[len-1] == ')') error(message[ERROR_OPENINGBRACKET],0);
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
                operand->addressmode |= CC;
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
                        operand->addressmode |= CC;
                        operand->cc_index = CC_INDEX_NC;
                        operand->addressmode |= CCA;
                        return;
                    }
                    break;
                case 'z':   // NZ
                case 'Z':
                    if(*ptr == 0) {
                        operand->cc = true;
                        operand->addressmode |= CC;
                        operand->cc_index = CC_INDEX_NZ;
                        operand->addressmode |= CCA;
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
                    operand->addressmode |= CC;
                    operand->cc_index = CC_INDEX_P;
                    return;
                case 'e':
                case 'E':
                    if(*ptr == 0) {
                        operand->cc = true;
                        operand->addressmode |= CC;
                        operand->cc_index = CC_INDEX_PE;
                        return;
                    }
                    break;
                case 'o':
                case 'O':
                    if(*ptr == 0) {
                        operand->cc = true;
                        operand->addressmode |= CC;
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
                operand->addressmode |= CC;
                operand->cc_index = CC_INDEX_Z;
                operand->addressmode |= CCA;
                return;
            }
            break;
        default:
            break;
    }
    
    if(*string) {
        if(operand->indirect) {
            len--;
            string++;
        }
        strcpy(operand->immediate_name, string);
        operand->immediate = getValue(string, false);
        operand->immediate_provided = true;
        operand->addressmode |= IMM;
    }
}

// FSM to parse each line into separate components, store in gbl currentline variable
void parseLine(char *src) {
    uint8_t oplength = 0;
    uint8_t x;
    bool done;
    bool asmcmd = false;
    uint8_t state;
    uint8_t argcount = 0;
    streamtoken_t streamtoken;

    // default current line items
    memset(&currentline, 0, sizeof(currentline));
    memset(&operand1, 0, (sizeof(operand_t) - sizeof(operand1.immediate_name) + 1));
    memset(&operand2, 0, (sizeof(operand_t) - sizeof(operand2.immediate_name) + 1));

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
                        default: // intentional fall-through
                            error(message[ERROR_INVALIDLABEL],0);
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
                advanceAnonymousLabel();
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
                    // should be an assembler command
                    asmcmd = true;
                    currentline.mnemonic = streamtoken.start;
                }
                else parse_command(streamtoken.start); // ez80 split suffix and set mnemonic for search

                currentline.current_instruction = instruction_lookup(currentline.mnemonic);
                if(currentline.current_instruction == NULL) {
                    if(!asmcmd) {
                        error(message[ERROR_INVALIDMNEMONIC],"%s",currentline.mnemonic);
                        state = PS_ERROR;
                        break;
                    }
                    // Check for assembler command
                    currentline.mnemonic = streamtoken.start + 1;
                    currentline.current_instruction = instruction_lookup(currentline.mnemonic);
                    if((currentline.current_instruction == NULL) ||
                       (currentline.current_instruction->type != ASSEMBLER)) {
                        error(message[ERROR_INVALIDMNEMONIC],"%s",currentline.mnemonic);
                        state = PS_ERROR;
                        break;
                    }
                    // Valid assembler command found (with a .)
                }
                switch(currentline.current_instruction->type) {
                    case EZ80:
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
                        break;
                    case ASSEMBLER:
                        currentline.next = streamtoken.next;
                        state = PS_DONE;
                        break;
                    case MACRO:
                        currentline.current_macro = currentline.current_instruction->macro;
                        currentline.current_instruction = NULL;
                        currentline.next = streamtoken.next;
                        state = PS_DONE;
                        break;
                }
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
                            error(message[ERROR_TOOMANYARGUMENTS],0);
                            state = PS_ERROR;
                            break;
                        }
                        oplength = getOperandToken(&streamtoken, streamtoken.next);
                        if(oplength == 0) {
                            error(message[ERROR_MISSINGOPERAND],0);
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

// Parse an immediate value from currentline.next
// services several assembler directives
void parse_asm_single_immediate(void) {
    streamtoken_t token;

    if(currentline.next) {
        if(getOperandToken(&token, currentline.next)) {
            operand1.immediate = getValue(token.start, true);
            operand1.immediate_provided = true;
            strcpy(operand1.immediate_name, token.start);
            if((token.terminator != 0) && (token.terminator != ';')) error(message[ERROR_TOOMANYARGUMENTS],0);
        }
        else error(message[ERROR_MISSINGOPERAND],0);
    }
    else error(message[ERROR_MISSINGOPERAND],0);
}

// Emits list data for the DB/DW/DW24/DW32 etc directives
void handle_asm_data(uint8_t wordtype) {
    int32_t value;
    streamtoken_t token;
    bool expectarg = true;

    definelabel(address);

    while(currentline.next) {
        if(getDefineValueToken(&token, currentline.next)) {
            if(currentExpandedMacro) {
                if(macroExpandArg(_macro_ASM_buffer, token.start, currentExpandedMacro)) {
                    token.start = _macro_ASM_buffer;
                }
            }

            if((token.start[0] == '\"') && (wordtype != ASM_DB)) {
                error(message[ERROR_STRING_NOTALLOWED],0);
                return;
            }

            switch(wordtype) {
                case ASM_DB:
                    switch(token.start[0]) {
                        case '\"':
                            emit_quotedstring(token.start);
                            break;
                        default:
                            value = getValue(token.start, false); // not needed in pass 1
                            if(pass == 2) validateRange8bit(value, token.start);
                            emit_8bit(value);
                            break;
                    }
                    break;
                case ASM_DW:
                    value = getValue(token.start, false);
                    if(pass == 2) validateRange16bit(value, token.start);
                    emit_16bit(value);
                    break;
                case ASM_DW24:
                    value = getValue(token.start, false);
                    if(pass == 2) validateRange24bit(value, token.start);
                    emit_24bit(value);
                    break;
                case ASM_DW32:
                    value = getValue(token.start, false);
                    emit_32bit(value);
                    break;
                default:
                    error(message[ERROR_INTERNAL],0);
                    break;
            }
            expectarg = false;
        }
        if(token.terminator == ',') {
            currentline.next = token.next;
            expectarg = true;
        }
        else {
            if((token.terminator != 0) && (token.terminator != ';')) error(message[ERROR_LISTFORMAT],0);
            currentline.next = NULL;
        }
    }
    if(expectarg) error(message[ERROR_MISSINGOPERAND],0);
}

void handle_asm_equ(void) {
    streamtoken_t token;

    if(currentline.next) {
        if(getDefineValueToken(&token, currentline.next)) {
            if((token.terminator != 0) && (token.terminator != ';')) error(message[ERROR_TOOMANYARGUMENTS],0);
            if(currentline.label) definelabel(getValue(token.start, true)); // needs to be defined in pass 1
            else error(message[ERROR_MISSINGLABEL],0);
        }
        else error(message[ERROR_MISSINGOPERAND],0);
    }
    else error(message[ERROR_MISSINGOPERAND],0);
}

void handle_asm_adl(void) {
    streamtoken_t token;

    if(currentline.next) {
        if(getDefineValueToken(&token, currentline.next) == 0) {
            error(message[ERROR_MISSINGOPERAND],0);
            return;
        }
        if(currentExpandedMacro) {
            if(macroExpandArg(_macro_ASM_buffer, token.start, currentExpandedMacro)) {
                token.start = _macro_ASM_buffer;
            }
        }

        if(fast_strcasecmp(token.start, "adl")) {
            error(message[ERROR_INVALIDOPERAND],0);
            return;
        }
        if(token.terminator == '=') {
            if(getDefineValueToken(&token, token.next)) {
                operand2.immediate = getValue(token.start, true); // needs to be defined in pass 1
                operand2.immediate_provided = true;
                strcpy(operand2.immediate_name, token.start);
            }
            else error(message[ERROR_MISSINGOPERAND],0);
        }        
        else error(message[ERROR_MISSINGOPERAND],0);
    }
    else error(message[ERROR_MISSINGOPERAND],0);


    if((operand2.immediate != 0) && (operand2.immediate != 1)) {
        error(message[ERROR_INVALID_ADLMODE],"%s", operand2.immediate_name);
    }

    adlmode = operand2.immediate;
}

void handle_asm_org(void) {
    uint24_t newaddress;
    
    parse_asm_single_immediate(); // get address from next token
    // address needs to be given in pass 1
    newaddress = operand1.immediate;
    if((adlmode == 0) && (newaddress > 0xffff)) {
        error(message[ERROR_ADDRESSRANGE],"%s", operand1.immediate_name); 
        return;
    }
    if((newaddress < address) && (address != start_address)) {
        error(message[ERROR_ADDRESSLOWER], 0);
        return;
    }
    definelabel(address);

    // Skip filling if this is the first .org statement
    if(address == start_address) {
        address = newaddress;
        return;
    }
    // Fill bytes on any subsequent .org statement
    while(address != newaddress) emit_8bit(fillbyte);
}

void handle_asm_include(void) {
    streamtoken_t token;

    if(!currentline.next) {
        error(message[ERROR_MISSINGOPERAND],0);
        return;
    }
    getDefineValueToken(&token, currentline.next);
    if(token.start[0] != '\"') {
        error(message[ERROR_STRINGFORMAT],0);
        return;
    }
    if((pass == 2) && (consolelist_enabled || list_enabled)) listEndLine();

    token.start[strlen(token.start)-1] = 0;
    if(strcmp(token.start+1, currentcontentitem->name) == 0) {
        error(message[ERROR_RECURSIVEINCLUDE],0);
        return;
    }
    processContent(token.start+1);

    if(pass == 1) sourcefilecount++;

    if((token.terminator != 0) && (token.terminator != ';')) error(message[ERROR_TOOMANYARGUMENTS],0);
}

void handle_asm_incbin(void) {
    streamtoken_t token;
    struct contentitem *ci;
    uint24_t n;

    if(!currentline.next) {
        error(message[ERROR_MISSINGOPERAND],0);
        return;
    }

    getDefineValueToken(&token, currentline.next);
    if(currentExpandedMacro) {
        if(macroExpandArg(_macro_ASM_buffer, token.start, currentExpandedMacro)) {
            token.start = _macro_ASM_buffer;
        }
    }
    if(token.start[0] != '\"') {
        error(message[ERROR_STRINGFORMAT],0);
        return;
    }
    token.start[strlen(token.start)-1] = 0;

    // Prepare content
    ci = findContent(token.start+1);
    if(ci == NULL) {
        if(pass == 1) {
            ci = insertContent(token.start+1);
            if(ci == NULL) return;
        }
        else return;
    }

    if(pass == 1) {
        address += ci->size;
    }
    if(pass == 2) {
        if(list_enabled || consolelist_enabled) { // Output needs to pass to the listing through emit_8bit, performance-hit
            for(n = 0; n < ci->size; n++) emit_8bit(ci->buffer[n]);
        }
        else {
            io_write(FILE_OUTPUT, ci->buffer, ci->size);
            address += ci->size;
        }
    }
    binfilecount++;
    if((token.terminator != 0) && (token.terminator != ';')) error(message[ERROR_TOOMANYARGUMENTS],0);
}

void handle_asm_blk(uint8_t width) {
    uint24_t num;
    int32_t val = 0;
    streamtoken_t token;

    definelabel(address);

    if(!currentline.next) {
        error(message[ERROR_MISSINGOPERAND],0);
        return;
    }

    if(getDefineValueToken(&token, currentline.next) == 0) {
        error(message[ERROR_MISSINGOPERAND],0); // we need at least one value
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
            error(message[ERROR_MISSINGOPERAND],0);
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
            error(message[ERROR_LISTFORMAT],0);
        val = fillbyte;
    }
    while(num) {
        switch(width) {
            case 0:
                address += num;
                remaining_dsspaces += num;
                num = 0;
                if(val != fillbyte) warning(message[WARNING_UNSUPPORTED_INITIALIZER],"%s",token.start);
                break;
            case 1:
                if(pass == 2) validateRange8bit(val, token.start);
                emit_8bit(val);
                num -= 1;
                break;
            case 2:
                if(pass == 2) validateRange16bit(val, token.start);
                emit_16bit(val);
                num -= 1;
                break;
            case 3:
                if(pass == 2) validateRange24bit(val, token.start);
                emit_24bit(val);
                num -= 1;
                break;
            case 4:
                emit_32bit(val);
                num -= 1;
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
        error(message[ERROR_ZEROORNEGATIVE],"%s",operand1.immediate_name);
        return;
    }

    if((operand1.immediate & (operand1.immediate - 1)) != 0) {
        error(message[ERROR_POWER2],"%s",operand1.immediate_name); 
        return;
    }
    
    alignment = operand1.immediate;
    base = (~(operand1.immediate - 1) & address);

    if(address & (operand1.immediate -1)) base += alignment;
    delta = base - address;
    while(delta--) emit_8bit(fillbyte);

    address = base;
    definelabel(address); // set address to current line
}

void handle_asm_definemacro(void) {
    streamtoken_t token;
    struct contentitem *ci;
    uint8_t argcount = 0;
    char arglist[MACROMAXARGS][MACROARGLENGTH + 1];
    char macroline[LINEMAX+1];
    char *strend;
    definelabel(address);
    bool foundend = false;
    macro_t *macro;
    uint16_t startlinenumber,linelength,macrolength;

    _macrobuffer[0] = 0; // empty string
    strend = _macrobuffer;

    if(pass == 2 && (consolelist_enabled || list_enabled)) listEndLine(); // print out first line of macro definition

    ci = currentcontentitem;

    startlinenumber = ci->currentlinenumber;
    macrolength = 0;
    while((linelength = getnextContentMacroLine(macroline, ci))) {
        ci->currentlinenumber++;
        char *src = macroline;

        if(pass == 2 && (consolelist_enabled || list_enabled)) {
            listStartLine(src, ci->currentlinenumber);
            listEndLine();
        }
        if((!isspace(src[0])) && (src[0] != '@') && (src[0] != ';')) {
            error(message[ERROR_MACRO_NOGLOBALLABELS],0);
            break;
        }
        // skip leading space
        while(*src && (isspace(*src))) src++;
        if(fast_strncasecmp(src, "macro", 5) == 0) {
            error(message[ERROR_MACROINMACRO],0);
            break;
        }
        uint8_t skipdot = (*src == '.')?1:0;
        if(fast_strncasecmp(src+skipdot, "endmacro", 8) == 0) { 
            if(isspace(src[8+skipdot]) || (src[8+skipdot] == 0) || (src[8+skipdot] == ';')) {
                foundend = true;
                break;
            }
        }
        // concatenate to buffer end
        if(pass == 1) {
            if((macrolength + linelength) > MACRO_BUFFERSIZE) {
                error(message[ERROR_MACROTOOLARGE],0);
                return;
            }
            char *tmp = macroline;
            while(*tmp) {
                *strend++ = *tmp++;
                macrolength++;
            }
            *strend = 0;
        }
    }
    if(!foundend) {
        error(message[ERROR_MACROUNFINISHED],0);
        return;
    }

    // Only define macros in pass 1
    // parse arguments into array
    if(pass == 1) {
        if(!currentline.next) {
            error(message[ERROR_MACRONAME],0);
            return;
        }
        if(getMnemonicToken(&token, currentline.next) == 0) { // terminate on space
            error(message[ERROR_MACRONAME],0);
            return;
        }
        currentline.mnemonic = token.start;

        currentline.next = token.next;
        if((token.terminator == ' ') || (token.terminator == '\t')) {
            while(currentline.next) {
                if(argcount == MACROMAXARGS) error(message[ERROR_MACROARGCOUNT],"%s", token.start);
                if(getDefineValueToken(&token, currentline.next)) {
                    strcpy(arglist[argcount], token.start);
                    argcount++;
                }
                if(token.terminator == ',') currentline.next = token.next;
                else {
                    if((token.terminator != 0) &&(token.terminator != ';')) error(message[ERROR_LISTFORMAT],0);
                    currentline.next = NULL; 
                }
            }
        }
        // record the macro to memory
        macro = defineMacro(currentline.mnemonic, argcount, (char *)arglist, startlinenumber);
        if(!macro) {
            error(message[ERROR_MACROMEMORYALLOCATION],0);
            return;
        }
        setMacroBody(macro, _macrobuffer);
    }
}

void handle_asm_if(void) {
    streamtoken_t token;
    int24_t value;
    
    // No nested conditionals.
    if(inConditionalSection != 0) {
        error(message[ERROR_NESTEDCONDITIONALS],0);
        return;
    }

    if(currentline.next) {
        if(getMnemonicToken(&token, currentline.next) == 0) { // terminate on space
            error(message[ERROR_CONDITIONALEXPRESSION],0);
            return;
        }
        value = getValue(token.start, true);

        inConditionalSection = value ? 2 : 1;
    }
    else error(message[ERROR_MISSINGOPERAND],0);
}

void handle_asm_else(void) {
    // No nested conditionals.
    if(inConditionalSection == 0) {
        error(message[ERROR_MISSINGIFCONDITION],0);
        return;
    }
    inConditionalSection = inConditionalSection == 1 ? 2 : 1;
}

void handle_asm_endif(void) {
    if(inConditionalSection == 0) {
        error(message[ERROR_MISSINGIFCONDITION],0);
        return;
    }
    inConditionalSection = 0;
}

void handle_asm_fillbyte(void) {
    parse_asm_single_immediate(); // get fillbyte from next token
    if((!ignore_truncation_warnings) && ((operand1.immediate < -128) || (operand1.immediate > 255))) {
        warning(message[WARNING_TRUNCATED_8BIT],"%s",operand1.immediate_name);
    }
    fillbyte = operand1.immediate;
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
            handle_asm_data(ASM_DB);
            break;
        case(ASM_DS):
            handle_asm_blk(0);
            break;
        case(ASM_DW):
            handle_asm_data(ASM_DW);
            break;
        case(ASM_DW24):
            handle_asm_data(ASM_DW24);
            break;
        case(ASM_DW32):
            handle_asm_data(ASM_DW32);
            break;
        case(ASM_ASCIZ):
            handle_asm_data(ASM_DB);
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
        case(ASM_MACRO_END):
            error(message[ERROR_MACRONOTSTARTED],0);
            break;
    }
    return;
}

// Process the instructions found at each line, after parsing them
void processInstructions(void){
    operandlist_t *list;
    uint8_t listitem;
    bool match;
    bool condmatch;
    bool regamatch, regbmatch;

    if(currentline.mnemonic == NULL) definelabel(address);
    
    if(currentline.current_instruction) {
        if(currentline.current_instruction->type == EZ80) {
            if(inConditionalSection != 1) {
                // process this mnemonic by applying the instruction list as a filter to the operand-set
                list = currentline.current_instruction->list;
                match = false;
                for(listitem = 0; listitem < currentline.current_instruction->listnumber; listitem++) {
                    regamatch = (list->regsetA & operand1.reg) || !(list->regsetA | operand1.reg);
                    regbmatch = (list->regsetB & operand2.reg) || !(list->regsetB | operand2.reg);

                    condmatch = ((list->conditionsA & MODECHECK) == operand1.addressmode) && ((list->conditionsB & MODECHECK) == operand2.addressmode);
                    if(list->flags & F_CCOK) {
                        condmatch |= operand1.cc;
                        regamatch = true;
                    }
                    if(regamatch && regbmatch && condmatch) {
                        match = true;
                        emit_instruction(list);
                        break;
                    }
                    list++;
                }
                if(!match) error(message[ERROR_OPERANDSNOTMATCHING],0);
                return;
            }
        }
        else handle_assembler_command();
    }
    return;
}

void processMacro(void) {
    streamtoken_t token;
    uint8_t argcount = 0;
    macro_t *exp = currentline.current_macro;
    char macroline[LINEMAX+1];
    char errorline[LINEMAX+1];
    bool macro_invocation_warning = false;

    // Check for defined label
    if(currentline.label) definelabel(address);

    // set for additional line-based parsing/processing of the macro
    currentExpandedMacro = currentline.current_macro;

    // get arguments
    while(currentline.next) {
        if(getDefineValueToken(&token, currentline.next)) {
            argcount++;
            if(argcount > exp->argcount) {
                error(message[ERROR_MACROINCORRECTARG],"%d provided, %d expected", argcount, exp->argcount);
                return;
            }
            strcpy(exp->substitutions[argcount-1], token.start);
        }
        if(token.terminator == ',') currentline.next = token.next;
        else {
            if((token.terminator != 0) &&(token.terminator != ';')) error(message[ERROR_LISTFORMAT],0);
            currentline.next = NULL; 
        }
    }
    if(argcount != exp->argcount) {
        error(message[ERROR_MACROINCORRECTARG],"%d provided, %d expected", argcount, exp->argcount);
        return;
    }
    // open macro storage
    resetnextline(exp->body);

    // process body
    macrolinenumber = 1;
    while(getnextline(macroline)) {
        strcpy(errorline, macroline);
        if(pass == 2 && (consolelist_enabled || list_enabled)) listStartLine(macroline, macrolinenumber);
        parseLine(macroline);
        processInstructions();
        if(pass == 2 && (consolelist_enabled || list_enabled)) listEndLine();
        macrolinenumber++;
        if(global_errors) {
            vdp_set_text_colour(DARK_YELLOW);
            trimRight(errorline);
            printf("%s\r\n",errorline);
            vdp_set_text_colour(BRIGHT_WHITE);
            return;
        }
        if(issue_warning) {
            macro_invocation_warning = true;
            vdp_set_text_colour(DARK_YELLOW);
            printf("%s\r\n",errorline);
            vdp_set_text_colour(BRIGHT_WHITE);        
            issue_warning = false;
        }
    }
    // end processing
    currentExpandedMacro = NULL;
    if(macro_invocation_warning) issue_warning = true; // display invocation warning at upstream caller
}

// Initialize pass 1 / pass2 states for the assembler
void passInitialize(uint8_t passnumber) {
    pass = passnumber;
    address = start_address;
    currentExpandedMacro = NULL;
    inConditionalSection = 0;
    initAnonymousLabelTable();
    _contentstacklevel = 0;
    if(pass == 1) {
        sourcefilecount = 1;
        binfilecount = 0;
    }
    if(pass == 2) {
        fseek(filehandle[FILE_ANONYMOUS_LABELS], 0, 0);
    }
    issue_warning = false;
    remaining_dsspaces = 0;
}

void initFileContentTable(void) {
    filecontentsize = 0;
    memset(filecontent, 0, sizeof(filecontent));
}

struct contentitem *insertContent(char *filename) {
    struct contentitem *ci, *try;
    uint8_t index;

    // Allocate memory and fill out ci content
    ci = allocateMemory(sizeof(struct contentitem));
    if(ci == NULL) return NULL;
    ci->name = allocateString(filename);
    if(ci->name == NULL) return NULL;
    ci->fh = io_openfile(filename, "rb");
    if(ci->fh == 0) return NULL;
    ci->size = io_getfilesize(ci->fh);
    ci->buffer = allocateMemory(ci->size+1);
    ci->readptr = ci->buffer;
    if(fread(ci->buffer, 1, ci->size, ci->fh) != ci->size) {
        error(message[ERROR_READINGINPUT],0);
        return NULL;
    }
    ci->buffer[ci->size] = 0; // terminate stringbuffer
    ci->filebuffered = filesbuffered;
    strcpy(ci->labelscope, ""); // empty scope
    ci->next = NULL;
    fclose(ci->fh);

    // Update statistics
    filecontentsize += sizeof(struct contentitem);
    filecontentsize += strlen(filename) + 1;
    filecontentsize += ci->size + 1;

    // Placement
    index = hash256(filename);
    try = filecontent[index];
    // First item on index
    if(try == NULL) {
        filecontent[index] = ci;
        return ci;
    }

    // Collision on index, place at end of linked list. Items are always unique
    while(true) {
        if(try->next) {
            try = try->next;
        }
        else {
            try->next = ci;
            return ci;
        }
    }
}

struct contentitem *findContent(char *filename) {
    uint8_t index;
    struct contentitem *try;

    index = hash256(filename);
    try = filecontent[index];

    while(true)
    {
        if(try == NULL) return NULL;
        if(strcmp(try->name, filename) == 0) return try;
        try = try->next;
    }
}
uint16_t getnextContentMacroLine(char *dst, struct contentitem *ci) {
    uint16_t len = 0;
    char *ptr = ci->readptr;

    while(*ptr) {
        *dst++ = *ptr;
        len++;
        if(*ptr++ == '\n') {
            break;
        }
    }
    ci->readptr = ptr;
    *dst = 0;
    return len;
}

uint16_t getnextContentLine(struct contentitem *ci) {
    uint16_t len = 0;
    char *dst1 = ci->currentline;
    char *dst2 = ci->currenterrorline;
    char *ptr = ci->readptr;

    while(*ptr) {
        if((len++ == LINEMAX) && (*ptr != '\n')) {
            error(message[ERROR_LINETOOLONG],0);
            return 0;
        }
        *dst1++ = *ptr;
        *dst2++ = *ptr;
        if(*ptr++ == '\n') {
            break;
        }
    }
    ci->readptr = ptr;
    *dst1 = 0;
    *dst2 = 0;
    return len;
}

uint8_t currentStackLevel(void) {
    return _contentstacklevel;
}

struct contentitem *contentPop(void) {
    struct contentitem *ci;
    if(_contentstacklevel) {
        ci = _contentstack[--_contentstacklevel];
        if(_contentstacklevel) currentcontentitem = _contentstack[_contentstacklevel - 1];
        else currentcontentitem = NULL;
        return ci;
    }
    else return NULL;
}

bool contentPush(struct contentitem *ci) {
    if(_contentstacklevel == FILESTACK_MAXFILES) {
        error(message[ERROR_MAXINCLUDEFILES],0);
        return false;
    }
    _contentstack[_contentstacklevel++] = ci;
    if(_contentstacklevel > maxstackdepth) maxstackdepth = _contentstacklevel;
    currentcontentitem = ci;
    return true;
}

struct contentitem *currentContent(void) {
    if(_contentstacklevel) {
        return _contentstack[_contentstacklevel-1];
    }
    else return NULL;
}

bool processContent(char *filename) {
    char line[LINEMAX+1];      // Temp line buffer, will be deconstructed during streamtoken_t parsing
    char errorline[LINEMAX+1]; // Full integrity copy of each line
    struct contentitem *ci;

    // Prepare content
    ci = findContent(filename);
    if(ci == NULL) {
        if(pass == 1) {
            ci = insertContent(filename);
            if(ci == NULL) return false;
        }
        else return false;
    }

    // Prepare processing
    ci->readptr = ci->buffer;
    ci->currentlinenumber = 0;
    ci->currentline = line;
    ci->currenterrorline = errorline;
    if(!contentPush(ci)) return false;

    // Process
    while(getnextContentLine(ci)) {
        ci->currentlinenumber++;
        if((pass == 2) && (consolelist_enabled || list_enabled)) listStartLine(line, ci->currentlinenumber);

        parseLine(line);

        if(!currentline.current_macro) {
            processInstructions();
            if((pass == 2) && (consolelist_enabled || list_enabled)) listEndLine();
        }
        else {
            if((pass == 2) && (consolelist_enabled || list_enabled)) listEndLine();
            processMacro();
            if(issue_warning) { // warnings from the expanded macro
                vdp_set_text_colour(DARK_YELLOW);
                printf("Invoked from \"%s\" line %d as\r\n", filename, ci->currentlinenumber);
                printf("%s\r\n",errorline);
                vdp_set_text_colour(BRIGHT_WHITE);
                issue_warning = false;
            }
        }
        if(global_errors) {
            if(currentExpandedMacro) {
                vdp_set_text_colour(DARK_RED);
                printf("Invoked from \"%s\" line %d as\r\n", filename, ci->currentlinenumber);
                currentExpandedMacro = NULL;
            }
            if(currentStackLevel() == errorreportlevel) {
                vdp_set_text_colour(DARK_YELLOW);
                printf("%s\r\n",errorline);
                vdp_set_text_colour(BRIGHT_WHITE);
            }
            contentPop();
            return false;
        }
        if(issue_warning) { // local-level warnings
            vdp_set_text_colour(DARK_YELLOW);
            printf("%s\r\n",errorline);
            vdp_set_text_colour(BRIGHT_WHITE);
            issue_warning = false;
        }
    }
    if(inConditionalSection != 0) {
        error(message[ERROR_MISSINGENDIF],0);
        contentPop();
        return false;
    }
    contentPop();
    strcpy(ci->labelscope, ""); // empty scope for next pass

    return true;
}

bool assemble(char *filename) {
    global_errors = 0;
    errorreportlevel = 0;

    maxstackdepth = 0;
    initFileContentTable();

    // Pass 1
    printf("Pass 1...\r\n");
    passInitialize(1);
    processContent(filename);
    if(global_errors) return false;

    // Pass 2
    printf("Pass 2...\r\n");
    passInitialize(2);
    readAnonymousLabel();
    if(consolelist_enabled || list_enabled) listInit();
    processContent(filename);
    if(global_errors) return false;

    return true;
}

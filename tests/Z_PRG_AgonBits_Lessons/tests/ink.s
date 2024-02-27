;   Example of MOSlet

    .assume adl=1   
    .org $0B0000    ; NOTE different assemble address for MOSlets

    jp start        ; We jump over the header

    .align 64      
    .db "MOS"       
    .db 00h         
    .db 01h       

app_name:      .db     "ink.bin", 0        ; The executable name, only used in arg1
max_args:      EQU     16                  ; Maximum number of arguments allowed in argv
arg_pointer:   .ds     max_args * 3, 0     ; max 16 x 3 bytes each
num_args:      .db     0                   ; the number of arguments entered

; ---------------------------------------------
;
;   INITIAL SETUP CODE HERE
;
; ---------------------------------------------

start:              
    push af                             ; Push all registers to the stack
    push bc
    push de
    push ix
    push iy

    ld  IX, arg_pointer                 ; The argv array pointer address
    push IX
    call PARSE_PARAMS                   ; Parse the parameters
    ld a,c                              ; C contains the number of params entered
    ld (num_args),a                     ; store number of arguments entered
    pop IX                              ; IX: argv  

    cp 2  
    jr c, error                         ; if less than 2 args, then exit
    ld de, (ix+3)                       ; address of first arg, is a 0 terminated string for the file

    call STRING2INT                     ; turn string into a integer we can use
                                        ; hl = result, de = pointer to ASCII number
                                        ; no error checking, so watch out!
                                        ; lower byte L will contain the integer we want to use
    ld a, 17
    rst.lil $10                         ; change ink colour to...
    ld a, l 
    rst.lil $10                         ; the value we got back in L

now_exit:

    pop iy                              ; Pop all registers back from the stack
    pop ix
    pop de
    pop bc
    pop af
    ld hl,0                             ; Load the MOS API return code (0) for no errors.

    ret                                 ; Return MOS

error:
    ld hl, errorString                  ; location of string
    call printString
    jp now_exit

printString:                            ; print zero terminated string
    ld a,(hl)
    or a
    ret z
    RST.LIL 10h
    inc hl
    jr printString

errorString:
    .db "ERROR - no value entered\n\r",0

; ---------------------------------------------
;
;   PARAM PARSING ROUTINE WRITTEN BY OTHERS
;
; ---------------------------------------------

; Parse the parameter string into a C style array
; Parameters
; - HL: Address of parameter string
; - IX: Address for array pointer storage
; Returns:
; -  C: Number of parameters parsed

PARSE_PARAMS:      
    ld BC, app_name
    ld (IX+0), BC                       ; ARGV[0] = the executable name
    inc IX
    inc IX
    inc IX
    call skip_spaces                    ; Skip HL past any leading spaces

    ld BC, 1                            ; C: ARGC = 1 - also clears out top 16 bits of BCU
    ld B, max_args - 1                  ; B: Maximum number of arg_pointer

parse_step_2:    
    push BC                             ; Stack ARGC    
    push HL                             ; Stack start address of token
    call get_token                      ; Get the next token
    ld A, C                             ; A: Length of the token in characters
    pop DE                              ; Start address of token (was in HL)
    pop BC                              ; ARGC
    or  A                               ; Check for A=0 (no token found) OR at end of string
    ret Z

    ld  (IX+0), DE                      ; Store the pointer to the token
    push HL                             ; DE=HL
    pop DE
    call skip_spaces                    ; And skip HL past any spaces onto the next character
    Xor A
    ld (DE), A                          ; Zero-terminate the token
    inc IX
    inc IX
    inc IX                              ; Advance to next pointer position
    inc C                               ; Increment ARGC
    ld A, C                             ; Check for C >= A
    cp B
    jr C, parse_step_2                  ; And loop
    RET

; ---------------------------------------------

; Skip spaces in the parameter string
; Parameters:
; - HL: Address of parameter string
; Returns:
; - HL: Address of next none-space character
;    F: Z if at end of string, otherwise NZ if there are more tokens to be parsed

skip_spaces:       
    ld A, (HL)                  ; Get the character from the parameter string   
    cp ' '                      ; Exit if not space
    ret NZ 
    inc HL                      ; Advance to next character
    jr skip_spaces              ; Increment length


; ---------------------------------------------

; Get the next token
; Parameters:
; - HL: Address of parameter string
; Returns:
; - HL: Address of first character after token
; -  C: Length of token (in characters)

get_token:     
    ld C, 0                     ; Initialise length
token_loop:
    ld A, (HL)                  ; Get the character from the parameter string
    or A                        ; Exit if 0 (end of parameter string in MOS)
    ret Z
    cp 13                       ; Exit if CR (end of parameter string in BBC BASIC)
    ret Z
    cp ' '                      ; Exit if space (end of token)
    ret Z
    inc HL                      ; Advance to next character
    inc C                       ; Increment length
    jr token_loop


; ---------------------------------------------
;
;   STRING TO INTEGER ROUTINE WRITTEN BY OTHERS
;
; ---------------------------------------------

; Takes pointer to a string of ascii representing an integer and converts to 3 byte integer
; hl = result
; de = pointer to ASCII number

STRING2INT:
  Ld hl,0
s2i_loop: 
  ld a,(de)
  Sub 48
  Jr c,s2i_done
  Cp 10
  Jr nc,s2i_done
  Push hl
  Pop bc
  Add hl,hl                     ; x2
  Add hl,hl                     ; x4
  Add hl,bc                     ; x5
  Add hl,hl                     ; x10
  Ld bc,0
  Ld c,a
  Add hl,bc                     ; Add digit
  Inc de                        ; go to next number
  Jr s2i_loop
s2i_done:
  ret 

; ---------------------------------------------





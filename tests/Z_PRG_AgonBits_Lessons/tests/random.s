    ; extra MACRO files need to go here
    include "myMacros.inc"

    .assume adl=1                       ; ez80 ADL memory mode
    .org $40000                         ; load code here

    jp start_here                       ; jump to start of code

    .align 64                           ; MOS header
    .db "MOS",0,1     

    ; extra code files need to go here, or later
    include "debug.asm" 

start_here:
            
    push af                             ; store all the registers
    push bc
    push de
    push ix
    push iy

; ------------------
; This is our actual code

    ld hl, VDUdata                      ; address of string to use
    ld bc, endVDUdata - VDUdata         ; length of string
    rst.lil $18                         ; display message


LOOP_HERE:                              ; loop here until we hit ESC or SPACE key
                        
    MOSCALL $1E                         ; load IX with keymap address
    ld a, (ix + $0E)    
    bit 0, a                            ; index $0E, bit 0 is for ESC key in matrix
    jp nz, EXIT_HERE                    ; if pressed, ESC key to exit
                
    ld a, (ix + $0C)    
    bit 2, a                            ; index $0C bit 2 is for SPACE key in matrix
    call nz, DO_RANDOM                  ; if pressed, SPACE key to exit

    jr LOOP_HERE
    
; ------------------

DO_RANDOM:
    ld a, (ix + $0C)    
    bit 2, a                             ; index $0C bit 2 is for SPACE key in matrix
    jp nz, DO_RANDOM                     ; if pressed, SPACE key to exit

    call PRINT_RANDOM1
    call PRINT_RANDOM2

    ret 

PRINT_RANDOM1: 
    MOSCALL $08                          ; load IX with sysvars address
    ld a, (ix + $0)                      ; get first byte of clock counter

    ld b, 0
    ld c, 4
    call printHexA                       ; display it

    ret  

PRINT_RANDOM2:
    ; LD A, 0 is created here with two bytes, $3E $00
    .db $3E                             ; start of LD A, number
randSeed:
    .db $00                             ; 2nd part of LD A, number
    push bc 

    ld c,a
    add a,a
    add a,c
    add a,a
    add a,a
    add a,c
    add a,83
    ld (randSeed),a
    pop bc

    ld b, 0
    ld c, 6
    call printHexA

    ret 

; ------------------
; This is the data we send to VDP

VDUdata:
    .db     22,8                                ; set screen to MODE 8, this will also clear screen
    .db     31, 4, 4                            ; TAB to 10,20
    .db     "Clock based randomness"                  ; print this text

    .db     31, 4, 6                            ; TAB to 10,20
    .db     "Random from a seed value"          ; print this text

    .db     31, 0, 0                            ; TAB to 10,20
    .db     "Press SPACE for new number\r\n"    ; print this text
    .db     "or ESC to exit\r\n"                ; print this text
endVDUdata:

; ------------------
; This is where we exit the program

EXIT_HERE:

    pop iy                              ; Pop all registers back from the stack
    pop ix
    pop de
    pop bc
    pop af
    ld hl,0                             ; Load the MOS API return code (0) for no errors.
    ret                                 ; Return to MOS


























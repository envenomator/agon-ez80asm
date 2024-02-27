    include "myMacros.inc"

    .assume adl=1                       ; ez80 ADL memory mode
    .org $40000                         ; load code here

    jp start_here                       ; jump to start of code

    .align 64                           ; MOS header
    .db "MOS",0,1     

start_here:
            
    push af                             ; store all the registers
    push bc
    push de
    push ix
    push iy

; ------------------
; This is our actual code

    CLS 
    SET_COLOUR red
    SET_BG_COLOUR green
    TAB_TO 15,10

; Sending a VDU byte stream

    ld hl, VDUdata                      ; address of string to use
    ld bc, endVDUdata - VDUdata         ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP 

; reset the colours

    SET_COLOUR bright_white
    SET_BG_COLOUR black

; ------------------
; This is where we exit the program

    pop iy                              ; Pop all registers back from the stack
    pop ix
    pop de
    pop bc
    pop af
    ld hl,0                             ; Load the MOS API return code (0) for no errors.
    ret                                 ; Return to MOS

; ------------------
; This is the data we send to VDP

VDUdata:
    .db     "Hello Agon macros\r\n"     ; print this text
endVDUdata:

; ------------------
; colour data

red:            equ     1
green:          equ     2
blue:           equ     4
white:          equ     7
black:          equ     0
bright_white:   equ     15















































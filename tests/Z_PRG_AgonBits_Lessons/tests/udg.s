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

; Setup UDG character

    ld hl, udgData                      ; address of string to use
    ld bc, endUgdData - udgData         ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP 

; prepare the screen

    SET_MODE 8                          ; mode 8 is 640x480 pixels, 64 colours
    SET_COLOUR bright_red               ; colours are define at end of this code
    TAB_TO 5,10                         

    ld a, alien 
    rst.lil $10                         ; print our UDG

    SET_COLOUR blue
    SET_BG_COLOUR white
    TAB_TO 10,15

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
; UGD data
alien:    equ     128

udgData:
    .db     23, alien            ; define UDG character number
    .db     10000010b            ; binary data 0
    .db     01111100b            ; binary data 1
    .db     10101010b            ; binary data 2
    .db     10000010b            ; binary data 3
    .db     11000110b            ; binary data 4
    .db     00111000b            ; binary data 5
    .db     01000100b            ; binary data 6
    .db     10000010b            ; binary data 7

endUgdData:

; ------------------
; This is the text data we send to VDP

VDUdata:
    .db     "Hello Agon UDGs"     ; print this text
    .db     13,10                 ; CR, LF
endVDUdata:

; ------------------
; colour data

bright_red:     equ     9
green:          equ     2
blue:           equ     4
white:          equ     7
black:          equ     0
bright_white:   equ     15















































    .assume adl=1                       ; ez80 ADL memory mode
    .org $40000                         ; load code here

    jp start_here                       ; jump to start of code

    .align 64                           ; MOS header
    .db "MOS",0,1     

    include "myMacros.inc"

start_here:
            
    push af                             ; store all the registers
    push bc
    push de
    push ix
    push iy

; ------------------
; This is our actual code
    CLS                                 ; just CLS

    ld hl, VDUdata                      ; address of string to use
    ld bc, endVDUdata - VDUdata         ; length of string
    rst.lil $18                         ; display message
 
    ld hl, textBuffer                   ; HL needs to point to where text will be stored
    ld bc, 10                           ; BC is maximum nunber of chars
    ld e, 1                             ; 1 to clear buffer, 0 not to clear
    MOSCALL $09                         ; call $09 mos_editline

    cp 27                               ; A gives return code: 27 ESC, 13 CR
    jr z, hitEscape

hitEnter:
    ld hl, answer                       ; HL is location of answer to print
    jr printAnswer

hitEscape:
    ld hl, escaped                      ; lHL is ocation of 'escaped' text to print
    jr printAnswer

printAnswer:                    
    SET_BG_COLOUR 0                     ; reset background colour to black
    TAB_TO 0, 14                        ; go to TAB position

    ld bc,0
    ld a,0
    rst.lil $18                         ; print out text stored at HL, terminated in 0

    TAB_TO 0, 16                        ; go to TAB position

; ------------------
; This is where we exit the program

EXIT_HERE:

    ld a, 17
    rst.lil $10
    ld a, 128
    rst.lil $10                         ; BG to black

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
    .db     31, 0, 10                  ; TAB to 10,20
    .db     "Please enter your name:"  ; print this text
    .db     31, 0, 12                  ; TAB to 10,20
    .db     17, 128+4                  ; background colour
    .ds     10,32                      ; a line of 10 spaces
    .db     31, 0, 12                  ; TAB to 10,20
endVDUdata:

answer:         .db "You typed: "
textBuffer:     .ds 10,0
lineOfSpaces:   .ds 10,32

escaped:        .db "You escaped...",0





















































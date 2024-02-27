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

; Sending VDU commands byte by byte

    ld a, 22
    rst.lil $10                         ; set screen mode...
    ld a, 8
    rst.lil $10                         ; to mode 8, 320x240 64 colours


; Sending a VDU byte stream

    ld hl, VDUdata                      ; address of string to use
    ld bc, endVDUdata - VDUdata         ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP 

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
    .db     17, 0                       ; set text colour, 0 = black, 1 = red, etc
    .db     17, 11 + 128                ; set text background, +128 for background
    .db     31, 10, 10                  ; TAB to 10,20
    .db     "Hello Agon coders\r\n"     ; print this text
    .db     17, 15                      ; reset text colour, 15 = bright white
    .db     17, 0 + 128                 ; reset text background, +128 for background
endVDUdata:




































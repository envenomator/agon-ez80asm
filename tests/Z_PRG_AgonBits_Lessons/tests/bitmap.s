    ; extra MACRO files need to go here
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

; prepare the screen

    SET_MODE 8                          ; mode 8 is 640x480 pixels, 64 colours


; Sending a VDU byte stream

    ld hl, VDUdata                      ; address of string to use
    ld bc, endVDUdata - VDUdata         ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP 

    ld a, $08                           ; code to send to MOS
    rst.lil $08                         ; get IX pointer to System Variables

WAIT_HERE:                              ; loop here until we hit ESC key
    ld a, (ix + $05)                    ; get ASCII code of key pressed
    cp 27                               ; check if 27 (ascii code for ESC)   
    jp z, EXIT_HERE                     ; if pressed, jump to exit

    jr WAIT_HERE

; ------------------
; This is where we exit the program

EXIT_HERE:

    CLS 
    pop iy                              ; Pop all registers back from the stack
    pop ix
    pop de
    pop bc
    pop af
    ld hl,0                             ; Load the MOS API return code (0) for no errors.
    ret                                 ; Return to MOS

; ------------------
; This is the data we send to VDP

crystal:    EQU     0                   ; used for bitmap ID number
star:       EQU     1                   ; used for bitmap ID number

VDUdata:
    .db 23, 0, 192, 0                   ; set to non-scaled graphics

    ; LOAD THE BITMAP FROM A FILE
    ; file must be 24bit colour plus 8 bit alpha, byte order: RGBA
    ; 16x16 pixels RGBA should be 1,024 bytes in size

    .db 23, 27, 0, crystal              ; select bitmap 0 - crystal
    .db 23, 27, 1                       ; load bitmap data...
    .dw 16, 16                          ; of size 16x16, from file:
    incbin     "crystal.data"

    .db 23, 27, 0, star                 ; select bitmap 1 - star
    .db 23, 27, 1                       ; load bitmap data...
    .dw 16, 16                          ; of size 16x16, from file:
    incbin     "star.data"

    ; PRINT THE BITMAP ON THE SCREEN VDP v1.04 onwards
    .db 23, 27, 0, crystal              ; select bitmap 0 - crystal
    .db 23, 27, 3                       ; draw selected bitmap at...
    .dw 80, 50                          ; X, Y on the screen

    ; PLOT THE BITMAP ON THE SCREEN VDP v2.1.0 onwards
    .db 23, 27, 0, star                 ; select bitmap 0 - crystal
    .db 25, $ED                         ; plot absolute selected bitmap at...
    .dw 150, 120                         ; X, Y on the screen

endVDUdata:

; ------------------
  



















































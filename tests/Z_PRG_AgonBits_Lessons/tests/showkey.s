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

    CLS 

    ld a, 23                            ; HIDE CURSOR
    rst.lil $10
    ld a, 1
    rst.lil $10
    ld a,0                              ; VDU 23,1,0 = hide the text cursor
    rst.lil $10                         ; VDU 23,1,1 = show the text cursor                         
                
                                        ; From 'LESSON 03'
    ld a, $08                           ; code to send to MOS...
    rst.lil $08                         ; get IX pointer to System Variables

WAIT_HERE:                              ; loop here until we hit ESC key
    ld a, (ix + $05)                    ; get ASCII code of LAST key pressed
    cp 27                               ; check if 27 (ascii code for ESC)   
    jp z, EXIT_HERE                     ; if pressed, jump to EXIT_HERE

    ld b,10                             ; set the X position
    ld c,5                              ; set the Y position
    call printHexA                      ; display HEX value of A, at TAB B, C

    jr WAIT_HERE

; ------------------
; This is where we exit the program

EXIT_HERE:

    ld a, 23                            ; SHOW CURSOR
    rst.lil $10
    ld a, 1
    rst.lil $10
    ld a,1                              ; VDU 23,1,0 = hide the text cursor
    rst.lil $10                         ; VDU 23,1,1 = show the text cursor 

    CLS  
    pop iy                              ; Pop all registers back from the stack
    pop ix
    pop de
    pop bc
    pop af
    ld hl,0                             ; Load the MOS API return code (0) for no errors.
    ret                                 ; Return to MOS






































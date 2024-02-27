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

    ld d, 0                             ; Set D, our counter

LOOP_HERE:                              ; loop here until we hit ESC or SPACE key
                        
    MOSCALL $1E                         ; load IX with keymap address
    ld a, (ix + $0E)    
    bit 0, a                            ; index $0E, bit 0 is for ESC key in matrix
    jp nz, EXIT_HERE                    ; if pressed, ESC key to exit
                
    ld a, 00100000b                     ; put a bit value into A. Only set ONE bit
    call multiPurposeDelay              ; call the delay routine


    ld a, d                             ; store the counter in A

    ld b, 0                             ; x position in B
    ld c, 6                             ; y position in C
    call printHexA                      ; print A to screen in HEX

    inc d

    jr LOOP_HERE                        ; go round the loop



; ------------------
; This is the data we send to VDP

VDUdata:
    .db     22,8                         ; set screen to MODE 8, this will also clear screen
    .db     31, 0, 0                     ; TAB to 10,20
    .db     "Clock based delay"          ; print this text

    .db     31, 0, 2                     ; TAB to 10,20
    .db     "Press ESC to exit\r\n"      ; print this text
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






; ------------------
; delay routine

; routine waits a fixed time, then returns
; arrive with A =  the delay byte. One bit to be set only.
; eg. ld A, 00000100b

multiPurposeDelay:                      
    push bc                 
    ld b, a 
    MOSCALL $08                 ; get IX pointer to sysvars

waitLoop:

    ld a, (ix + 0)              ; ix+0h is lowest byte of clock timer

                                ;   we check if bit set is same as last time we checked.
                                ;   bit 0 - don't use
                                ;   bit 1 - changes 64 times per second
                                ;   bit 2 - changes 32 times per second
                                ;   bit 3 - changes 16 times per second

                                ;   bit 4 - changes 8 times per second
                                ;   bit 5 - changes 4 times per second
                                ;   bit 6 - changes 2 times per second
                                ;   bit 7 - changes 1 times per second
    and b 
    ld c,a 
    ld a, (oldTimeStamp)
    cp c                        ; is A same as last value?
    jr z, waitLoop              ; loop here if it is
    ld a, c 
    ld (oldTimeStamp), a        ; set new value

    pop bc
    ret

oldTimeStamp:   .db 00h






























    ld a, 00100000b                     ; put a bit value into A. Only set ONE bit
    call multiPurposeDelay              ; call the delay routine

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

    SET_MODE 8                          ; mode 8 is 320x240 pixels, 64 colours


; Sending a VDU byte stream

    ld hl, VDUdata                      ; address of string to use
    ld bc, endVDUdata - VDUdata         ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP 

    ld a, $08                           ; code to send to MOS
    rst.lil $08                         ; get IX pointer to System Variables

WAIT_HERE:                              ; loop here until we hit ESC key

    MOSCALL $1E                         ; load IX with keymap address
    ld a, (ix + $0E)    
    bit 0, a                            ; index $0E, bit 0 is for ESC key in matrix
    jp nz, EXIT_HERE                    ; if pressed, ESC key to exit
                
    ld a, (ix + $06)    
    bit 0, a                            ; index $06 bit 0 is for '1' key in matrix
    call nz, setFrame0                  ; if pressed, setFrame1

    ld a, (ix + $06)    
    bit 1, a                            ; index $06 bit 1 is for '2' key in matrix
    call nz, setFrame1                  ; if pressed, setFrame2

    ld a, (ix + $0A)    
    bit 4, a                            ; index $0A bit 4 is for 'h' key in matrix
    call nz, hideSprite                 ; if pressed, setFrame2

    ld a, (ix + $0A)    
    bit 1, a                            ; index $0A bit 1 is for 's' key in matrix
    call nz, showSprite                 ; if pressed, setFrame2

    jp WAIT_HERE

; ------------------

setFrame0:
    ld hl, set0                         ; address of string to use
    ld bc, endset0 - set0               ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP 
    ret 

set0:
    .db 23, 27, 4, 0                    ; select sprite 0 
    .db 23, 27, 10, 0                   ; select frame 0
endset0:

; ------------------

setFrame1:
    ld hl, set1                         ; address of string to use
    ld bc, endset1 - set1               ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP
    ret 

set1:
    .db 23, 27, 4, 0                    ; select sprite 0 
    .db 23, 27, 10, 1                   ; select frame 1
endset1:

; ------------------

hideSprite:
    ld hl, hide                         ; address of string to use
    ld bc, endhide - hide               ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP
    ret 

hide:
    .db 23, 27, 4, 0                    ; select sprite 0
    .db 23, 27, 12                      ; hide current sprite
endhide:

; ------------------

showSprite:
    ld hl, show                         ; address of string to use
    ld bc, endshow - show               ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP
    ret 

show:
    .db 23, 27, 4, 0                    ; select sprite 0
    .db 23, 27, 11                      ; show current sprite
endshow:


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
our_sprite: EQU     0                   ; sprite ID - always start at 0 upwards

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


    ; SETUP THE SPRITE

    .db 23, 27, 4, our_sprite           ; select sprite 0
    .db 23, 27, 5                       ; clear frames
    .db 23, 27, 6, crystal              ; add bitmap frame crystal to sprite
    .db 23, 27, 6, star                 ; add bitmap frame star to sprite
    .db 23, 27, 7, 1                    ; activate 1 sprite(s)
    .db 23, 27, 11                      ; show current sprite

    ; MOVE A SPRITE

    .db 23, 27, 4, our_sprite           ; select sprite 0 
    .db 23, 27, 13                      ; move currrent sprite to...
    .dw 150, 100                        ; x,y (as words)

    .db 23, 27, 15                      ; update sprites in GPU


    ; PLOT A RECTANGLE

    .db 18, 0, 45                       ; set graphics colour: mode (0), colour 45 = purple

    .db 25, 69                          ; PLOT: mode (69 is a point in current colour),
    .dw 80,80                           ; X; Y;

    .db 25, 101                         ; PLOT: mode (101 is a filled rectangle),
    .dw 190,130                         ; X; Y;

endVDUdata:

; ------------------
  



















































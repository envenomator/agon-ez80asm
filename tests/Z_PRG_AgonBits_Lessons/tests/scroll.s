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

; ------------------

LOOP_HERE:                              ; loop here until we hit ESC key

    MOSCALL $1E                         ; load IX with keymap address

    ld a, (ix + $0E)    
    bit 0, a                            ; index $0E, bit 0 is for ESC key in matrix
    jp nz, EXIT_HERE                    ; if pressed, ESC key to exit

    MOSCALL $1E                         ; load IX with keymap address                
    ld a, (ix + $03)    
    bit 1, a                            ; index $06 bit 0 is for 'left' key in matrix
    call nz, scroll_left                ; if pressed, setFrame1

    MOSCALL $1E                         ; load IX with keymap address
    ld a, (ix + $0F)    
    bit 1, a                            ; index $06 bit 0 is for 'left' key in matrix
    call nz, scroll_right               ; if pressed, setFrame1

    MOSCALL $1E                         ; load IX with keymap address
    ld a, (ix + $07)    
    bit 1, a                            ; index $06 bit 0 is for 'left' key in matrix
    call nz, scroll_up                  ; if pressed, setFrame1

    MOSCALL $1E                         ; load IX with keymap address
    ld a, (ix + $05)    
    bit 1, a                            ; index $06 bit 0 is for 'left' key in matrix
    call nz, scroll_down                ; if pressed, setFrame1

    jp LOOP_HERE

; ------------------

scroll_left:
    ld a, 1
    ld (scrollDirection), a
    call doScroll
    ret

scroll_right:
    ld a, 0
    ld (scrollDirection), a
    call doScroll
    ret

scroll_up:
    ld a, 3
    ld (scrollDirection), a
    call doScroll
    ret

scroll_down:
    ld a, 2
    ld (scrollDirection), a
    call doScroll
    ret

; ------------------

doScroll:
    ld hl, scrollData                           ; address of string to use
    ld bc, endScrollData - scrollData           ; length of string
    rst.lil $18                                 ; Call the MOS API to send data to VDP

    ld a, 00010000b
    call multiPurposeDelay


    ret 

; Scroll the screen
; VDU 23, 7, extent, direction, speed

scrollData:
    .db 23, 7, 2                                ; extent: 0 = text viewport, 1 = whole screen
                                                ; 2 = graphics viewport
scrollDirection: .db 0                          ; direction: 0 right, 1 left, 2 down, 3 up
    .db 1                                       ; speed = number of pixels
endScrollData:


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

VDUdata:
    .db 23, 0, 192, 0                   ; set to non-scaled graphics

    .db 17, 128  +7                     ; background text black
    .db 12                              ; cls

    .db 24                              ; set graphics viewport:-
    .dw 40, 150, 270, 40                ; 24, left; bottom; right; top;

    .db 17, 128                      ; background text grey
    .db 12                              ; cls
    ; FOR A SINGLE PIXEL PLOT

    .db 18, 0, bright_red               ; set graphics colour: mode (0), colour

    .db 25, 69                          ; PLOT: mode (69 is a point in current colour),
    .dw 200,80                          ; X; Y;

    ; FOR A LINE

    .db 18, 0, bright_magenta           ; set graphics colour: mode (0), colour

    .db 25, 69                          ; PLOT: mode (69 is a point in current colour),
    .dw 300, 60                         ; X; Y;

    .db 25, 13                          ; PLOT: mode (13 is a line),
    .dw 250,130                         ; X; Y;

    ; FOR A RECTANGLE

    .db 18, 0, green                    ; set graphics colour: mode (0), colour

    .db 25, 69                          ; PLOT: mode (69 is a point in current colour),
    .dw 10,120                          ; X; Y;

    .db 25, 101                         ; PLOT: mode (101 is a filled rectangle),
    .dw 100,180                         ; X; Y;


   ; FOR A CIRCLE   

    .db 18, 0, bright_yellow            ; set graphics colour: mode (0), colour

    .db 25, 68                          ; PLOT: mode (69 is a MOVE TO but don't plot point),
    .dw 180,140                         ; X; Y;

    .db 25, 149                         ; PLOT: mode (149 is an outlined circle),
    .dw 200,160                         ; X; Y;

    ; FOR A FILLED TRIANGLE

    .db 18, 0, blue                     ; set graphics colour: mode (0), colour

    .db 25, 69                          ; PLOT: mode (69 is a point in current colour),
    .dw 10,10                           ; X; Y;

    .db 25, 69                          ; PLOT: mode (69 is a point in current colour),
    .dw 50, 100                         ; X; Y;

    .db 25, 85                          ; PLOT: mode (85 is a filled triangle),
    .dw 200,20                          ; X; Y;


endVDUdata:

; ------------------
; colour data

bright_red:     equ     9
green:          equ     2
bright_yellow:  equ     11
bright_magenta: equ     13
blue:           equ     4
white:          equ     7
black:          equ     0
bright_white:   equ     15

; ------------------

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









































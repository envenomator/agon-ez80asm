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

; prepare the screen

    SET_MODE 8                          ; mode 8 is 640x480 pixels, 64 colours


; Sending a VDU byte stream to create some graphics

    ld hl, VDUdata                      ; address of string to use
    ld bc, endVDUdata - VDUdata         ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP 


    MOSCALL $08                         ; set IX pointer to the sysvars

    ; define which pixel to check       ; NOTE pixel positions are WORDS 
                                        ; so we need to define two bytes
    call resetReplyFlag                 ; make sure Flag is reset
    call doPixelQuery                   ; send the query to find pixel colour
    call waitForFlagUpdate              ; wait for p flag to be updated

                                        ; NOTE (ix + 0A),(ix + 0B),(ix + 0C) contain R,B,G colour values
    ld a, (ix + $16)                    ; (ix + $16) contains the palette index of the colour

    ld b, 0                             ; set B to X pos
    ld c, 0                             ; set C to Y pos
    call printHexA                      ; print out what is in A at X, Y


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
; This is the query we send to VDP

doPixelQuery:                           ; routine sends request to VDP for pixel colour
    ld hl, vduQuery
    ld bc, endvduQuery - vduQuery
    rst.lil $18
    ret 

vduQuery:
                .db     23,0,$84        ; get pixel of colour at X, Y
vduX:           .dw     200             ; X is a word
vduY:           .dw     80              ; Y s a word
endvduQuery:

; ---------------------------------------------

resetReplyFlag:

    ld a, (ix + $04)                    ; get flag for point
    res 2,a                             ; reset the P flag
    ld (ix + $04), a                    ; store flag status
    ret  

waitForFlagUpdate:

    ld a, (ix + $04)                    ; plflags variable
    bit 2,a                             ; bit2 is result from a point query
    jr z, waitForFlagUpdate             ; if not updated, then wait here

    ret 

; ---------------------------------------------

VDUdata:
    .db 23, 0, 192, 0                   ; set to non-scaled graphics

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
; colour definitions - palette index colours

bright_red:     equ     9
green:          equ     2
bright_yellow:  equ     11
bright_magenta: equ     13
blue:           equ     4
white:          equ     7
black:          equ     0
bright_white:   equ     15















































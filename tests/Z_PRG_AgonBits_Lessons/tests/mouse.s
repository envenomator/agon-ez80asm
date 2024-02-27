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

    ; enable mouse command
    ld a, 23
    rst.lil $10
    ld a, 0
    rst.lil $10
    ld a, 89h
    rst.lil $10
    ld a, 0                             ; 0=enable, 1=disable
    rst.lil $10

    ; set mouse icon style
    ld a, 23
    rst.lil $10
    ld a, 0
    rst.lil $10
    ld a, 89h
    rst.lil $10
    ld a, 3                             ; setCursor
    rst.lil $10
    ld a, 5                             ; fab-gl style number, or my own defined ones
    rst.lil $10
    ld a, 0                             ; 0 as 16 bit number
    rst.lil $10

; Sending an initialising VDU byte stream

    ld hl, VDUdata                      ; address of string to use
    ld bc, endVDUdata - VDUdata         ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP 

; ------------------

MOUSE_IS_UP_LOOP:                       ; loop here until we hit ESC key

    MOSCALL $1E                         ; load IX with keymap address
    ld a, (ix + $0E)    
    bit 0, a                            ; index $0E, bit 0 is for ESC key in matrix
    jp nz, EXIT_HERE                    ; if pressed, ESC key to exit
                

    MOSCALL $08                         ; get IX pointer to sysvars
    ld a, (ix + $2D)                    ; + $2D is mouse buttons status
    and 00000001b                       ; bit 0 is left mouse button
    cp 1                                ; check if pressed
    jp z, DO_MOUSE_DOWN                 ; if so, then jump to mouse down routines

    jp MOUSE_IS_UP_LOOP

; ---------------

DO_MOUSE_DOWN:
    ; set start position
    ld a, (ix + $29)                    ; mouse x position
    ld (mouse_start_x), a 
    ld a, (ix + $2A)                    ; mouse x position
    ld (mouse_start_x + 1), a 

    ld a, (ix + $2B)                    ; mouse y position
    ld (mouse_start_y), a 
    ld a, (ix + $2C)                    ; mouse y position
    ld (mouse_start_y + 1), a 

    call plotStart

    jp MOUSE_STILLDOWN_LOOP

; ------------------

plotStart:
    ld hl, plot                         ; address of string to use
    ld bc, endPlot - plot               ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP 
    ret 

plot:
    .db 25, 69                          ; PLOT point at...
mouse_start_x:  .dw 0                   ; x
mouse_start_y:  .dw 0                   ; y

endPlot:

; ------------------

MOUSE_STILLDOWN_LOOP:

    MOSCALL $1E                         ; load IX with keymap address
    ld a, (ix + $0E)    
    bit 0, a                            ; index $0E, bit 0 is for ESC key in matrix
    jp nz, EXIT_HERE                    ; if pressed, ESC key to exit 

    MOSCALL $08                         ; get IX pointer to sysvars
    ld a, (ix + $2D)                    ; +$2D is mouse buttons status
    and 00000001b
    cp 0
    jp z, MOUSE_IS_UP_LOOP

    ld a, (ix + $29)                    ; mouse x position
    ld (mouse_pos_x), a 
    ld a, (ix + $2A)                    ; mouse x position
    ld (mouse_pos_x + 1), a 

    ld a, (ix + $2B)                    ; mouse y position
    ld (mouse_pos_y), a 
    ld a, (ix + $2C)                    ; mouse y position
    ld (mouse_pos_y + 1), a 

    call drawLine

    jp MOUSE_STILLDOWN_LOOP

; ------------------

drawLine:
    ld hl, line                         ; address of string to use
    ld bc, endLine - line               ; length of string
    rst.lil $18                         ; Call the MOS API to send data to VDP 
    ret 

line:
    .db 25, 5                          ; PLOT line from last position to...
mouse_pos_x:  .dw 0                    ; x
mouse_pos_y:  .dw 0                    ; y

endLine:


; ------------------
; This is where we exit the program

EXIT_HERE:

    CLS 

    ; disable mouse command
    ld a, 23
    rst.lil $10
    ld a, 0
    rst.lil $10
    ld a, 89h
    rst.lil $10
    ld a, 1                             ; 0=enable, 1=disable
    rst.lil $10

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

endVDUdata:

; ------------------
  
; Cursor styles

; 0 CursorPointerAmigaLike  
; 11x11 Amiga like colored mouse pointer

; 1 CursorPointerSimpleReduced  
; 10x15 mouse pointer

; 2 CursorPointerSimple     
; 11x19 mouse pointer

; 3 CursorPointerShadowed   
; 11x19 shadowed mouse pointer

; 4 CursorPointer   
; 12x17 mouse pointer

; 5 CursorPen   
; 16x16 pen

; 6 CursorCross1    
; 9x9 cross

; 7 CursorCross2    
; 11x11 cross

; 8 CursorPoint     
; 5x5 point

; 9 CursorLeftArrow     
; 11x11 left arrow

; 10 CursorRightArrow   
; 11x11 right arrow

; 11 CursorDownArrow    
; 11x11 down arrow

; 12 CursorUpArrow  
; 11x11 up arrow

; 13 CursorMove     
; 19x19 move

; 14 CursorResize1  
; 12x12 resize orientation 1

; 15 CursorResize2  
; 12x12 resize orientation 2

; 16 CursorResize3  
; 11x17 resize orientation 3

; 17 CursorResize4  
; 17x11 resize orientation 4

; 18 CursorTextInput    
; 7x15 text input


















































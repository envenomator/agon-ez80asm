    .assume adl=1       ; ez80 ADL memory mode
    .org $40000         ; load code here

    jp start_here       ; jump to start of code

    .align 64           ; MOS header
    .db "MOS",0,1     


start_here:
            
    push af             ; store all the registers
    push bc
    push de
    push ix
    push iy

; ------------------
; This is our actual code

    ld hl, string       ; address of string to use
    ld bc,0             ; length of string, or 0 if a delimiter is used
    ld a,0              ; A is the delimiter 
    rst.lil $18         ; Call the MOS API to send data to VDP 

; ------------------
; This is where we exit the program

    pop iy              ; Pop all registers back from the stack
    pop ix
    pop de
    pop bc
    pop af
    ld hl,0             ; Load the MOS API return code (0) for no errors.    
    ret                 ; Return to MOS

; ------------------

string:
    .db "Hello Agon World\r\n",0 




































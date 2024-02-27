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

    ld hl, VDUdata                      ; address of string to use
    ld bc, endVDUdata - VDUdata         ; length of string
    rst.lil $18                         ; display message

LOOP_HERE:                              ; loop here until we hit ESC or SPACE key
                        
    ld a, $08
    rst.lil $08                         ; get IX pointer to sysvars 
    ld a, (ix + $05)
    cp 32                               ; 32 is ascii code for SPACE   
    jp z, EXIT_HERE                     ; if pressed, SPACE key to exit
                
    ld a, $1E
    rst.lil $08                         ; get IX pointer to keyvals matrix of pressed keys
    ld a, (ix + $0E)    
    bit 0, a                            ; index $0E, bit 0 is for ESC key in matrix
    jp nz, EXIT_HERE                    ; if pressed, ESC key to exit

    jr LOOP_HERE

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
; This is the data we send to VDP

VDUdata:
    .db     22,4                        ; set screen to MODE 8, this will also clear screen
    .db     31, 10, 10                  ; TAB to 10,20
    .db     "Press ESC to exit\r\n"     ; print this text
endVDUdata:



























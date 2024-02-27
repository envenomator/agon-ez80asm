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
                                        ; try loading the file
    ld hl, filename                     ; file name
    ld de, lastKey                      ; were to load data
    ld bc, 1                            ; num bytes to read

    MOSCALL $01                         ; try to open and read byte from file

    ld hl, startText                    ; print initial text
    call printString

    ld a, (lastKey)
    rst.lil $10                         ; print last key saved to file

    ld hl, newline                      ; print initial text
    call printString

wait_for_keyup:                         ; wait for key up state so we only do it once
    MOSCALL $08                         ; get IX pointer to sysvars
    ld a, (ix + 18h)                    ; get key state
    cp 0                                ; are keys up, none currently pressed?
    jr nz, wait_for_keyup               ; loop if there is a key still pressed


LOOP_HERE:                              ; loop here until we press a key
                        
    MOSCALL $08                         ; get IX pointer to sysvars
    ld a, (ix + 18h)                    ; get key state, 0 or 1
    cp 0                                ; are keys up, none currently pressed?
    jr z, LOOP_HERE                     ; nothing is currently pressed

                                        ; a key was pressed, so fetch it
    MOSCALL $08                         ; get IX pointer to sysvars
    ld a, (ix + 05h)                    ; ix+5h is 'last key pressed'
    ld (lastKey),a                      ; store the key value ready to save

                                        ; we need to delete the old file first
    ld hl, filename                     ; file name
    MOSCALL $05                         ; delete call    
                   
    ld a, 00010000b                     ; wait a moment for SD to catch up
    call multiPurposeDelay
                                        ; save new file
    ld hl, filename                     ; file name
    ld de, lastKey                      ; data to save
    ld bc, 1                            ; num bytes to save

    MOSCALL $02                         ; save the new file, then EXIT
    
; ------------------
; This is where we exit the program

EXIT_HERE:

    ld hl, exitText
    call printString

    pop iy                              ; Pop all registers back from the stack
    pop ix
    pop de
    pop bc
    pop af
    ld hl,0                             ; Load the MOS API return code (0) for no errors.
    ret                                 ; Return to MOS


; ------------------------------------
; DATA AND FUNCTIONS
; ------------------------------------

filename:   .db "savedata.ini",0        ; prefs file storing high score
lastKey:    .db 32

; ------------------
; This is the text we send to VDP

startText:
    .db     "Last time you pressed: ",0         ; print this text

newline:
    .db     "\r\nPress another key...\r\n",0 

exitText:
    .db     "Thanks, Goodbye\r\n",0                     ; print this text

; ------------------

printString:                            ; print zero terminated string
    ld a,(hl)
    or a
    ret z
    RST.LIL 10h
    inc hl
    jr printString

; ------------------

multiPurposeDelay:              ; routine to wait for a given time
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























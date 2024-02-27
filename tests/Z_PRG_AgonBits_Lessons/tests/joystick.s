	; AGON CONSOLE8
	; Joystick Lesson
	; Richard Turnnidge 2024

	.assume adl=1		; big memory mode
	.org $40000		; load code here

	jp start_here		; jump to start of code

	.align 64		; MOS header
	.db "MOS",0,1

; ---------------------------------------------
;
;	INITIAL SETUP CODE HERE
;
; ---------------------------------------------

	include "myMacros.inc"
	include "debug.asm"

start_here:
	; store everything as good practice

	push af
	push bc
	push de
	push ix
	push iy

showTitle:
	ld hl, title_str		; data to send
	ld bc, end_title - title_str	; lenght of data
	rst.lil $18

; ---------------------------------------------
;
;	MAIN LOOP
;
; ---------------------------------------------

MAIN_LOOP:

get_key_input:
	MOSCALL $08		; get IX pointer to sysvars
	ld a, (ix + 05h)	; ix+5h is 'last key pressed'

	cp 27			; is it ESC key?
	jp z, exit_here		; if so exit cleanly


check_joystick:
				; needed to clear flags, else it didn't seem to work
	ld a,0			; set A to 0
	or a 			; clear flags

	in a, (portC)		; grab current io value of port C

	ld b, 13		; BC is XY of print position
	ld c, 5
	call printBin		; display value of port C in BINARY

	ld a,0			; set A to 0
	or a 			; clear flags

	in a, (portD)		; grab current io value of port D

	ld b, 13 		; BC is XY of print position
	ld c, 7
	call printBin

	in a, (portD)		; grab current io value of port D
	bit 5, a
	call nz, sayUp

	in a, (portD)		; grab current io value of port D
	bit 5, a
	call z, sayDown


	jp MAIN_LOOP

portC: 	EQU 	$9E
portD: 	EQU 	$A2

sayDown:
	TAB_TO 1,9
	ld hl, downTxt
	ld bc,0
	ld a,0
	rst.lil $18
	ret 

sayUp:
	TAB_TO 1,9
	ld hl, upTxt
	ld bc,0
	ld a,0
	rst.lil $18
	ret  

downTxt:
	.db "Button DOWN",0

upTxt:
	.db "Button UP  ",0



; ---------------------------------------------
;
;	EXIT CODE CLEANLY
;
; ---------------------------------------------

exit_here:

	CLS
				; reset all values before returning to MOS
	pop iy
	pop ix
	pop de
	pop bc
	pop af
	ld hl,0

	ret			; return to MOS here

; ---------------------------------------------
;
;	DATA & STRINGS
;
; ---------------------------------------------

title_str:
	.db	12 		; CLS
	.db	31,1,1		; TAB to 0,0
	.db "Joystick Lesson z80 assembler"	; text to show

	.db	31,8,3		; TAB to 0,0
	.db "Bits:76543210"	; text to show

	.db	31,13,4		; TAB to 0,0
	.db "--------"	; text to show

	.db	31,1,5		; TAB to 0,0
	.db "eZ80 Port C:"	; text to show

	.db	31,1,7		; TAB to 0,0
	.db "eZ80 Port D:"	; text to show

end_title:
















































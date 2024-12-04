;	Nested Macros test for Jeroen
; 	Richard Turnnidge 2024

; ---------------------------------------------
;
;	MACROS
;
; ---------------------------------------------

	macro MOSCALL afunc
	ld a, afunc
	rst.lil $08
	endmacro

	macro TAB_TO_XY x, y
	ld a, 31
	rst.lil $10
	ld a, x
	rst.lil $10
	ld a, y
	rst.lil $10
	endmacro

	macro SETCOLOUR colour
	ld a, 17
	rst.lil $10
	ld a, colour
	rst.lil $10
	endmacro

	macro PRINTCAPTION_AT_XY caption, colour, x, y
	SETCOLOUR colour
	TAB_TO_XY x, y 
	ld hl, caption
	call printString
	endmacro

	macro CLEARSCREEN
	ld a, 12
	rst.lil $10								; CLS
	endmacro

	macro HIDE_CURSOR
	ld a, 23
	rst.lil $10
	ld a, 1
	rst.lil $10
	ld a,0
	rst.lil $10								;VDU 23,1,0
	endmacro

; ---------------------------------------------
;
;	INITIALISE
;
; ---------------------------------------------

	.assume adl=1						; big memory mode
	.org $40000							; load code here

	jp start_here						; jump to start of code

	.align 64							; MOS header
	.db "MOS",0,1

; ---------------------------------------------
;
;	INITIAL SETUP CODE HERE
;
; ---------------------------------------------

start_here:
										; store everything as good practice	
	push af								; pop back when we return from code later
	push bc
	push de
	push ix
	push iy


	CLEARSCREEN

	HIDE_CURSOR							; hide the cursor

	PRINTCAPTION_AT_XY caption1, 1, 10,10
	PRINTCAPTION_AT_XY caption2, 2, 15,20
	PRINTCAPTION_AT_XY caption3, 3, 0,30
	PRINTCAPTION_AT_XY caption4, 4, 18,5

; ---------------------------------------------
;
;	MAIN LOOP
;
; ---------------------------------------------

MAIN_LOOP:	
	MOSCALL $08							; get IX pointer to sysvars
	ld a, (ix + 05h)					; ix+5h is 'last key pressed'
	cp 27								; is it ESC key?
	jp z, exit_here						; if so exit cleanly


	jp MAIN_LOOP

; ---------------------------------------------
;
;	EXIT CODE CLEANLY
;
; ---------------------------------------------

exit_here:

										; reset all values before returning to MOS
	pop iy
	pop ix
	pop de
	pop bc
	pop af
	ld hl,0

	ret									; return to MOS here

; ---------------------------------------------
;
;	OTHER ROUTINES	
;
; ---------------------------------------------

printString:                			; print zero terminated string
    ld a,(hl)
    or a
    ret z
    RST.LIL 10h
    inc hl
    jr printString

; ---------------------------------------------
;
;	TEXT AND DATA	
;
; ---------------------------------------------

caption1: 	.asciz "Hello World"
caption2: 	.asciz "ez80asm rocks"
caption3: 	.asciz "we love feature creep"
caption4: 	.asciz "Agon is cool"

LINEFEED:   .asciz "\r\n",0





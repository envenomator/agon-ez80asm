; ---------------------------------------------
;
;	DEBUG ROUTINES - Richard Turnnidge 2023
;
; ---------------------------------------------
	
printHexA:					; print A to screen as HEX byte pair at pos B,C

	ld (originalA), a		; store A for later

	ld a, 31				; TAB at x,y
	rst.lil $10
	ld a, b					; x=B
	rst.lil $10
	ld a, c					; y=C
	rst.lil $10				; put tab at BC position

step1:
	ld a, (originalA)		; get A, then split into two nibbles
	and 11110000b			; get higher nibble
	rra
	rra
	rra
	rra						; move across to lower nibble
	add a,48				; increase to ascii code range 0-9
	cp 58					; is A less than 10? (58+)
	jr c, step2				; carry on if less, else...
	add a, 7				; add 7 to get A-F char if larger than 10

step2:	
	rst.lil $10				; print the A char

	ld a, (originalA)		; get A back again
	and 00001111b			; now just get lower nibble
	add a,48				; increase to ascii code range 0-9
	cp 58					; is A less than 10 (58+)
	jp c, step3				; carry on if less, else...
	add a, 7				; add 7 to get A-F char if larger than 10	

step3:	
	rst.lil $10				; print the A char
	
	ld a, (originalA)		; get original A back
	ret						; return to main code

originalA: 	.db 	0		; used to store A


; ---------------------------------------------

printBin:			; take A as number and put into binary, B,C as X,Y

	push af 		; store A for a moment

	ld a, 31		; TAB at x,y
	rst.lil $10
	ld a, b			; x=b
	rst.lil $10
	ld a,c			; y=c
	rst.lil $10		; put tab at BC position

	pop af 			; get original A back again

	ld b, 8  		; number of bits to print
	ld hl, binString
rpt:
	ld (hl), 48 		; ASCII 0 is 48, 1 is 49 ; reset first

	bit 7, a
	jr z, nxt
	ld (hl), 49
nxt:	
	inc hl			; next position in string
	rla 
	djnz rpt

	ld hl, printStr
	ld bc, endPrintStr - printStr

	rst.lil $18

	ret


printStr:			; data to send to VDP

binString:	.db 	"00000000"

endPrintStr:



































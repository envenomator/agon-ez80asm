;
; Title:	Hello World - Initialisation Code
; Author:	Dean Belfield
; Created:	22/11/2022
; Last Updated: 25/11/2022
;
; Modinfo:
; 25/11/2022:	Added parameter parsing; now accepts CR or NUL as end of string markers

			SEGMENT CODE
			
			XREF	__low_bss
			XREF	__len_bss
			
			XREF	_main
			
			.ASSUME	ADL = 1	

_start:	
			PUSH	HL			; Clear the RAM
			PUSH	BC
			CALL	_clear_bss
			POP		BC
			POP		HL

			PUSH	IX			; Parameter 2: argv[0] = IX
			PUSH	BC			; Parameter 1: argc
			CALL	_main		; int main(int argc, char *argv[])
			POP	DE				; Balance the stack
			POP	DE

			RET
			
; Clear the memory
;
_clear_bss:		LD	BC, __len_bss		; Check for non-zero length
			LD	a, __len_bss >> 16
			OR	A, C
			OR	A, B
			RET	Z			; BSS is zero-length ...
			XOR	A, A
			LD 	(__low_bss), A
			SBC	HL, HL			; HL = 0
			DEC	BC			; 1st byte's taken care of
			SBC	HL, BC
			RET	Z		  	; Just 1 byte ...
			LD	HL, __low_bss		; Reset HL
			LD	DE, __low_bss + 1	; [DE] = bss + 1
			LDIR				; Clear this section
			RET

			END

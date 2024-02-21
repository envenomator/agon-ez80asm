;
; Title:	Mos asm loader
; Author:	Jeroen Venema
;           Init code by Dean Belfeld
; Created:	02/03/2023
;
; Modinfo:
; 02/03/2023:	Initial version
; 21/02/2024:	Loads generic binary at address $40000, just passing parameter string in HL
;				Changed to ez80asm version

	.ASSUME	ADL = 1
	.ORG $0B0000

mos_load:		EQU	01h
argv_ptrs_max:	EQU	    16		; Maximum number of arguments allowed in argv
user_start:		EQU 40000h			
max_size:		EQU  6000h		; Max 384KB
;
; Start in ADL mode
;
	JP	_start	; Jump to start
;
; The header stuff is from byte 64 onwards
;

_exec_name:		DB	"EZ80ASM.BIN", 0		; The executable name, only used in argv
_load_name:		DB	"/MOS/EZ80ASM.LDR", 0	; The loadable module name

	ALIGN	64

	DB		"MOS"		; Flag for MOS - to confirm this is a valid MOS command
	DB		00h			; MOS header version 0
	DB		01h			; Flag for run mode (0: Z80, 1: ADL)
;
; And the code follows on immediately after the header
;
_start:
	PUSH	AF			; Preserve the registers
	PUSH	BC
	PUSH	DE
	PUSH	IX
	PUSH	IY

	LD		A, MB		; Save MB
	PUSH 	AF
	XOR 	A
	LD		MB,	A		; Clear to zero so MOS API calls know how to use 24-bit addresses.

	PUSH	HL			; parameter string
	; Load module into memory at start address
	LD		HL,	_load_name
	LD		DE,	user_start
	LD		BC,	max_size
	LD		A,	mos_load
	RST.LIL 08h
	POP		HL			; recover parameter string
	OR		A
	JR		Z,	_continue

	LD		HL, _error_string
	CALL	prstr
	JR		_exit

_continue:
	CALL	user_start	; Start payload

_exit:
	POP		AF
	LD		MB, A

	POP		IY			; Restore registers
	POP		IX
	POP		DE
	POP		BC
	POP		AF
	LD		HL,0		; Always exit with 'success' to MOS
	RET

prstr:
	LD		A,(HL)
	OR		A
	RET		Z
	RST.LIL	10h
	INC		HL
	JR		prstr

_error_string:
	DB	"Error loading /MOS/EZ80ASM.LDR\r\n",0

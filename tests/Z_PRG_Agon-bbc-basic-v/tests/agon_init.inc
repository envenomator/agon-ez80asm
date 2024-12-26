;
; Title:	BBC Basic for AGON - Initialisation Code
;		Initialisation Code
; Author:	Dean Belfield
; Created:	04/12/2024
; Last Updated:	14/12/2024
;
; Modinfo:
; 14/12/2024:	Fix for *BYE command

;			SEGMENT __VECTORS
			
;			XREF	START
;			XREF	ACCS
;			XREF	TELL
		
			.ASSUME	ADL = 0
			.ORG 0x0000
				
			; INCLUDE	"equs.inc"
			
argv_ptrs_max:		EQU	16				; Maximum number of arguments allowed in argv
			
;
; Start in mixed mode. Assumes MBASE is set to correct segment
;	
			JP	_start				; Jump to start
			DS	5

RST_08:			RST.LIS	08h				; API call
			RET
			DS 	5
			
RST_10:			RST.LIS 10h				; Output
			RET
			DS	5
			
RST_18:			RST.LIS	18h				; Block Output
			RET
			DS	5
			
RST_20:			DS	8
RST_28:			DS	8
RST_30:			DS	8	

;	
; The NMI interrupt vector (not currently used by AGON)
;
RST_38:			EI
			RETI
;
; The header stuff is from byte 64 onwards
;
			ALIGN	64
			
			DB	"MOS"				; Flag for MOS - to confirm this is a valid MOS command
			DB	00h				; MOS header version 0
			DB	00h				; Flag for run mode (0: Z80, 1: ADL)
			
_exec_name:		DB	"BBCBASIC.BIN", 0		; The executable name, only used in argv	
	
;
; And the code follows on immediately after the header
;
_start:			PUSH.LIL	IY			; Preserve IY

			LD		IY, 0			; Preserve SPS
			ADD		IY, SP
			PUSH.LIL	IY

			EX		(SP), HL		; Get the SPS part of the return address
			PUSH.LIL	HL
			EX		(SP), HL		; And restore it for BASIC
	
			PUSH.LIL	AF			; Preserve the rest of the registers
			PUSH.LIL	BC
			PUSH.LIL	DE
			PUSH.LIL	IX

			LD		A, MB			; Segment base
			LD		IX, argv_ptrs		; The argv array pointer address
			CALL		_set_aix24		; Convert to a 24-bit address			
			PUSH.LIL	IX
			CALL		_parse_params		; Parse the parameters
			POP.LIL		IX			; IX: argv
			LD		B, 0			;  C: argc
			CALL		_main			; Start user code

			POP.LIL		IX			; Restore the registers
			POP.LIL		DE
			POP.LIL		BC
			POP.LIL		AF

			EX		DE, HL 			; DE: Return code from BASIC
			POP.LIL		HL 			; The SPS part of the return address
			POP.LIL		IY			; Get the preserved SPS
			LD		SP, IY			; Restore SPS
			EX		(SP), HL		; Store the SPS part of the return address on the stack
			EX		DE, HL 			; HL: Return code from BASIC
			
			POP.LIL		IY			; Restore IY
			RET.L					; Return to MOS

; The main routine
; IXU: argv - pointer to array of parameters
;   C: argc - number of parameters
; Returns:
;  HL: Error code, or 0 if OK
;
_main:			LD	HL, ACCS		; Clear the ACCS
			LD	(HL), 0
			LD	A, C			
			CP	2
			JR	Z, _autoload		; 2 parameters = autoload
			JR	C, _startbasic		; 1 parameter = normal start
;			CALL	STAR_VERSION		; Output the AGON version
			CALL	TELL
			DB	"Usage:\n\r"
			DB	"RUN . <filename>\n\r", 0
			LD	HL, 0			; The error code
			RET
;							
_autoload:		LD.LIL	HL, (IX+3)		; HLU: Address of filename
			LD	DE, ACCS		;  DE: Destination address
@@:			LD.LIL	A, (HL)			; Fetch the filename byte
			LD	(DE), A			; 
			INC.LIL	HL			; Increase the source pointer
			INC	E			; We only need to increase E as ACCS is on a page boundary
			JR	NZ, @B			; Loop until we hit a 0 byte
			DEC	E
			LD	A, CR
			LD	(DE), A			; Replace the 0 byte with a CR for BBC BASIC
;
_startbasic:		POP	 HL			; Pop the return address to init off SPS
			PUSH.LIL HL 			; Stack it on SPL (*BYE will use this as the return address)
			JP	 START			; And start BASIC

; Parse the parameter string into a C array
; Parameters
; -   A: Segment base
; - HLU: Address of parameter string
; - IXU: Address for array pointer storage
; Returns:
; -   C: Number of parameters parsed
;
_parse_params:		LD		BC, _exec_name		; Get the address of the app name in this segment			
			CALL		_set_abc24		; Convert it to a 24-bit address based upon segment base
			LD.LIL		(IX+0), BC		; ARGV[0] = the executable name
			INC.LIL		IX
			INC.LIL		IX
			INC.LIL		IX
			CALL		_skip_spaces		; Skip HL past any leading spaces
;
			LD		BC, 1			; C: ARGC = 1 - also clears out top 16 bits of BCU
			LD		B, argv_ptrs_max - 1	; B: Maximum number of argv_ptrs
;
_parse_params_1:	PUSH		BC			; Stack ARGC	
			PUSH.LIL	HL			; Stack start address of token
			CALL		_get_token		; Get the next token
			LD		A, C			; A: Length of the token in characters
			POP.LIL		DE			; Start address of token (was in HL)
			POP		BC			; ARGC
			OR		A			; Check for A=0 (no token found) OR at end of string
			RET		Z
;
			LD.LIL		(IX+0), DE		; Store the pointer to the token
			PUSH.LIL	HL			; DE=HL
			POP.LIL		DE
			CALL		_skip_spaces		; And skip HL past any spaces onto the next character
			XOR		A
			LD.LIL		(DE), A			; Zero-terminate the token
			INC.LIL		IX
			INC.LIL		IX
			INC.LIL		IX			; Advance to next pointer position
			INC		C			; Increment ARGC
			LD		A, C			; Check for C >= A
			CP		B
			JR		C, _parse_params_1	; And loop
			RET

; Get the next token
; Parameters:
; - HL: Address of parameter string
; Returns:
; - HL: Address of first character after token
; -  C: Length of token (in characters)
;
_get_token:		LD		C, 0			; Initialise length
@@:			LD.LIL		A, (HL)			; Get the character from the parameter string
			OR		A			; Exit if 0 (end of parameter string in MOS)
			RET 		Z
			CP		13			; Exit if CR (end of parameter string in BBC BASIC)
			RET		Z
			CP		' '			; Exit if space (end of token)
			RET		Z
			INC.LIL		HL			; Advance to next character
			INC 		C			; Increment length
			JR		@B
	
; Skip spaces in the parameter string
; Parameters:
; - HL: Address of parameter string
; Returns:
; - HL: Address of next none-space character
;    F: Z if at end of string, otherwise NZ if there are more tokens to be parsed
;
_skip_spaces:		LD.LIL		A, (HL)			; Get the character from the parameter string	
			CP		' '			; Exit if not space
			RET		NZ
			INC.LIL		HL			; Advance to next character
			JR		_skip_spaces		; Increment length
			
; Set the MSB of BC (U) to A
; Parameters:
; - BC: 16-bit address
; -  A: Value to stick in U of BC
; Returns:
; - BCU
;
_set_abc24:		PUSH.LIL	HL			; Preserve HL
			PUSH.LIL	BC			; Stick BC onto SPL
			LD.LIL		HL, 2			; HL: SP+2
			ADD.LIL		HL, SP
			LD.LIL		(HL), A			; Store A in it
			POP.LIL		BC			; Fetch ammended BC
			POP.LIL		HL			; Restore HL
			RET

; Set the MSB of BC (U) to A
; Parameters:
; - IX: 16-bit address
; -  A: Value to stick in U of BC
; Returns:
; - IXU
;
_set_aix24:		PUSH.LIL	IX			; Stick IX onto SPL
			LD.LIL		IX, 2			; IX: SP+2
			ADD.LIL		IX, SP
			LD.LIL		(IX), A			; Store A in it
			POP.LIL		IX			; Fetch ammended IX
			RET
			
; Storage for the argv array pointers
;
argv_ptrs:		BLKP	argv_ptrs_max, 0		; Storage for the argv array pointers

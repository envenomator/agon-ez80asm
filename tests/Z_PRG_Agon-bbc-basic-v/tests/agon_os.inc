;
; Title:	BBC Basic for AGON - MOS stuff
; Author:	Dean Belfield
; Created:	04/12/2024
; Last Updated:	17/12/2024
;
; Modinfo:
; 08/12/2024:	Added OSCLI and file I/O
; 11/12/2024:	Added ESC key handling
; 		Added OSWORD
; 12/12/2024:	Added OSRDCH, OSBYTE_81 and fixed *EDIT
; 17/12/2024:	Added OSWORD_01, OSWORD_02, OSWORD_0E, GET$(x,y), fixed INKEY, POS, VPOS and autoload

			.ASSUME	ADL = 0
				
			; INCLUDE	"equs.inc"
			; INCLUDE "macros.inc"
			; INCLUDE "mos_api.inc"	; In MOS/src
		
			; SEGMENT CODE
			
			; XDEF	OSWORD
			; XDEF	OSBYTE
			; XDEF	OSINIT
			; XDEF	OSOPEN
			; XDEF	OSSHUT
			; XDEF	OSLOAD
			; XDEF	OSSAVE
			; XDEF	OSLINE
			; XDEF	OSSTAT
			; XDEF	OSWRCH
			; XDEF	OSRDCH
			; XDEF	OSBGET
			; XDEF	OSBPUT
			; XDEF	OSCLI
			; XDEF	PROMPT
			; XDEF	GETPTR
			; XDEF	PUTPTR
			; XDEF	GETEXT
			; XDEF	TRAP
			; XDEF	LTRAP
			; XDEF	BYE
			; XDEF	RESET
			; XDEF	ESCSET
			
			; XREF	EXTERR
			; XREF	VBLANK_INIT
			; XREF	VBLANK_STOP
			; XREF	USER
			; XREF	COUNT
			; XREF	COUNT0
			; XREF	COUNT1
			; XREF	GETCSR 
			; XREF	GETSCHR_1
			; XREF	NULLTOCR
			; XREF	CRLF
			; XREF	FLAGS
			; XREF	OSWRCHPT
			; XREF	OSWRCHCH
			; XREF	OSWRCHFH
			; XREF	KEYASCII
			; XREF	KEYDOWN
			; XREF	LISTON 
			; XREF	PAGE_
			; XREF	CSTR_FNAME
			; XREF	CSTR_FINDCH
			; XREF	CSTR_CAT 
			; XREF	CSTR_ENDSWITH
			; XREF	CSTR_LINE 
			; XREF	NEWIT
			; XREF	BAD
			; XREF	CLEAN
			; XREF	LINNUM
			; XREF	BUFFER
			; XREF	NXT
			; XREF	ERROR_
			; XREF	XEQ
			; XREF	LEXAN2
			; XREF	GETTOP
			; XREF	FINDL
			; XREF	DEL
			; XREF	LISTIT
			; XREF	ESCAPE
			; XREF	ASC_TO_NUMBER
			; XREF	CLOOP
			; XREF	SCRAP
			; XREF	POINT_
			; XREF	SOUND_
			; XREF	EXPRI 
			; XREF	COMMA 
			; XREF	BRAKET 
			; XREF 	GETSCHR 
			; XREF	ZERO
			; XREF	TRUE

;OSINIT - Initialise RAM mapping etc.
;If BASIC is entered by BBCBASIC FILENAME then file
;FILENAME.BBC is automatically CHAINed.
;   Outputs: DE = initial value of HIMEM (top of RAM)
;            HL = initial value of PAGE (user program)
;            Z-flag reset indicates AUTO-RUN.
;  Destroys: A,D,E,H,L,F
;
OSINIT:			CALL	VBLANK_INIT
			XOR	A
			LD	(FLAGS), A		; Clear flags and set F = Z
			LD 	HL, USER
			LD	DE, RAM_Top
			LD	E, A			; Page boundary
			LD	A, (ACCS)		; Return NZ if there is a file to chain
			OR	A			
			RET	

; PROMPT: output the input prompt
;
PROMPT: 		LD	A,'>'			; Falls through to OSWRCH

; OSWRCH: Write a character out to the ESP32 VDU handler via the MOS
; Parameters:
; - A: Character to write
;
OSWRCH:			PUSH	HL
			LD	HL, LISTON		; Fetch the LISTON variable
			BIT	3, (HL)			; Check whether we are in *EDIT mode
			JR	NZ, OSWRCH_BUFFER	; Yes, so just output to buffer
;
			LD	HL, (OSWRCHCH)		; L: Channel #
			DEC	L			; If it is 1
			JR	Z, OSWRCH_FILE		; Then we are outputting to a file
;
			POP	HL			; Otherwise
			RST.LIS	10h			; Output the character to MOS
			RET
;	
OSWRCH_BUFFER:		LD	HL, (OSWRCHPT)		; Fetch the pointer buffer
			CP	0AH			; Just ignore this
			JR	Z, OSWRCH_BUFFER2
			CP	0DH			; Is it the end of line?
			JR	NZ, OSWRCH_BUFFER1	; No, so carry on
			XOR	A			; Turn it into a NUL character
OSWRCH_BUFFER1:		LD	(HL), A			; Echo the character into the buffer
			INC	HL			; Increment pointer
			LD	(OSWRCHPT), HL		; Write pointer back
OSWRCH_BUFFER2:		POP	HL			
			RET
;
OSWRCH_FILE:		PUSH	DE
			LD	E, H			; Filehandle to E
			CALL	OSBPUT			; Write the byte out
			POP	DE
			POP	HL
			RET

; OSRDCH
;
OSRDCH:			CALL    NXT			; Check if we are doing GET$(x,y)
			CP      '('
			JR	Z, @F 			; Yes, so skip to that functionality
			MOSCALL	mos_getkey		; Otherwise, read keyboard
			CP	1Bh
			JR	Z, LTRAP1 
			RET
;
@@:			INC	IY			; Skip '('
			CALL    EXPRI         	  	; Get the first parameter
			EXX
			PUSH	HL
			CALL	COMMA			; Get the second parameter
			CALL	EXPRI
			EXX 
			POP	DE 			; DE: X coordinate 
			CALL	BRAKET 			; Check for trailing bracket
			JP 	GETSCHR			; Read the character

; OSLINE: Invoke the line editor
;
OSLINE:			LD 	E, 1			; Default is to clear the buffer

; Entry point to line editor that does not clear the buffer
; Parameters:
; - HL: addresses destination buffer (on page boundary)
; Returns:
; -  A: 0
; NB: Buffer filled, terminated by CR
; 
OSLINE1:		PUSH	IY			
			PUSH	HL			; Buffer address
			LD	BC, 256			; Buffer length
			MOSCALL	mos_editline		; Call the MOS line editor
			POP	HL			; Pop the address
			POP	IY
			PUSH	AF			; Stack the return value (key pressed)
			CALL	NULLTOCR		; Turn the 0 character to a CR
			CALL	CRLF			; Display CRLF
			POP	AF
			CP	1Bh 			; Check if ESC terminated the input
			JP	Z, LTRAP1 		; Yes, so do the ESC thing
			LD	A, (FLAGS)		; Otherwise
			RES	7, A 			; Clear the escape flag
			LD	(FLAGS), A 
			CALL	WAIT_VBLANK 		; Wait a frame 
 			XOR	A			; Return A = 0
			LD	(KEYDOWN), A 
			LD	(KEYASCII), A
			RET		

;
; ESCSET
; Set the escape flag (bit 7 of FLAGS = 1) if escape is enabled (bit 6 of FLAGS = 0)
;
ESCSET: 		PUSH    HL
        		LD      HL,FLAGS		; Pointer to FLAGS
        		BIT     6,(HL)			; If bit 6 is set, then
        		JR      NZ,ESCDIS		; escape is disabled, so skip
        		SET     7,(HL)			; Set bit 7, the escape flag
ESCDIS: 		POP     HL
        		RET	

;
; ESCTEST
; Test for ESC key
;
ESCTEST:		CALL	READKEY			; Read the keyboard
			RET	NZ			; Skip if no key is pressed				
			CP	1BH			; If ESC pressed then
			JR	Z,ESCSET		; jump to the escape set routine
			RET

; Read the keyboard
; Returns:
; - A: ASCII of the pressed key
; - F: Z if the key is pressed, otherwise NZ
;
READKEY:		LD	A, (KEYDOWN)		; Get key down
			DEC	A 			; Set Z flag if keydown is 1
			LD	A, (KEYASCII)		; Get key ASCII value
			RET 
;
; TRAP
; This is called whenever BASIC needs to check for ESC
;
TRAP:			CALL	ESCTEST			; Read keyboard, test for ESC, set FLAGS
;
LTRAP:			LD	A,(FLAGS)		; Get FLAGS
			OR	A			; This checks for bit 7; if it is not set then the result will
			RET	P			; be positive (bit 7 is the sign bit in Z80), so return
LTRAP1:			LD	HL,FLAGS 		; Escape is pressed at this point, so
			RES	7,(HL)			; Clear the escape pressed flag and
			JP	ESCAPE			; Jump to the ESCAPE error routine in exec.asm

; RESET
;
RESET:			RET				; Yes this is fine

; OSOPEN
; HL: Pointer to path
;  F: C Z
;     x x OPENIN
; 	  OPENOUT
;     x	  OPENUP
; Returns:
;  A: Filehandle, 0 if cannot open
;
OSOPEN:			LD	C, fa_read
			JR	Z, @F
			LD	C, fa_write | fa_open_append
			JR	C, @F
			LD	C, fa_write | fa_create_always
@@:			MOSCALL	mos_fopen			
			RET

;OSSHUT - Close disk file(s).
; E = file channel
;  If E=0 all files are closed (except SPOOL)
; Destroys: A,B,C,D,E,H,L,F
;
OSSHUT:			PUSH	BC
			LD	C, E
			MOSCALL	mos_fclose
			POP	BC
			RET
	
; OSBGET - Read a byte from a random disk file.
;  E = file channel
; Returns
;  A = byte read
;  Carry set if LAST BYTE of file
; Destroys: A,B,C,F
;
OSBGET:			PUSH	BC
			LD	C, E
			MOSCALL	mos_fgetc
			POP	BC
			RET
	
; OSBPUT - Write a byte to a random disk file.
;  E = file channel
;  A = byte to write
; Destroys: A,B,C,F
;	
OSBPUT:			PUSH	BC
			LD	C, E
			LD	B, A
			MOSCALL	mos_fputc
			POP	BC
			RET

; OSSTAT - Read file status
;  E = file channel
; Returns
;  F: Z flag set - EOF
;  A: If Z then A = 0
; Destroys: A,D,E,H,L,F
;
OSSTAT:			PUSH	BC
			LD	C, E
			MOSCALL	mos_feof
			POP	BC
			CP	1
			RET
	
; GETPTR - Return file pointer.
;    E = file channel
; Returns:
; DEHL = pointer (0-&7FFFFF)
; Destroys: A,B,C,D,E,H,L,F
;
GETPTR:			PUSH		IY
			LD		C, E 
			MOSCALL		mos_getfil 	; HLU: Pointer to FIL structure
			PUSH.LIL	HL
			POP.LIL		IY		; IYU: Pointer to FIL structure
			LD.LIL		L, (IY + FIL.fptr + 0)
			LD.LIL		H, (IY + FIL.fptr + 1)
			LD.LIL		E, (IY + FIL.fptr + 2)
			LD.LIL		D, (IY + FIL.fptr + 3)
			POP		IY
			RET

; PUTPTR - Update file pointer.
;    A = file channel
; DEHL = new pointer (0-&7FFFFF)
; Destroys: A,B,C,D,E,H,L,F
;
PUTPTR:			PUSH		IY 			
			LD		C, A  		; C: Filehandle
			PUSH.LIL	HL 		
			LD.LIL		HL, 2
			ADD.LIL		HL, SP
			LD.LIL		(HL), E 	; 3rd byte of DWORD set to E
			POP.LIL		HL
			LD		E, D  		; 4th byte passed as E
			MOSCALL		mos_flseek
			POP		IY 
			RET
	
; GETEXT - Find file size.
;    E = file channel
; Returns:
; DEHL = file size (0-&800000)
; Destroys: A,B,C,D,E,H,L,F
;
GETEXT:			PUSH		IY 
			LD		C, E 
			MOSCALL		mos_getfil 	; HLU: Pointer to FIL structure
			PUSH.LIL	HL
			POP.LIL		IY		; IYU: Pointer to FIL structure
			LD.LIL		L, (IY + FFOBJID.objsize + 0)
			LD.LIL		H, (IY + FFOBJID.objsize + 1)
			LD.LIL		E, (IY + FFOBJID.objsize + 2)
			LD.LIL		D, (IY + FFOBJID.objsize + 3)
			POP		IY 
			RET	

;OSLOAD - Load an area of memory from a file.
;   Inputs: HL addresses filename (CR terminated)
;           DE = address at which to load
;           BC = maximum allowed size (bytes)
;  Outputs: Carry reset indicates no room for file.
; Destroys: A,B,C,D,E,H,L,F
;
OSLOAD:			PUSH	BC			; Stack the size
			PUSH	DE			; Stack the load address
			LD	DE, ACCS		; Buffer address for filename
			CALL	CSTR_FNAME		; Fetch filename from MOS into buffer
			LD	HL, ACCS		; HL: Filename
			CALL	EXT_DEFAULT		; Tack on the extension .BBC if not specified
			CALL	EXT_HANDLER		; Get the default handler
			POP	DE			; Restore the load address
			POP	BC			; Restore the size
			OR	A
			JP 	Z, OSLOAD_BBC
;
; Load the file in as a text file
;
OSLOAD_TXT:		XOR	A			; Set file attributes to read
			CALL	OSOPEN			; Open the file			
			LD 	E, A 			; The filehandle
			OR	A
			LD	A, 4			; File not found error
			JP	Z, OSERROR		; Jump to error handler
			CALL	NEWIT			; Call NEW to clear the program space
;
OSLOAD_TXT1:		LD	HL, ACCS 		; Where the input is going to be stored
;
; First skip any whitespace (indents) at the beginning of the input
;
@@:			CALL	OSBGET			; Read the byte into A
			JR	C, OSLOAD_TXT3		; Is it EOF?
			CP	LF 			; Is it LF?
			JR	Z, OSLOAD_TXT3 		; Yes, so skip to the next line
			CP	21h			; Is it less than or equal to ASCII space?
			JR	C, @B 			; Yes, so keep looping
			LD	(HL), A 		; Store the first character
			INC	L
;
; Now read the rest of the line in
;
OSLOAD_TXT2:		CALL	OSBGET			; Read the byte into A
			JR	C, OSLOAD_TXT4		; Is it EOF?
			CP	20h			; Skip if not an ASCII character
			JR	C, @F
			LD	(HL), A 		; Store in the input buffer			
			INC	L			; Increment the buffer pointer
			JP	Z, BAD			; If the buffer is full (wrapped to 0) then jump to Bad Program error
@@:			CP	LF			; Check for LF
			JR	NZ, OSLOAD_TXT2		; If not, then loop to read the rest of the characters in
;
; Finally, handle EOL/EOF
;
OSLOAD_TXT3:		LD	(HL), CR		; Store a CR for BBC BASIC
			LD	A, L			; Check for minimum line length
			CP	2			; If it is 2 characters or less (including CR)
			JR	C, @F			; Then don't bother entering it
			PUSH	DE			; Preserve the filehandle
			CALL	OSEDIT			; Enter the line in memory
			CALL	C,CLEAN			; If a new line has been entered, then call CLEAN to set TOP and write &FFFF end of program marker
			POP	DE
@@:			CALL	OSSTAT			; End of file?
			JR	NZ, OSLOAD_TXT1		; No, so loop
			CALL	OSSHUT			; Close the file
			SCF				; Flag to BASIC that we're good
			RET
;
; Special case for BASIC programs with no blank line at the end
;
OSLOAD_TXT4:		CP	20h			; Skip if not an ASCII character
			JR	C, @F
			LD	(HL), A			; Store the character
			INC	L
			JP	Z, BAD
@@:			JR	OSLOAD_TXT3
;
; This bit enters the line into memory
; Also called from OSLOAD_TXT
; Returns:
; F: C if a new line has been entered (CLEAN will need to be called)
;
OSEDIT:			XOR	A			; Entry point after *EDIT
			LD      (COUNT),A
			LD      IY,ACCS
			CALL    LINNUM			; HL: The line number from the input buffer
			CALL    NXT			; Skip spaces
			LD      A,H			; HL: The line number will be 0 for immediate mode or when auto line numbering is used
			OR      L
			JR      Z,LNZERO        	; Skip if there is no line number in the input buffer
;
; This bit does the lexical analysis and tokenisation
;
LNZERO:			LD	DE,BUFFER	
                	LD	C,1			; LEFT MODE	
                	PUSH	HL	
                	CALL	LEXAN2			; LEXICAL ANALYSIS	
                	POP	HL	
                	LD	(DE),A			; TERMINATOR	
                	XOR	A	
                	LD	B,A	
                	LD	C,E			; BC=LINE LENGTH	
                	INC	DE	
                	LD	(DE),A			; ZERO NEXT	
                	LD	A,H	
                	OR	L	
                	LD	IY,BUFFER		; FOR XEQ	
                	JP	Z,XEQ			; DIRECT MODE	
                	PUSH	BC	
                	CALL	FINDL	
                	CALL	Z,DEL	
                	POP	BC	
                	LD	A,C	
                	OR	A	
                	RET	Z
                	ADD	A,4	
                	LD	C,A			; LENGTH INCLUSIVE	
                	PUSH	DE			; LINE NUMBER	
                	PUSH	BC			; SAVE LINE LENGTH	
                	EX	DE,HL	
                	PUSH	BC	
                	CALL	GETTOP	
                	POP	BC	
                	PUSH	HL	
                	ADD	HL,BC	
                	PUSH	HL	
                	INC	H	
                	XOR	A	
                	SBC	HL,SP	
                	POP	HL	
                	JP	NC,ERROR_		; "No room"	
                	EX	(SP),HL	
                	PUSH	HL	
                	INC	HL	
                	OR	A	
                	SBC	HL,DE	
                	LD	B,H			; BC=AMOUNT TO MOVE	
                	LD	C,L	
                	POP	HL	
                	POP	DE	
                	JR	Z,ATENDos	
                	LDDR				; MAKE SPACE	
ATENDos:          	POP	BC			; LINE LENGTH	
                	POP	DE			; LINE NUMBER	
                	INC	HL	
                	LD	(HL),C			; STORE LENGTH	
                	INC	HL	
                	LD	(HL),E			; STORE LINE NUMBER	
                	INC	HL	
                	LD	(HL),D	
                	INC	HL	
                	LD	DE,BUFFER	
                	EX	DE,HL	
                	DEC	C	
                	DEC	C	
                	DEC	C	
                	LDIR				; ADD LINE
			SCF
			RET	
;
; Load the file in as a tokenised binary blob
;
OSLOAD_BBC:		MOSCALL	mos_load		; Call LOAD in MOS
			RET	NC			; If load returns with carry reset - NO ROOM
			OR	A			; If there is no error (A=0)
			SCF				; Need to set carry indicating there was room
			RET	Z			; Return
;
OSERROR:		PUSH	AF			; Handle the MOS error
			LD	HL, ACCS		; Address of the buffer
			LD	BC, 256			; Length of the buffer
			LD	E, A			; The error code
			MOSCALL	mos_getError		; Copy the error message into the buffer
			POP	AF			
			PUSH	HL			; Stack the address of the error (now in ACCS)		
			ADD	A, 127			; Add 127 to the error code (MOS errors start at 128, and are trappable)
			JP	EXTERR			; Trigger an external error

;OSSAVE - Save an area of memory to a file.
;   Inputs: HL addresses filename (term CR)
;           DE = start address of data to save
;           BC = length of data to save (bytes)
; Destroys: A,B,C,D,E,H,L,F
;
OSSAVE:			PUSH	BC			; Stack the size
			PUSH	DE			; Stack the save address
			LD	DE, ACCS		; Buffer address for filename
			CALL	CSTR_FNAME		; Fetch filename from MOS into buffer
			LD	HL, ACCS		; HL: Filename
			CALL	EXT_DEFAULT		; Tack on the extension .BBC if not specified
			CALL	EXT_HANDLER		; Get the default handler
			POP	DE			; Restore the save address
			POP	BC			; Restore the size
			OR	A			; Is the extension .BBC
			JR	Z, OSSAVE_BBC		; Yes, so use that
;
; Save the file out as a text file
;
OSSAVE_TXT:		LD 	A, (OSWRCHCH)		; Stack the current channel
			PUSH	AF
			XOR	A
			INC	A			; Make sure C is clear, A is 1, for OPENOUT
			LD	(OSWRCHCH), A
			CALL	OSOPEN			; Open the file
			LD	(OSWRCHFH), A		; Store the file handle for OSWRCH
			LD	IX, LISTON		; Required for LISTIT
			LD	HL, (PAGE_)		; Get start of program area
			EXX
			LD	BC, 0			; Set the initial indent counters
			EXX			
OSSAVE_TXT1:		LD	A, (HL)			; Check for end of program marker
			OR	A		
			JR	Z, OSSAVE_TXT2
			INC	HL			; Skip the length byte
			LD	E, (HL)			; Get the line number
			INC	HL
			LD	D, (HL)
			INC	HL
			CALL	LISTIT			; List the line
			JR	OSSAVE_TXT1
OSSAVE_TXT2:		LD	A, (OSWRCHFH)		; Get the file handle
			LD	E, A
			CALL	OSSHUT			; Close it
			POP	AF			; Restore the channel
			LD	(OSWRCHCH), A		
			RET
;
; Save the file out as a tokenised binary blob
;
OSSAVE_BBC:		MOSCALL	mos_save		; Call SAVE in MOS
			OR	A			; If there is no error (A=0)
			RET	Z			; Just return
			JR	OSERROR			; Trip an error

; Check if an extension is specified in the filename
; Add a default if not specified
; HL: Filename (CSTR format)
;
EXT_DEFAULT:		PUSH	HL			; Stack the filename pointer	
			LD	C, '.'			; Search for dot (marks start of extension)
			CALL	CSTR_FINDCH
			OR	A			; Check for end of string marker
			JR	NZ, @F			; No, so skip as we have an extension at this point			
			LD	DE, EXT_LOOKUP		; Get the first (default extension)
			CALL	CSTR_CAT		; Concat it to string pointed to by HL
@@:			POP	HL			; Restore the filename pointer
			RET
			
; Check if an extension is valid and, if so, provide a pointer to a handler
; HL: Filename (CSTR format)
; Returns:
;  A: Filename extension type (0=BBC tokenised, 1=ASCII untokenised)
;
EXT_HANDLER:		PUSH	HL			; Stack the filename pointer
			LD	C, '.'			; Find the '.'
			CALL	CSTR_FINDCH
			LD	DE, EXT_LOOKUP		; The lookup table
;
EXT_HANDLER_1:		PUSH	HL			; Stack the pointer to the extension
			CALL	CSTR_ENDSWITH		; Check whether the string ends with the entry in the lookup
			POP	HL			; Restore the pointer to the extension
			JR	Z, EXT_HANDLER_2	; We have a match!
;
@@:			LD	A, (DE)			; Skip to the end of the entry in the lookup
			INC	DE
			OR	A
			JR	NZ, @B
			INC	DE			; Skip the file extension # byte
;
			LD	A, (DE)			; Are we at the end of the table?
			OR	A
			JR	NZ, EXT_HANDLER_1	; No, so loop
;			
			LD      A,204			; Throw a "Bad name" error
        		CALL    EXTERR
        		DB    	"Bad name", 0
;
EXT_HANDLER_2:		INC	DE			; Skip to the file extension # byte
			LD	A, (DE)		
			POP	HL			; Restore the filename pointer
			RET

; Extension lookup table
; CSTR, TYPE
; 	- 0: BBC (tokenised BBC BASIC for Z80 format)
; 	- 1: Human readable plain text
;
EXT_LOOKUP:		DB	".BBC", 0, 0		; First entry is the default extension
			DB	".TXT", 0, 1
			DB	".ASC", 0, 1
			DB	".BAS", 0, 1
			DB	0			; End of table
; OSWORD
;
OSWORD:			CP	01H			; GETIME
			JR	Z, OSWORD_01
			CP	02H			; PUTIME
			JR	Z, OSWORD_02
			CP	0EH			; GETIMS
			JR	Z, OSWORD_0E
			CP	0FH			; PUTIMS
			JR	Z, @F
			CP	07H			; SOUND
			; JR	Z, OSWORD_07
			JP Z, SOUND_ ; REALTIVE JUMP TOO FAR
			CP	08H			; ENVELOPE
			JR	Z, @F
			CP	09H			; POINT
			JR	Z, OSWORD_09
			JP	HUH			; Anything else trips an error
@@:			RET				; Dummy return for unimplemented functions

; GETIME: return current time in centiseconds
;
OSWORD_01:		PUSH 	IX
			MOSCALL	mos_sysvars
			LD	B, 4
@@:			LD.LIL	A, (IX + sysvar_time)
			LD	(HL), A
			INC	HL
			INC.LIL	IX
			DJNZ 	@B
			POP	IX
			RET

; PUTIME: set time in centiseconds
;
OSWORD_02:		PUSH 	IX
			MOSCALL	mos_sysvars
			LD	B, 4
@@:			LD	A, (HL)
			LD.LIL 	(IX + sysvar_time), A
			INC	HL
			INC.LIL IX
			DJNZ 	@B
			POP	IX
			RET

; ; SOUND channel,volume,pitch,duration
; ; Parameters:
; ; - HL: Pointer to data
; ;   - 0,1: Channel
; ;   - 2,3: Volume 0 (off) to 15 (full volume)
; ;   - 4,5: Pitch 0 - 255
; ;   - 6,7: Duration -1 to 254 (duration in 20ths of a second, -1 = play forever)
; ;
; OSWORD_07:		EQU	SOUND_
; ; NOTE: we call this directly above because ez80asm has trouble resolving the label

; OSWORD 0x09: POINT
; Parameters:
; - HL: Address of data
;   - 0,1: X coordinate
;   - 2,3: Y coordinate
;
OSWORD_09:		LD	DE,(SCRAP+0)
			LD	HL,(SCRAP+2)
			CALL	POINT_
			LD	(SCRAP+4),A
			RET	

; GETIMS - Get time from RTC
;
OSWORD_0E:		PUSH	IY
			MOSCALL	mos_getrtc
			POP	IY
			RET

;
; OSBYTE
; Parameters:
; - A: FX #
; - L: First parameter
; - H: Second parameter
;
OSBYTE:			CP	0BH			; Keyboard auto-repeat delay
			JR	Z, OSBYTE_0B
			CP	0CH			; Keyboard auto-repeat rate
			JR	Z, OSBYTE_0C
			CP	13H			; Wait for vblank
			JR	Z, OSBYTE_13		
			CP	76H			; Set keyboard LED
			JR	Z, OSBYTE_76
			CP	81H			; Read the keyboard
			JP	Z, OSBYTE_81
			CP	86H			; Get cursor coordinates
			JP	Z, OSBYTE_86
			CP	87H			; Fetch current mode and character under cursor
			JP	Z, OSBYTE_87
			CP	A0H			; Fetch system variable
			JP	Z, OSBYTE_A0		
;
; Anything else trips an error
;
HUH:    		LD      A,254			; Bad command error
        		CALL    EXTERR
        		DB    	"Bad command"
        		DEFB    0				

; OSBYTE 0x0B (FX 11,n): Keyboard auto-repeat delay
; Parameters:
; - HL: Repeat delay
;
OSBYTE_0B:		VDU	23
			VDU	0
			VDU	vdp_keystate
			VDU	L
			VDU	H 
			VDU	0
			VDU 	0
			VDU	255
			RET 

; OSBYTE 0x0C (FX 12,n): Keyboard auto-repeat rate
; Parameters:
; - HL: Repeat rate
;
OSBYTE_0C:		VDU	23
			VDU	0
			VDU	vdp_keystate
			VDU	0
			VDU 	0
			VDU	L
			VDU	H 
			VDU	255
			RET 

; OSBYTE 0x13 (FX 19): Wait for vertical blank interrupt
;
OSBYTE_13:		CALL	WAIT_VBLANK
			LD	L, 0			; Returns 0
			JP	COUNT0
;
; OSBYTE 0x76 (FX 118,n): Set Keyboard LED
; Parameters:
; - L: LED (Bit 0: Scroll Lock, Bit 1: Caps Lock, Bit 2: Num Lock)
;
OSBYTE_76:		VDU	23
			VDU	0
			VDU	vdp_keystate
			VDU	0
			VDU 	0
			VDU	0
			VDU	0 
			VDU	L
			RET 

; OSBYTE 0x81: Read the keyboard
; Parameters:
; - HL = Time to wait (centiseconds)
; Returns:
; - F: Carry reset indicates time-out
; - H: NZ if timed out
; - L: The character typed
; Destroys: A,D,E,H,L,F
;
OSBYTE_81:		EXX
			BIT 	7, H 			; Check for minus numbers
			EXX
			JR	NZ, OSBYTE_81_1		; Yes, so do INKEY(-n)
			CALL	READKEY			; Read the keyboard 
			JR	Z, @F 			; Skip if we have a key
			CALL	WAIT_VBLANK 		; Wait a frame
			LD	A, H 			; Check loop counter
			OR 	L
			DEC 	HL			; Decrement
			JR	NZ, OSBYTE_81		; And loop 
			RET 				; H: Will be set to 255 to flag timeout
;
@@:			LD	HL, KEYDOWN		; We have a key, so 
			LD	(HL), 0			; clear the keydown flag
			CP	1BH			; If we are pressing ESC, 
			JP	Z, ESCSET 		; Then handle ESC
			LD	H, 0			; H: Not timed out
			LD	L, A			; L: The character
			RET	
;
;
; Check immediately whether a given key is being pressed
; Result is integer numeric
;
OSBYTE_81_1:		MOSCALL	mos_getkbmap		; Get the base address of the keyboard
			INC	HL			; Index from 0
			LD	A, L			; Negate the LSB of the answer
			NEG
			LD	C, A			;  E: The positive keycode value
			LD	A, 1			; Throw an "Out of range" error
			JP	M, ERROR_		; if the argument < - 128
;
			LD	HL, BITLOOKUP		; HL: The bit lookup table
			LD	DE, 0
			LD	A, C
			AND	00000111b		; Just need the first three bits
			LD	E, A			; DE: The bit number
			ADD	HL, DE
			LD	B, (HL)			;  B: The mask
;
			LD	A, C			; Fetch the keycode again
			AND	01111000b		; And divide by 8
			RRCA
			RRCA
			RRCA
			LD	E, A			; DE: The offset (the MSW has already been cleared previously)
			ADD.LIL	IX, DE			; IX: The address
			LD.LIL	A, (IX+0)		;  A: The keypress
			AND	B			; Check whether the bit is set
			JP	Z, ZERO			; No, so return 0
			JP	TRUEev			; Otherwise return -1
;
; A bit lookup table
;
BITLOOKUP:		DB	01h, 02h, 04h, 08h
			DB	10h, 20h, 40h, 80h	

; OSBYTE 0x86: Fetch cursor coordinates
; Returns:
; - L: X Coordinate (POS)
; - H: Y Coordinate (VPOS)
;
OSBYTE_86:		PUSH	IX			; Get the system vars in IX
			MOSCALL	mos_sysvars		; Reset the semaphore
			RES.LIL	0, (IX+sysvar_vpd_pflags)
			VDU	23
			VDU	0
			VDU	vdp_cursor
@@:			BIT.LIL	0, (IX+sysvar_vpd_pflags)
			JR	Z, @B			; Wait for the result
			LD.LIL	L, (IX + sysvar_cursorX)
			LD.LIL	H, (IX + sysvar_cursorY)			
			POP	IX			
			RET	

; OSBYTE 0x87: Fetch current mode and character under cursor
;
OSBYTE_87:		PUSH	IX
			CALL	GETCSR			; Get the current screen position
			CALL	GETSCHR			; Read character from screen
			LD	L, A 
			MOSCALL	mos_sysvars
			LD.LIL	H, (IX+sysvar_scrMode)	; H: Screen mode
			POP	IX
			JP	COUNT1
			
; OSBYTE 0xA0: Fetch system variable
; Parameters:
; - L: The system variable to fetch
;
OSBYTE_A0:		PUSH	IX
			MOSCALL	mos_sysvars		; Fetch pointer to system variables
			LD.LIL	BC, 0			
			LD	C, L			; BCU = L
			ADD.LIL	IX, BC			; Add to IX
			LD.LIL	L, (IX + 0)		; Fetch the return value
			POP	IX
			JP 	COUNT0

; OSCLI
;
;
;OSCLI - Process a MOS command
;
OSCLI: 			CALL    SKIPSP
			CP      CR
			RET     Z
			CP      '|'
			RET     Z
			EX      DE,HL
			LD      HL,COMDS
OSCLI0:			LD      A,(DE)
			CALL    UPPRC
			CP      (HL)
			JR      Z,OSCLI2
			JR      C,OSCLI6
OSCLI1:			BIT     7,(HL)
			INC     HL
			JR      Z,OSCLI1
			INC     HL
			INC     HL
			JR      OSCLI0
;
OSCLI2:			PUSH    DE
OSCLI3:			INC     DE
			INC     HL
			LD      A,(DE)
			CALL    UPPRC
			CP      '.'			; ABBREVIATED?
			JR      Z,OSCLI4
			XOR     (HL)
			JR      Z,OSCLI3
			CP      80H
			JR      Z,OSCLI4
			POP     DE
			JR      OSCLI1
;
OSCLI4:			POP     AF
		        INC     DE
OSCLI5:			BIT     7,(HL)
			INC     HL
			JR      Z,OSCLI5
			LD      A,(HL)
			INC     HL
			LD      H,(HL)
			LD      L,A
			PUSH    HL
			EX      DE,HL
			JP      SKIPSP
;
OSCLI6:			EX	DE, HL			; HL: Buffer for command
			LD	DE, ACCS		; Buffer for command string is ACCS (the string accumulator)
			PUSH	DE			; Store buffer address
			CALL	CSTR_LINE		; Fetch the line
			POP	HL			; HL: Pointer to command string in ACCS
			PUSH	IY
			MOSCALL	mos_oscli		; Returns OSCLI error in A
			POP	IY
			OR	A			; 0 means MOS returned OK
			RET	Z			; So don't do anything
			JP 	OSERROR			; Otherwise it's a MOS error

SKIPSP:			LD      A,(HL)			
        		CP      ' '
        		RET     NZ
        		INC     HL
        		JR      SKIPSP	

UPPRC:  		AND     7FH
			CP      '`'
			RET     C
			AND     5FH			; CONVERT TO UPPER CASE
			RET	

; Each command has bit 7 of the last character set, and is followed by the address of the handler
; These must be in alphabetical order
;		
COMDS:  		DB	"BY",'E'+80h		; BYE
			DW	BYE
			DB	"EDI",'T'+80h		; EDIT
			DW	STAR_EDIT
			DB	'F','X'+80h		; FX
			DW	STAR_FX
;			DB	'VERSIO','N'+80h	; VERSION
;			DW	STAR_VERSION
			DB	FFh			

; *BYE
;
BYE:			CALL	VBLANK_STOP		; Restore MOS interrupts
			POP.LIL	IX 			; The return address to init
			LD	HL, 0			; The return code
			JP	(IX)

; *EDIT linenum
;
STAR_EDIT:		CALL	ASC_TO_NUMBER		; DE: Line number to edit
			EX	DE, HL			; HL: Line number
			CALL	FINDL			; HL: Address in RAM of tokenised line			
			LD	A, 41			; F:NZ If the line is not found
			JP	NZ, ERROR_		; Do error 41: No such line in that case
;
; Use LISTIT to output the line to the ACCS buffer
;
			INC	HL			; Skip the length byte
			LD	E, (HL)			; Fetch the line number
			INC	HL
			LD	D, (HL)
			INC	HL
			LD	IX, ACCS		; Pointer to where the copy is to be stored
			LD	(OSWRCHPT), IX
			LD	IX, LISTON		; Pointer to LISTON variable in RAM
			LD	A, (IX)			; Store that variable
			PUSH	AF
			LD	(IX), 09h		; Set to echo to buffer
			CALL	LISTIT
			POP	AF
			LD	(IX), A			; Restore the original LISTON variable			
			LD	HL, ACCS		; HL: ACCS
			LD	E, L			;  E: 0 - Don't clear the buffer; ACCS is on a page boundary so L is 0
			CALL	OSLINE1			; Invoke the editor
			CALL	OSEDIT
			CALL    C,CLEAN			; Set TOP, write out &FFFF end of program marker
			JP      CLOOP			; Jump back to immediate mode

; OSCLI FX n
;
STAR_FX:		CALL	ASC_TO_NUMBER
			LD	C, E			; C: Save FX #
			CALL	ASC_TO_NUMBER
			LD	A, D  			; Is first parameter > 255?
			OR 	A 			
			JR	Z, STAR_FX1		; Yes, so skip next bit 
			EX	DE, HL 			; Parameter is 16-bit
			JR	STAR_FX2 
;
STAR_FX1:		LD	B, E 			; B: Save First parameter
			CALL	ASC_TO_NUMBER		; Fetch second parameter
			LD	L, B 			; L: First parameter
			LD	H, E 			; H: Second parameter
;
STAR_FX2:		LD	A, C 			; A: FX #
			JP	OSBYTE	

; Helper Functions
;
WAIT_VBLANK:		PUSH 	IX			; Wait for VBLANK interrupt
			MOSCALL	mos_sysvars		; Fetch pointer to system variables
			LD.LIL	A, (IX + sysvar_time + 0)
@@:			CP.LIL 	A, (IX + sysvar_time + 0)
			JR	Z, @B
			POP	IX
			RET
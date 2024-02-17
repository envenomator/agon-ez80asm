	XDEF _io_filesize
	
	segment CODE
	.assume ADL=1

mos_getfil:			.EQU	19h
fil_obj:			.EQU	0
ffobjid_objsize:	.EQU	11

; Gets filesize from an open file
; requires MOS mos_getfil call
; Input: A - MOS filehandle
; Output: HL - 24bit filesize

_io_filesize:
	PUSH IX
	LD	 IX, 0h
	ADD	 IX, SP

    ; get pointer to FIL structure in MOS memory
	LD	A, (IX+6)
    LD  C, A
    LD  A, mos_getfil
    RST.LIL 08h

    LD  DE, fil_obj + ffobjid_objsize    ; offset to lower 3bytes in FSIZE_t, part of the FFOBJD struct that HL points to
    ADD HL, DE
    LD  HL, (HL)     ; load actual FSIZE_t value (lower 3 bytes)

	LD	SP, IX
	POP	IX
	RET
end
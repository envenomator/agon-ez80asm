;
;Automatically created from original source on 2024-12-15 15:29:12
;
                .ASSUME ADL = 0	
;	.ORG 0x0000
;                SEGMENT CODE	
;
;                XDEF	NEWIT	
;                XDEF	BAD	
;                XDEF	CLEAN	
;                XDEF	LINNUM	
;                XDEF	ERROR_	
;                XDEF	GETTOP	
;                XDEF	DEL	
;                XDEF	LISTIT	
;
;BBC BASIC INTERPRETER - Z80 VERSION
;COMMANDS AND COMMON MODULE - "MAIN"
;(C) COPYRIGHT R.T.RUSSELL 1981-2024
;
;THE NAME BBC BASIC IS USED WITH THE PERMISSION
;OF THE BRITISH BROADCASTING CORPORATION AND IS
;NOT TRANSFERRABLE TO A FORKED OR DERIVED WORK.
;
;VERSION 2.3, 07-05-1984
;VERSION 3.0, 01-03-1987
;VERSION 5.0, 31-05-2024
;VERSION 5.1, 10-08-2024
;
;                XREF	XEQ	
;                XREF	RUN0	
;                XREF	CHAIN0	
;                XREF	TERMQ	
;                XREF	MUL16	
;                XREF	X14OR5	
;                XREF	SPACES	
;                XREF	ESCAPE	
;                XREF	CHECK	
;                XREF	SEARCH	
;
;                XREF	OSWRCH	
;                XREF	OSLINE	
;                XREF	OSINIT	
;                XREF	OSLOAD	
;                XREF	OSSAVE	
;                XREF	OSBGET	
;                XREF	OSBPUT	
;                XREF	OSSHUT	
;                XREF	OSSTAT	
;                XREF	PROMPT	
;                XREF	LTRAP	
;                XREF	OSCLI	
;                XREF	RESET	
;
;                XREF	COMMA	
;                XREF	BRAKET	
;                XREF	ZERO	
;                XREF	ITEMI	
;                XREF	EXPRI	
;                XREF	EXPRS	
;                XREF	DECODE	
;                XREF	LOADN	
;                XREF	SFIX	
;
;                XDEF	NXT	
;                XDEF	NLIST	
;                XDEF	START	
;                XDEF	OUTCHR	
;                XDEF	OUT	
;                XDEF	ERROR_	
;                XDEF	EXTERR	
;                XDEF	REPORT	
;                XDEF	CLOOP	
;                XDEF	WARM	
;                XDEF	CLEAR	
;                XDEF	CRLF	
;                XDEF	SAYLN	
;                XDEF	LOAD0	
;                XDEF	TELL	
;                XDEF	FINDL	
;                XDEF	GETTOP	
;                XDEF	SETLIN	
;                XDEF	GETVAR	
;                XDEF	PUTVAR	
;                XDEF	GETDEF	
;                XDEF	LOCATE	
;                XDEF	CREATE	
;                XDEF	PBCDL	
;                XDEF	LEXAN2	
;                XDEF	RANGE	
;                XDEF	VERMSG	
;                XDEF	KEYWDS	
;                XDEF	KEYWDL	
;
;                XREF	PAGE_	
;                XREF	ACCS	
;                XREF	BUFFER	
;                XREF	LOMEM	
;                XREF	HIMEM	
;                XREF	COUNT	
;                XREF	WIDTH	
;                XREF	FREE	
;                XREF	STAVAR	
;                XREF	DYNVAR	
;                XREF	ERRTXT	
;                XREF	ERR	
;                XREF	ERL	
;                XREF	CURLIN	
;                XREF	ERRTRP	
;                XREF	ONERSP	
;                XREF	FNPTR	
;                XREF	PROPTR	
;                XREF	AUTONO	
;                XREF	INCREM	
;                XREF	LISTON	
;                XREF	TRACEN	
;
; CR             EQU	0DH	
; LF             EQU	0AH	
; ESC            EQU	1BH	
;
TERROR:         EQU	85H	
TLINE:          EQU	86H	
TELSE:          EQU	8BH	
TTHEN:          EQU	8CH	
TLINO:          EQU	8DH	
TFN:            EQU	0A4H	
TTO:            EQU	0B8H	
TWHILE:         EQU	0C7H	
TCASE:          EQU	0C8H	
TWHEN:          EQU	0C9H	
TOF:            EQU	0CAH	
TENDCASE:       EQU	0CBH	
TOTHERWISE:     EQU	0CCH	
TENDIF:         EQU	0CDH	
TENDWHILE:      EQU	0CEH	
TDATA:          EQU	0DCH	
TDIM:           EQU	0DEH	
TFOR:           EQU	0E3H	
TGOSUB:         EQU	0E4H	
TGOTO:          EQU	0E5H	
TIF:            EQU	0E7H	
TLOCAL:         EQU	0EAH	
TNEXT:          EQU	0EDH	
TON:            EQU	0EEH	
TPROC:          EQU	0F2H	
TREM:           EQU	0F4H	
TREPEAT:        EQU	0F5H	
TRESTORE:       EQU	0F7H	
TTRACE:         EQU	0FCH	
TUNTIL:         EQU	0FDH	
TEXIT:          EQU	10H	
;
TOKLO:          EQU	8FH	
TOKHI:          EQU	93H	
OFFSET:         EQU	0CFH-TOKLO	
;
START:          JP	COLD	
                JP	WARM	
                JP	ESCAPE	
                JP	EXTERR	
                JP	TELL	
                JP	TEXT_	
                JP	ITEMI	
                JP	EXPRI	
                JP	EXPRS	
                JP	OSCLI	
                JP	OSBGET	
                JP	OSBPUT	
                JP	OSSTAT	
                JP	OSSHUT	
COLD:           LD	HL,STAVAR	;COLD START	
                LD	SP,HL	
                LD	(HL),10	
                INC	L	
                LD	(HL),9	
                INC	L	
                XOR	A	
PURGE:          LD	(HL),A		;CLEAR SCRATCHPAD	
                INC	L	
                JR	NZ,PURGE	
                LD	A,37H		;V3.0	
                LD	(LISTON),A	
                LD	HL,NOTICE	
                LD	(ERRTXT),HL	
                CALL	OSINIT	
                LD	(HIMEM),DE	
                LD	(PAGE_),HL	
                CALL	NEWIT	
                JP	NZ,CHAIN0	;AUTO-RUN	
                CALL	TELL	
VERMSG:         DB	"BBC BASIC (Z80) Version 5.00  "	
                DB	CR	
                DB	LF	
NOTICE:         DB	"(C) Copyright R.T.Russell 2024"	
                DB	CR	
                DB	LF	
                DB	0	
WARM:           DB	0F6H	
CLOOP:          SCF	
                LD	SP,(HIMEM)	
                CALL	PROMPT		;PROMPT USER	
                LD	HL,LISTON	
                LD	A,(HL)	
                AND	0FH		;LISTO	
                OR	30H		;OPT 3	
                LD	(HL),A	
                SBC	HL,HL		;HL <- 0 (V3.0)	
                LD	(ERRTRP),HL	
                LD	(ONERSP),HL	
                LD	(CURLIN),HL	;For CMOS EDIT->LIST	
                LD	HL,(AUTONO)	
                PUSH	HL	
                LD	A,H	
                OR	L	
                JR	Z,NOAUTO	
                PUSH	HL	
                CALL	PBCD		;AUTO NUMBER	
                POP	HL	
                LD	BC,(INCREM)	
                LD	B,0	
                ADD	HL,BC	
                JP	C,TOOBIGmn	
                LD	(AUTONO),HL	
                LD	A,' '	
                CALL	OUTCHR	
NOAUTO:         LD	HL,ACCS	
                CALL	OSLINE		;GET CONSOLE INPUT	
                XOR	A	
                LD	(COUNT),A	
                LD	IY,ACCS	
                LD	HL,COMNDS	
                CALL	LEX0	
                POP	HL	
                JR	NZ,NOTCMD	
                ADD	A,A	
                LD	C,A	
                LD	A,H	
                OR	L	
                JR	NZ,INAUTO	
                LD	B,A	
                LD	HL,CMDTABmn	
                ADD	HL,BC	
                LD	A,(HL)		;TABLE ENTRY	
                INC	HL	
                LD	H,(HL)	
                LD	L,A	
                INC	IY	
                CALL	NXT	
                JP	(HL)		;EXECUTE COMMAND	
;
INAUTO:         LD	IY,ACCS	
NOTCMD:         LD	A,H	
                OR	L	
                CALL	Z,LINNUM	
                CALL	NXT	
                LD	DE,BUFFER	
                LD	C,1		;LEFT MODE	
                PUSH	HL	
                CALL	LEXAN2		;LEXICAL ANALYSIS	
                POP	HL	
                LD	(DE),A		;TERMINATOR	
                XOR	A	
                LD	B,A	
                LD	C,E		;BC=LINE LENGTH	
                INC	DE	
                LD	(DE),A		;ZERO NEXT	
                LD	A,H	
                OR	L	
                LD	IY,BUFFER	;FOR XEQ	
                JP	Z,XEQ		;DIRECT MODE	
                PUSH	BC	
                CALL	FINDL	
                CALL	Z,DEL	
                POP	BC	
                LD	A,C	
                OR	A	
                JR	Z,CLOOP2	;DELETE LINE ONLY	
                ADD	A,4	
                LD	C,A		;LENGTH INCLUSIVE	
                PUSH	DE		;LINE NUMBER	
                PUSH	BC		;SAVE LINE LENGTH	
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
                JP	NC,ERROR_	;"No room"	
                EX	(SP),HL	
                PUSH	HL	
                INC	HL	
                OR	A	
                SBC	HL,DE	
                LD	B,H		;BC=AMOUNT TO MOVE	
                LD	C,L	
                POP	HL	
                POP	DE	
                JR	Z,ATEND	
                LDDR			;MAKE SPACE	
ATEND:          POP	BC		;LINE LENGTH	
                POP	DE		;LINE NUMBER	
                INC	HL	
                LD	(HL),C		;STORE LENGTH	
                INC	HL	
                LD	(HL),E		;STORE LINE NUMBER	
                INC	HL	
                LD	(HL),D	
                INC	HL	
                LD	DE,BUFFER	
                EX	DE,HL	
                DEC	C	
                DEC	C	
                DEC	C	
                LDIR			;ADD LINE	
                CALL	CLEAN	
CLOOP2:         JP	CLOOP	
;
;LIST OF TOKENS AND KEYWORDS.
;IF A KEYWORD IS FOLLOWED BY NUL THEN IT WILL
; ONLY MATCH WITH THE WORD FOLLOWED IMMEDIATELY
; BY A DELIMITER.
;
KEYWDS:         DB	80H	
                DB	"AND"	
                DB	94H	
                DB	"ABS"	
                DB	95H	
                DB	"ACS"	
                DB	96H	
                DB	"ADVAL"	
                DB	97H	
                DB	"ASC"	
                DB	98H	
                DB	"ASN"	
                DB	99H	
                DB	"ATN"	
                DB	9AH	
                DB	"BGET "	
                DB	0D5H	
                DB	"BPUT "	
                DB	0FH	
                DB	"BY "		; v5	
                DB	0FBH	
                DB	"COLOUR"	
                DB	0FBH	
                DB	"COLOR"	
                DB	0D6H	
                DB	"CALL"	
                DB	0C8H	
                DB	"CASE"		; v5	
                DB	0D7H	
                DB	"CHAIN"	
                DB	0BDH	
                DB	"CHR$"	
                DB	0D8H	
                DB	"CLEAR "	
                DB	0D9H	
                DB	"CLOSE "	
                DB	0DAH	
                DB	"CLG "	
                DB	0DBH	
                DB	"CLS "	
                DB	9BH	
                DB	"COS"	
                DB	9CH	
                DB	"COUNT "	
                DB	01H	
                DB	"CIRCLE"	; v5	
                DB	0DCH	
                DB	"DATA"	
                DB	9DH	
                DB	"DEG"	
                DB	0DDH	
                DB	"DEF"	
                DB	81H	
                DB	"DIV"	
                DB	0DEH	
                DB	"DIM"	
                DB	0DFH	
                DB	"DRAW"	
                DB	0E1H	
                DB	"ENDPROC "	
                DB	0CEH	
                DB	"ENDWHILE "	; v5	
                DB	0CBH	
                DB	"ENDCASE "	; v5	
                DB	0CDH	
                DB	"ENDIF "	; v5	
                DB	0E0H	
                DB	"END "	
                DB	0E2H	
                DB	"ENVELOPE"	
                DB	8BH	
                DB	"ELSE"	
                DB	0A0H	
                DB	"EVAL"	
                DB	9EH	
                DB	"ERL "	
                DB	85H	
                DB	"ERROR"	
                DB	0C5H	
                DB	"EOF "	
                DB	82H	
                DB	"EOR"	
                DB	9FH	
                DB	"ERR "	
                DB	10H	
                DB	"EXIT "		; v5	
                DB	0A1H	
                DB	"EXP"	
                DB	0A2H	
                DB	"EXT "	
                DB	02H	
                DB	"ELLIPSE"	; v5	
                DB	0E3H	
                DB	"FOR"	
                DB	0A3H	
                DB	"FALSE "	
                DB	03H	
                DB	"FILL"		; v5	
                DB	0A4H	
                DB	"FN"	
                DB	0E5H	
                DB	"GOTO"	
                DB	0BEH	
                DB	"GET$"	
                DB	0A5H	
                DB	"GET"	
                DB	0E4H	
                DB	"GOSUB"	
                DB	0E6H	
                DB	"GCOL"	
                DB	93H	
                DB	"HIMEM "	
                DB	0E8H	
                DB	"INPUT"	
                DB	0E7H	
                DB	"IF"	
                DB	0BFH	
                DB	"INKEY$"	
                DB	0A6H	
                DB	"INKEY"	
                DB	0A8H	
                DB	"INT"	
                DB	0A7H	
                DB	"INSTR("	
                DB	0CH	
                DB	"INSTALL"	; v5	
                DB	86H	
                DB	"LINE"	
                DB	92H	
                DB	"LOMEM "	
                DB	0EAH	
                DB	"LOCAL"	
                DB	0C0H	
                DB	"LEFT$("	
                DB	0A9H	
                DB	"LEN"	
                DB	0E9H	
                DB	"LET"	
                DB	0ABH	
                DB	"LOG"	
                DB	0AAH	
                DB	"LN"	
                DB	0C1H	
                DB	"MID$("	
                DB	0EBH	
                DB	"MODE"	
                DB	83H	
                DB	"MOD"	
                DB	0ECH	
                DB	"MOVE"	
                DB	04H	
                DB	"MOUSE"		; v5	
                DB	0EDH	
                DB	"NEXT"	
                DB	0ACH	
                DB	"NOT"	
                DB	0EEH	
                DB	"ON"	
                DB	87H	
                DB	"OFF "	
                DB	0CAH	
                DB	"OF "		; v5	
                DB	05H	
                DB	"ORIGIN"	; v5	
                DB	84H	
                DB	"OR"	
                DB	8EH	
                DB	"OPENIN"	
                DB	0AEH	
                DB	"OPENOUT"	
                DB	0ADH	
                DB	"OPENUP"	
                DB	0FFH	
                DB	"OSCLI"	
                DB	0CCH	
                DB	"OTHERWISE"	; v5	
                DB	0F1H	
                DB	"PRINT"	
                DB	90H	
                DB	"PAGE "	
                DB	8FH	
                DB	"PTR "	
                DB	0AFH	
                DB	"PI "	
                DB	0F0H	
                DB	"PLOT"	
                DB	0B0H	
                DB	"POINT("	
                DB	0F2H	
                DB	"PROC"	
                DB	0B1H	
                DB	"POS "	
                DB	0EH	
                DB	"PUT"		; Token changed	
                DB	06H	
                DB	"QUIT "		; v5	
                DB	0F8H	
                DB	"RETURN "	
                DB	0F5H	
                DB	"REPEAT"	
                DB	0F6H	
                DB	"REPORT "	
                DB	0F3H	
                DB	"READ"	
                DB	0F4H	
                DB	"REM"	
                DB	0F9H	
                DB	"RUN "	
                DB	0B2H	
                DB	"RAD"	
                DB	0F7H	
                DB	"RESTORE"	
                DB	0C2H	
                DB	"RIGHT$("	
                DB	0B3H	
                DB	"RND "	
                DB	07H	
                DB	"RECTANGLE"	; v5	
                DB	88H	
                DB	"STEP"	
                DB	0B4H	
                DB	"SGN"	
                DB	0B5H	
                DB	"SIN"	
                DB	0B6H	
                DB	"SQR"	
                DB	89H	
                DB	"SPC"	
                DB	0C3H	
                DB	"STR$"	
                DB	0C4H	
                DB	"STRING$("	
                DB	0D4H	
                DB	"SOUND"	
                DB	0FAH	
                DB	"STOP "	
                DB	0C6H	
                DB	"SUM"		; v5	
                DB	08H	
                DB	"SWAP"		; v5	
                DB	09H	
                DB	"SYS"		; v5	
                DB	0B7H	
                DB	"TAN"	
                DB	8AH	
                DB	"TAB("	
                DB	8CH	
                DB	"THEN"	
                DB	91H	
                DB	"TIME "	
                DB	0AH	
                DB	"TINT"	
                DB	0B8H	
                DB	"TO"	
                DB	0FCH	
                DB	"TRACE"	
                DB	0B9H	
                DB	"TRUE "	
                DB	0FDH	
                DB	"UNTIL"	
                DB	0BAH	
                DB	"USR"	
                DB	0EFH	
                DB	"VDU"	
                DB	0BBH	
                DB	"VAL"	
                DB	0BCH	
                DB	"VPOS "	
                DB	0C7H	
                DB	"WHILE"		; v5	
                DB	0C9H	
                DB	"WHEN"		; v5	
                DB	0BH	
                DB	"WAIT "		; v5	
                DB	0FEH	
                DB	"WIDTH"	
;'LEFT' TOKENS:
                DB	0CFH	
                DB	"PTR"	
                DB	0D1H	
                DB	"TIME"	
                DB	0D3H	
                DB	"HIMEM"	
                DB	0D2H	
                DB	"LOMEM"	
                DB	0D0H	
                DB	"PAGE"	
;
                DB	11H	
                DB	"Missing "	
                DB	12H	
                DB	"No such "	
                DB	13H	
                DB	"Bad "	
                DB	14H	
                DB	" range"	
                DB	15H	
                DB	"variable"	
                DB	16H	
                DB	"Out of"	
                DB	17H	
                DB	"No "	
                DB	18H	
                DB	" space"	
                DB	19H	
                DB	"Not in a "	
                DB	1AH	
                DB	" loop"	
                DB	1BH	
                DB	" not "	
KEYWDL:         EQU	$-KEYWDS	
                DW	-1	
;
;LIST OF IMMEDIATE MODE COMMANDS:
;
COMNDS:         DB	80H	
                DB	"AUTO"	
                DB	81H	
                DB	"DELETE"	
                DB	82H	
                DB	"LIST"	
                DB	83H	
                DB	"LOAD"	
                DB	84H	
                DB	"NEW "	
                DB	85H	
                DB	"OLD "	
                DB	86H	
                DB	"RENUMBER"	
                DB	87H	
                DB	"SAVE"	
                DW	-1	
;
;IMMEDIATE MODE COMMANDS:
;
CMDTABmn:         DW	AUTO	
                DW	DELETE	
                DW	LIST	
                DW	LOAD	
                DW	NEW	
                DW	OLD	
                DW	RENUM	
                DW	SAVE	
;
;ERROR MESSAGES:
;
ERRWDS:         DB	17H	
                DB	"room"	
                DB	0	
                DB	16H	
                DB	14H	
                DW	0	
                DB	"Multiple label"	
                DB	0	
                DB	"Mistake"	
                DB	0	
                DB	11H	
                DB	','	
                DB	0	
                DB	"Type mismatch"	
                DB	0	
                DB	19H	
                DB	TFN	
                DW	0	
                DB	11H	
                DB	'"'	
                DB	0	
                DB	13H	
                DB	TDIM	
                DB	0	
                DB	TDIM	
                DB	18H	
                DB	0	
                DB	19H	
                DB	TFN	
                DB	" or "	
                DB	TPROC	
                DB	0	
                DB	19H	
                DB	TPROC	
                DB	0	
                DB	13H	
                DB	"use of array"	
                DB	0	
                DB	13H	
                DB	"subscript"	
                DB	0	
                DB	"Syntax error"	
                DB	0	
                DB	"Escape"	
                DB	0	
                DB	"Division by zero"	
                DB	0	
                DB	"String too long"	
                DB	0	
                DB	"Number too big"	
                DB	0	
                DB	"-ve root"	
                DB	0	
                DB	"Log"	
                DB	14H	
                DB	0	
                DB	"Accuracy lost"	
                DB	0	
                DB	"Exponent"	
                DB	14H	
                DW	0	
                DB	12H	
                DB	15H	
                DB	0	
                DB	11H	
                DB	')'	
                DB	0	
                DB	13H	
                DB	"hex or binary"	
                DB	0	
                DB	12H	
                DB	TFN	
                DB	'/'	
                DB	TPROC	
                DB	0	
                DB	13H	
                DB	"call"	
                DB	0	
                DB	13H	
                DB	"arguments"	
                DB	0	
                DB	19H	
                DB	TFOR	
                DB	1AH	
                DB	0	
                DB	"Can't match "	
                DB	TFOR	
                DB	0	
                DB	13H	
                DB	TFOR	
                DB	' '	
                DB	15H	
                DW	0	
                DB	11H	
                DB	TTO	
                DW	0	
                DB	17H	
                DB	TGOSUB	
                DB	0	
                DB	TON	
                DB	" syntax"	
                DB	0	
                DB	TON	
                DB	14H	
                DB	0	
                DB	12H	
                DB	"line"	
                DB	0	
                DB	16H	
                DB	' '	
                DB	TDATA	
                DB	0	
                DB	19H	
                DB	TREPEAT	
                DB	1AH	
                DB	0	
                DB	13H	
                DB	TEXIT	
                DB	0	
                DB	11H	
                DB	'#'	
                DB	0	
                DB	19H		;46 Not in a WHILE loop	
                DB	TWHILE	
                DB	1AH	
                DB	0	
                DB	11H		;47 Missing ENDCASE	
                DB	TENDCASE	
                DB	0	
                DB	TOF		;48 OF not last	
                DB	1BH	
                DB	"last"	
                DB	0	
                DB	11H		;49 Missing ENDIF	
                DB	TENDIF	
                DB	0	
                DW	0	
                DB	0	
                DB	TON		;53 ON ERROR not LOCAL	
                DB	' '	
                DB	TERROR	
                DB	1BH	
                DB	TLOCAL	
                DB	0	
                DB	TDATA		;54 DATA not LOCAL	
                DB	1BH	
                DB	TLOCAL	
                DB	0	
;
;Indent tokens (first four needn't be at start of line):
;
TOKADD:         DB	TFOR	
                DB	TREPEAT	
                DB	TWHILE	
                DB	TCASE	
                DB	TELSE	
                DB	TWHEN	
                DB	TOTHERWISE	
LENADD:         EQU	$-TOKADD	
;
;Outdent tokens (first three needn't be at start of line):
;
TOKSUB:         DB	TNEXT	
                DB	TUNTIL	
                DB	TENDWHILE	
                DB	TENDCASE	
                DB	TENDIF	
                DB	TELSE	
                DB	TWHEN	
                DB	TOTHERWISE	
LENSUB:         EQU	$-TOKSUB	
;
;COMMANDS:
;
;DELETE line,line
;
DELETE:         CALL	DLPAIR	
DELET1:         LD	A,(HL)	
                OR	A	
                JR	Z,WARMNC	
                INC	HL	
                LD	E,(HL)	
                INC	HL	
                LD	D,(HL)	
                DEC	HL	
                DEC	HL	
                EX	DE,HL	
                SCF	
                SBC	HL,BC	
                EX	DE,HL	
                JR	NC,WARMNC	
                PUSH	BC	
                CALL	DEL	
                POP	BC	
                JR	DELET1	
;
;LISTO expr
;
LISTO:          INC	IY		;SKIP "O"	
                CALL	EXPRI	
                EXX	
                LD	A,L	
                LD	(LISTON),A	
                JP	CLOOP	
;
;LIST
;LIST line
;LIST line,line [IF string]
;LIST ,line
;LIST line,
;
LIST:           CP	'O'	
                JR	Z,LISTO	
                LD	C,1	
                LD	DE,BUFFER	
                CALL	LEXAN2	
                LD	(DE),A	
                LD	IY,BUFFER	
                CALL	DLPAIR	
                CALL	NXT	
                CP	TIF		;IF CLAUSE ?	
                LD	A,0		;INIT IF-CLAUSE LENGTH	
                JR	NZ,LISTB	
                INC	IY		;SKIP IF	
                CALL	NXT		;SKIP SPACES (IF ANY)	
                EX	DE,HL	
                PUSH	IY	
                POP	HL		;HL ADDRESSES IF CLAUSE	
                LD	A,CR	
                PUSH	BC	
                LD	BC,256	
                CPIR			;LOCATE CR	
                LD	A,C	
                CPL			;A = SUBSTRING LENGTH	
                POP	BC	
                EX	DE,HL	
LISTB:          LD	E,A		;IF-CLAUSE LENGTH	
                LD	A,B	
                OR	C	
                JR	NZ,LISTA	
                DEC	BC	
LISTA:          EXX	
                LD	IX,LISTON	
                LD	E,0		;INDENTATION COUNT	
                EXX	
                LD	A,20	
;
LISTC:          PUSH	BC		;SAVE HIGH LINE NUMBER	
                PUSH	DE		;SAVE IF-CLAUSE LENGTH	
                PUSH	HL		;SAVE PROGRAM POINTER	
                EX	AF,AF'	
                LD	A,(HL)	
                OR	A	
                JR	Z,WARMNC	
;
;CHECK IF PAST TERMINATING LINE NUMBER:
;
                LD	A,E		;A = IF-CLAUSE LENGTH	
                INC	HL	
                LD	E,(HL)	
                INC	HL	
                LD	D,(HL)		;DE = LINE NUMBER	
                DEC	HL	
                DEC	HL	
                PUSH	DE		;SAVE LINE NUMBER	
                EX	DE,HL	
                SCF	
                SBC	HL,BC	
                EX	DE,HL	
                POP	DE		;RESTORE LINE NUMBER	
WARMNC:         JP	NC,WARM	
                LD	C,(HL)		;C = LINE LENGTH + 4	
                LD	B,A		;B = IF-CLAUSE LENGTH	
;
;CHECK FOR IF CLAUSE:
;
                INC	HL	
                INC	HL	
                INC	HL		;HL ADDRESSES LINE TEXT	
                DEC	C	
                DEC	C	
                DEC	C	
                DEC	C		;C = LINE LENGTH	
                PUSH	DE		;SAVE LINE NUMBER	
                PUSH	HL		;SAVE LINE ADDRESS	
                XOR	A		;A <- 0	
                CP	B		;WAS THERE AN IF-CLAUSE	
                PUSH	IY	
                POP	DE		;DE ADDRESSES IF-CLAUSE	
                CALL	NZ,SEARCH	;SEARCH FOR IF CLAUSE	
                POP	HL		;RESTORE LINE ADDRESS	
                POP	DE		;RESTORE LINE NUMBER	
                PUSH	IY	
                CALL	Z,LISTIT	;LIST IF MATCH	
                POP	IY	
;
                EX	AF,AF'	
                DEC	A	
                CALL	LTRAP	
                POP	HL		;RESTORE POINTER	
                LD	E,(HL)	
                LD	D,0	
                ADD	HL,DE		;ADDRESS NEXT LINE	
                POP	DE		;RESTORE IF-CLAUSE LEN	
                POP	BC		;RESTORE HI LINE NUMBER	
                JR	LISTC	
;
;RENUMBER
;RENUMBER start
;RENUMBER start,increment
;RENUMBER ,increment
;
RENUM:          CALL	CLEAR		;USES DYNAMIC AREA	
                CALL	PAIR		;LOAD HL,BC	
                EXX	
                LD	HL,(PAGE_)	
                LD	DE,(LOMEM)	
RENUM1:         LD	A,(HL)		;BUILD TABLE	
                OR	A	
                JR	Z,RENUM2	
                INC	HL	
                LD	C,(HL)		;OLD LINE NUMBER	
                INC	HL	
                LD	B,(HL)	
                EX	DE,HL	
                LD	(HL),C	
                INC	HL	
                LD	(HL),B	
                INC	HL	
                EXX	
                PUSH	HL	
                ADD	HL,BC		;ADD INCREMENT	
                JP	C,TOOBIGmn	;"Too big"	
                EXX	
                POP	BC	
                LD	(HL),C	
                INC	HL	
                LD	(HL),B	
                INC	HL	
                EX	DE,HL	
                DEC	HL	
                DEC	HL	
                XOR	A	
                LD	B,A	
                LD	C,(HL)	
                ADD	HL,BC		;NEXT LINE	
                EX	DE,HL	
                PUSH	HL	
                INC	H	
                SBC	HL,SP	
                POP	HL	
                EX	DE,HL	
                JR	C,RENUM1	;CONTINUE	
                JP	ERROR_		;'No room' (A = 0)	
;
RENUM2:         EX	DE,HL	
                LD	(HL),-1	
                INC	HL	
                LD	(HL),-1	
                LD	DE,(LOMEM)	
                EXX	
                LD	HL,(PAGE_)	
RENUM3:         LD	C,(HL)	
                LD	A,C	
                OR	A	
                JR	Z,WARMNC	
                EXX	
                EX	DE,HL	
                INC	HL	
                INC	HL	
                LD	E,(HL)	
                INC	HL	
                LD	D,(HL)	
                INC	HL	
                PUSH	DE	
                EX	DE,HL	
                EXX	
                POP	DE	
                INC	HL	
                LD	(HL),E		;NEW LINE NUMBER	
                INC	HL	
                LD	(HL),D	
                INC	HL	
                DEC	C	
                DEC	C	
                DEC	C	
                LD	B,0	
RENUM7:         LD	A,TLINO	
                CPIR			;SEARCH FOR LINE NUMBER	
                JR	NZ,RENUM3	
                PUSH	BC	
                PUSH	HL	
                PUSH	HL	
                POP	IY	
                EXX	
                PUSH	HL	
                CALL	DECODE		;DECODE LINE NUMBER	
                POP	HL	
                EXX	
                LD	B,H	
                LD	C,L	
                LD	HL,(LOMEM)	
RENUM4:         LD	E,(HL)		;CROSS-REFERENCE TABLE	
                INC	HL	
                LD	D,(HL)	
                INC	HL	
                EX	DE,HL	
                OR	A		;CLEAR CARRY	
                SBC	HL,BC	
                EX	DE,HL	
                LD	E,(HL)		;NEW NUMBER	
                INC	HL	
                LD	D,(HL)	
                INC	HL	
                JR	C,RENUM4	
                EX	DE,HL	
                JR	Z,RENUM5	;FOUND	
                CALL	TELL	
                DB	"Failed at "	
                DB	0	
                EXX	
                PUSH	HL	
                EXX	
                POP	HL	
                CALL	PBCDL	
                CALL	CRLF	
                JR	RENUM6	
RENUM5:         POP	DE	
                PUSH	DE	
                DEC	DE	
                CALL	ENCODE		;RE-WRITE NUMBER	
RENUM6:         POP	HL	
                POP	BC	
                JR	RENUM7	
;
;AUTO
;AUTO start,increment
;AUTO start
;AUTO ,increment
;
AUTO:           CALL	PAIR	
                LD	(AUTONO),HL	
                LD	A,C	
                LD	(INCREM),A	
                JR	CLOOP0	
;
;BAD
;NEW
;
BAD:            CALL	TELL		;"Bad program'	
                DB	13H	
                DB	"program"	
                DB	CR	
                DB	LF	
                DB	0	
NEW:            CALL	NEWIT	
                JR	CLOOP0	
;
;LOAD filename
;
LOAD:           CALL	EXPRS		;GET FILENAME	
                LD	A,CR	
                LD	(DE),A	
                CALL	LOAD0	
                CALL	CLEAR	
                JR	WARM0	
;
;OLD
;
OLD:            LD	HL,(PAGE_)	
                PUSH	HL	
                INC	HL	
                INC	HL	
                INC	HL	
                LD	BC,252	
                LD	A,CR	
                CPIR	
                JR	NZ,BAD	
                LD	A,L	
                POP	HL	
                LD	(HL),A	
                CALL	CLEAN	
CLOOP0:         JP	CLOOP	
;
;SAVE filename
;
SAVE:           CALL	EXPRS		;FILENAME	
                LD	A,CR	
                LD	(DE),A	
                LD	DE,(PAGE_)	
                CALL	GETTOP	
                OR	A	
                SBC	HL,DE	
                LD	B,H		;LENGTH OF PROGRAM	
                LD	C,L	
                LD	HL,ACCS	
                CALL	OSSAVE	
WARM0:          JP	WARM	
;
;ERROR
;N.B. CARE NEEDED BECAUSE SP MAY NOT BE VALID (E.G. ABOVE HIMEM)
;
ERROR_:         LD	HL,ERRWDS	
                LD	C,A	
                OR	A	
                JR	Z,ERROR1	
                LD	B,A		;ERROR NUMBER	
                XOR	A	
ERROR0:         CP	(HL)	
                INC	HL	
                JR	NZ,ERROR0	
                DJNZ	ERROR0	
                JR	ERROR1		;MUST NOT PUSH HL HERE	
;
EXTERR:         POP	HL	
                LD	C,A	
ERROR1:         LD	(ERRTXT),HL	
                LD	HL,(ONERSP)	
                LD	A,H	
                OR	L	
                LD	SP,(HIMEM)	;MUST SET SP BEFORE 'CALL'	
                JR	Z,ERROR4	
                LD	SP,HL	
ERROR4:         LD	A,C		;ERROR NUMBER	
                CALL	SETLIN		;SP IS SET NOW	
                LD	(ERR),A	
                LD	(ERL),HL	
                OR	A	
                JR	Z,ERROR2	;'FATAL' ERROR	
                LD	HL,(ERRTRP)	
                LD	A,H	
                OR	L	
                PUSH	HL	
                POP	IY	
                JP	NZ,XEQ		;ERROR TRAPPED	
ERROR2:         LD	SP,(HIMEM)	
                SBC	HL,HL	
                LD	(AUTONO),HL	
                LD	(TRACEN),HL	;CANCEL TRACE	
                CALL	RESET		;RESET OPSYS	
                CALL	CRLF	
                CALL	REPORT		;MESSAGE	
                LD	HL,(ERL)	
                CALL	SAYLN	
                LD	E,0	
                CALL	C,OSSHUT	;CLOSE ALL FILES	
                CALL	CRLF	
                JR	CLOOP0	
;
;SUBROUTINES:
;
;
;LEX - SEARCH FOR KEYWORDS
;   Inputs: HL = start of keyword table
;           IY = start of match text
;  Outputs: If found, Z-flag set, A=token.
;           If not found, Z-flag reset, A=(IY).
;           IY updated (if NZ, IY unchanged).
; Destroys: A,B,H,L,IY,F
;
LEX:            LD	HL,KEYWDS	
LEX0:           LD	A,(IY)	
                LD	B,(HL)	
                INC	HL	
                CP	(HL)	
                JR	Z,LEX2	
                RET	C		;FAIL EXIT	
LEX1:           INC	HL	
                LD	A,(HL)	
                CP	160	
                JP	PE,LEX1	
                JR	LEX0	
;
LEX2:           PUSH	IY		;SAVE POINTER	
LEX3:           INC	HL	
                LD	A,(HL)	
                CP	160	
                JP	PO,LEX6		;FOUND	
                INC	IY	
                LD	A,(IY)	
                CP	(HL)	
                JR	NZ,LEX7	
                CP	161	
                JP	PE,LEX3	
LEX7:           LD	A,(IY)	
                CP	'.'	
                JR	Z,LEX6		;FOUND (ABBREV.)	
                CALL	RANGE1	
                JR	C,LEX5	
LEX4:           POP	IY		;RESTORE POINTER	
                JR	LEX1	
;
LEX5:           LD	A,(HL)	
                CP	' '	
                JR	NZ,LEX4	
                DEC	IY	
LEX6:           POP	AF	
                XOR	A	
                LD	A,B	
                RET	
;
;DEL - DELETE A PROGRAM LINE.
;   Inputs: HL addresses program line.
; Destroys: B,C,F
;
DEL:            PUSH	DE	
                PUSH	HL	
                PUSH	HL	
                LD	B,0	
                LD	C,(HL)	
                ADD	HL,BC	
                PUSH	HL	
                EX	DE,HL	
                CALL	GETTOP	
                SBC	HL,DE	
                LD	B,H	
                LD	C,L	
                POP	HL	
                POP	DE	
                LDIR			;DELETE LINE	
                POP	HL	
                POP	DE	
                RET	
;
;LOAD0 - LOAD A DISK FILE THEN CLEAN.
;   Inputs: Filename in ACCS (term CR)
; Destroys: A,B,C,D,E,H,L,F
;
;CLEAN - CHECK FOR BAD PROGRAM, FIND END OF TEXT
; AND WRITE FF FF.
; Destroys: A,B,C,H,L,F
;
LOAD0:          LD	DE,(PAGE_)	
                LD	HL,-256	
                ADD	HL,SP	
                SBC	HL,DE		;FIND AVAILABLE SPACE	
                LD	B,H	
                LD	C,L	
                LD	HL,ACCS	
                CALL	OSLOAD		;LOAD	
                CALL	NC,NEWIT	
                LD	A,0	
                JP	NC,ERROR_	;"No room"	
CLEAN:          CALL	GETTOP	
                DEC	HL	
                LD	(HL),-1		;WRITE &FFFF	
                DEC	HL	
                LD	(HL),-1	
                JR	CLEAR	
;
GETTOP:         LD	HL,(PAGE_)	
                LD	B,0	
                LD	A,CR	
GETOP1:         LD	C,(HL)	
                INC	C	
                DEC	C	
                JR	Z,GETOP2	
                ADD	HL,BC	
                DEC	HL	
                CP	(HL)	
                INC	HL	
                JR	Z,GETOP1	
                JP	BAD	
GETOP2:         INC	HL		;N.B. CALLED FROM NEWIT	
                INC	HL	
                INC	HL	
                RET	
;
;NEWIT - NEW PROGRAM THEN CLEAR
;   Destroys: H,L
;
;CLEAR - CLEAR ALL DYNAMIC VARIABLES INCLUDING
; FUNCTION AND PROCEDURE POINTERS.
;   Destroys: Nothing
;
NEWIT:          LD	HL,(PAGE_)	
                LD	(HL),0	
CLEAR:          PUSH	HL	
                PUSH	BC	
                PUSH	AF	
                CALL	GETTOP	
                LD	(LOMEM),HL	
                LD	(FREE),HL	
                LD	HL,DYNVAR	
                ; LD	B,2*(54+2)	
                LD	B,54+2*2
CLEAR1:         LD	(HL),0	
                INC	HL	
                DJNZ	CLEAR1	
                POP	AF	
                POP	BC	
                POP	HL	
                RET	
;
;LISTIT - LIST A PROGRAM LINE.
;    Inputs: HL addresses line
;            DE = line number (binary)
;	     E' = indentation count
;            IX addresses LISTON
;  Destroys: A,D,E,B',C',D',E',H',L',IY,F
;
LISTIT:         PUSH	HL	
                EX	DE,HL	
                PUSH	BC	
                CALL	PBCD	
                POP	BC	
                POP	HL	
                LD	A,(HL)	
                EXX	
                LD	HL,TOKSUB	
                LD	BC,LENSUB	
                CPIR	
                CALL	Z,INDSUB	
                CP	TENDCASE	
                CALL	Z,INDSUB	
                LD	A,' '	
                BIT	0,(IX)	
                CALL	NZ,OUTCHR	
                LD	A,E	
                ADD	A,A	
                BIT	1,(IX)	
                CALL	NZ,SPACES	
                EXX	
                LD	A,(HL)	
                LD	E,0	
                EXX	
                LD	BC,LENADD	
LIST5:          LD	HL,TOKADD	
                CPIR	
                CALL	Z,INDADD	
                CP	TCASE	
                CALL	Z,INDADD	
                EXX	
LIST8:          LD	A,(HL)	
                INC	HL	
                CP	CR	
                JR	Z,LIST9	
                LD	D,A	
                CP	TEXIT	
                JR	NZ,LIST6	
                SET	7,E	
LIST6:          CP	'"'	
                JR	NZ,LIST7	
                INC	E	
LIST7:          CALL	LOUT	
                LD	A,E	
                AND	81H	
                JR	NZ,LIST8	
                LD	A,(HL)	
                EXX	
                LD	HL,TOKSUB	
                LD	BC,3	
                CPIR	
                CALL	Z,INDSUB	
                LD	C,4	
                JR	LIST5	
;
LIST9:          LD	A,D	
                CP	TTHEN	
                EXX	
                CALL	Z,INDADD	
                EXX	
                JR	CRLF	
;
PRLINO:         PUSH	HL	
                POP	IY	
                PUSH	BC	
                CALL	DECODE	
                POP	BC	
                EXX	
                PUSH	BC	
                PUSH	DE	
                CALL	PBCDL	
                POP	DE	
                POP	BC	
                EXX	
                PUSH	IY	
                POP	HL	
                RET	
;
LOUT:           BIT	0,E	
                JR	NZ,OUTCHR	
                CP	TLINO	
                JR	Z,PRLINO	
                CALL	OUT	
                RET	
;
INDSUB:         DEC	E	
                JP	P,INDRET	
INDADD:         INC	E	
INDRET:         RET	
;
;CRLF - SEND CARRIAGE RETURN, LINE FEED.
;  Destroys: A,F
;OUTCHR - OUTPUT A CHARACTER TO CONSOLE.
;    Inputs: A = character
;  Destroys: A,F
;
CRLF:           LD	A,CR	
                CALL	OUTCHR	
                LD	A,LF	
OUTCHR:         CALL	OSWRCH	
                SUB	CR	
                JR	Z,CARRET	
                RET	C		;NON-PRINTING	
                LD	A,(COUNT)	
                INC	A	
CARRET:         LD	(COUNT),A	
                RET	Z	
                PUSH	HL	
                LD	HL,(WIDTH)	
                CP	L	
                POP	HL	
                RET	NZ	
                JR	CRLF	
;
;OUT - SEND CHARACTER OR KEYWORD
;   Inputs: A = character (>=10, <128)
;           A = Token (<10, >=128)
;  Destroys: A,F
;
OUT:            CP	160	
                JP	PE,OUTCHR	
                PUSH	BC	
                PUSH	HL	
                LD	HL,KEYWDS	
                LD	BC,KEYWDL	
                CPIR	
                CALL	NZ,OUTCHR	
                LD	B,160	
                CP	145	
                JP	PE,TOKEN1	
                INC	B	
TOKEN1:         LD	A,(HL)	
                INC	HL	
                CP	B	
                PUSH	AF	
                CALL	PE,OUTCHR	
                POP	AF	
                JP	PE,TOKEN1	
                POP	HL	
                POP	BC	
                RET	
;
;FINDL - FIND PROGRAM LINE.
;   Inputs: HL = line number (binary)
;  Outputs: HL addresses line (if found)
;           DE = line number
;           Z-flag set if found.
; Destroys: A,B,C,D,E,H,L,F
;
FINDL:          EX	DE,HL	
                LD	HL,(PAGE_)	
                XOR	A		;A=0	
                CP	(HL)	
                INC	A	
                RET	NC	
                XOR	A		;CLEAR CARRY	
                LD	B,A	
FINDL1:         LD	C,(HL)	
                PUSH	HL	
                INC	HL	
                LD	A,(HL)	
                INC	HL	
                LD	H,(HL)	
                LD	L,A	
                SBC	HL,DE	
                POP	HL	
                RET	NC		;FOUND | PAST	
                ADD	HL,BC	
                JR	FINDL1	
;
;SETLIN - Search program for line containing address.
;   Inputs: Address in (CURLIN)
;  Outputs: Line number in HL
; Destroys: B,C,D,E,H,L,F
;
SETLIN:         LD	B,0	
                LD	DE,(CURLIN)	
                LD	HL,(PAGE_)	
                OR	A	
                SBC	HL,DE	
                ADD	HL,DE	
                JR	NC,SET3	
SET1:           LD	C,(HL)	
                INC	C	
                DEC	C	
                JR	Z,SET3	
                ADD	HL,BC	
                SBC	HL,DE	
                ADD	HL,DE	
                JR	C,SET1	
                SBC	HL,BC	
                INC	HL	
                LD	E,(HL)		;LINE NUMBER	
                INC	HL	
                LD	D,(HL)	
                EX	DE,HL	
SET2:           RET	
;
SET3:           LD	HL,0	
                JR	SET2	
;
;SAYLN - PRINT " at line nnnn" MESSAGE.
;   Inputs: HL = line number
;  Outputs: Carry=0 if line number is zero.
;           Carry=1 if line number is non-zero.
; Destroys: A,B,C,D,E,H,L,F
;
SAYLN:          LD	A,H	
                OR	L	
                RET	Z	
                CALL	TELL	
                DB	" at line "	
                DB	0	
PBCDL:          LD	C,0	
                JR	PBCD0	
;
;PBCD - PRINT NUMBER AS DECIMAL INTEGER.
;   Inputs: HL = number (binary).
;  Outputs: Carry = 1
; Destroys: A,B,C,D,E,H,L,F
;
PBCD:           LD	C,' '	
PBCD0:          LD	B,5	
                LD	DE,10000	
PBCD1:          XOR	A	
PBCD2:          SBC	HL,DE	
                INC	A	
                JR	NC,PBCD2	
                ADD	HL,DE	
                DEC	A	
                JR	Z,PBCD3	
                SET	4,C	
                SET	5,C	
PBCD3:          OR	C	
                CALL	NZ,OUTCHR	
                LD	A,B	
                CP	5	
                JR	Z,PBCD4	
                ADD	HL,HL	
                LD	D,H	
                LD	E,L	
                ADD	HL,HL	
                ADD	HL,HL	
                ADD	HL,DE	
PBCD4:          LD	DE,1000	
                DJNZ	PBCD1	
                SCF	
                RET	
;
;HANDLE WHOLE ARRAY:
;
GETV1:          INC	IY	
                INC	IY		;SKIP ()	
                PUSH	HL		;SET EXIT CONDITIONS	
                POP	IX	
                LD	A,D	
                OR	64		;FLAG ARRAY	
                CP	A	
                RET	
;
;PUTVAR - CREATE VARIABLE AND INITIALISE TO ZERO.
;   Inputs: HL, IY as returned from GETVAR (NZ).
;  Outputs: As GETVAR.
; Destroys: everything
;
PUTVAR:         CALL	CREATE	
                LD	A,(IY)	
                CP	'('	
                JR	NZ,GETVZ	;SET EXIT CONDITIONS	
                LD	A,(IY+1)	
                CP	')'		;WHOLE ARRAY?	
                JR	Z,GETV1	
ARRAY:          LD	A,14		;'Bad use of array'	
ERROR3:         JP	ERROR_	
;
;GETVAR - GET LOCATION OF VARIABLE, RETURN IN HL & IX
;   Inputs: IY addresses first character.
;  Outputs: Carry set and NZ if illegal character.
;           Z-flag set if variable found, then:
;            A = variable type (0,4,5,128 or 129)
;                (68,69 or 193 for whole array)
;            HL = IX = variable pointer.
;            IY updated
;           If Z-flag & carry reset, then:
;            HL, IY set for subsequent PUTVAR call.
; Destroys: everything
;
GETVAR:         LD	A,(IY)	
                CP	'!'	
                JR	Z,GETV5	
                CP	'?'	
                JR	Z,GETV6	
                CP	'|'	
                JR	Z,GETVF	
                CP	'$'	
                JR	Z,GETV4	
                CALL	LOCATE	
                RET	NZ	
                LD	A,(IY)	
                CP	'('		;ARRAY?	
                JR	NZ,GETVX	;EXIT	
                LD	A,(IY+1)	
                CP	')'		;WHOLE ARRAY?	
                JR	Z,GETV1	
                PUSH	DE		;SAVE TYPE	
                LD	A,(HL)	
                INC	HL	
                LD	H,(HL)	
                LD	L,A		;INDIRECT LINK	
                AND	0FEH	
                OR	H	
                JR	Z,ARRAY	
                LD	A,(HL)		;NO. OF DIMENSIONS	
                OR	A	
                JR	Z,ARRAY	
                INC	HL	
                LD	DE,0		;ACCUMULATOR	
                PUSH	AF	
                INC	IY		;SKIP (	
GETV3:          PUSH	HL	
                PUSH	DE	
                CALL	EXPRI		;SUBSCRIPT	
                EXX	
                POP	DE	
                EX	(SP),HL	
                LD	C,(HL)	
                INC	HL	
                LD	B,(HL)	
                INC	HL	
                EX	(SP),HL	
                EX	DE,HL	
                PUSH	DE	
                CALL	MUL16		;HL=HL*BC	
                POP	DE	
                ADD	HL,DE	
                EX	DE,HL	
                OR	A	
                SBC	HL,BC	
                LD	A,15	
                JR	NC,ERROR3	;"Subscript"	
                POP	HL	
                POP	AF	
                DEC	A		;DIMENSION COUNTER	
                JR	NZ,GETV2	
                CALL	BRAKET		;CLOSING BRACKET	
                POP	AF		;RESTORE TYPE	
                PUSH	HL	
                CALL	X14OR5		;DE=DE*n	
                POP	HL	
                ADD	HL,DE	
                LD	D,A		;TYPE	
                LD	A,(IY)	
GETVX:          CP	'?'	
                JR	Z,GETV9	
                CP	'!'	
                JR	Z,GETV8	
GETVZ:          PUSH	HL		;SET EXIT CONDITIONS	
                POP	IX	
                LD	A,D	
                CP	A	
                RET	
;
GETV2:          PUSH	AF	
                CALL	COMMA	
                JR	GETV3	
;
;PROCESS UNARY & BINARY INDIRECTION:
;
GETV5:          LD	A,4		;UNARY 32-BIT INDIRN.	
                JR	GETV7	
GETV6:          XOR	A		;UNARY 8-BIT INDIRECTION	
                JR	GETV7	
GETVF:          LD	A,5		;VARIANT INDIRECTION	
                JR	GETV7	
GETV4:          LD	A,128		;STATIC STRING	
GETV7:          SBC	HL,HL	
                PUSH	AF	
                JR	GETV0	
;
GETV8:          LD	B,4		;32-BIT BINARY INDIRN.	
                JR	GETVA	
GETV9:          LD	B,0		;8-BIT BINARY INDIRN.	
GETVA:          PUSH	HL	
                POP	IX	
                LD	A,D		;TYPE	
                CP	129	
                RET	Z		;STRING!	
                PUSH	BC	
                CALL	LOADN		;LEFT OPERAND	
                CALL	SFIX	
                EXX	
GETV0:          PUSH	HL	
                INC	IY	
                CALL	ITEMI	
                EXX	
                POP	DE	
                POP	AF	
                ADD	HL,DE	
                PUSH	HL	
                POP	IX	
                CP	A	
                RET	
;
;GETDEF - Find entry for FN or PROC in dynamic area.
;   Inputs: IY addresses byte following "DEF" token.
;  Outputs: Z flag set if found
;           Carry set if neither FN or PROC first.
;           If Z: HL points to entry
;                 IY addresses delimiter
; Destroys: A,D,E,H,L,IY,F
;
GETDEF:         LD	A,(IY+1)	
                CALL	RANGE1	
                RET	C	
                LD	A,(IY)	
                LD	HL,FNPTR	
                CP	TFN	
                JR	Z,LOC2	
                LD	HL,PROPTR	
                CP	TPROC	
                JR	Z,LOC2	
                SCF	
                RET	
;
;LOCATE - Try to locate variable name in static or
;dynamic variables.  If illegal first character return
;carry, non-zero.  If found, return no-carry, zero.
;If not found, return no-carry, non-zero.
;   Inputs: IY addresses first character of name.
;           A=(IY)
;  Outputs: Z-flag set if found, then:
;            IY addresses terminator
;            HL addresses location of variable
;            D=type of variable:  4 = integer
;                                 5 = floating point
;                               129 = string
; Destroys: A,D,E,H,L,IY,F
;
LOCATE:         SUB	'@'	
                RET	C	
                LD	H,0	
                CP	'Z'-'@'+1	
                JR	NC,LOC0		;NOT STATIC	
                ADD	A,A	
                LD	L,A	
                LD	A,(IY+1)	;2nd CHARACTER	
                CP	'%'	
                JR	NZ,LOC1		;NOT STATIC	
                LD	A,(IY+2)	
                CP	'('	
                JR	Z,LOC1		;NOT STATIC	
                ADD	HL,HL	
                LD	DE,STAVAR	;STATIC VARIABLES	
                ADD	HL,DE	
                INC	IY	
                INC	IY	
                LD	D,4		;INTEGER TYPE	
                XOR	A	
                RET	
;
LOC0:           CP	'_'-'@'	
                RET	C	
                CP	'z'-'@'+1	
                CCF	
                DEC	A		;SET NZ	
                RET	C	
                SUB	3	
                ADD	A,A	
                LD	L,A	
LOC1:           LD	DE,DYNVAR	;DYNAMIC VARIABLES	
                DEC	L	
                DEC	L	
                SCF	
                RET	M	
                ADD	HL,DE	
LOC2:           LD	E,(HL)	
                INC	HL	
                LD	D,(HL)	
                LD	A,D	
                OR	E	
                JR	Z,LOC6		;UNDEFINED VARIABLE	
                LD	H,D	
                LD	L,E	
                INC	HL		;SKIP LINK	
                INC	HL	
                PUSH	IY	
LOC3:           LD	A,(HL)		;COMPARE	
                INC	HL	
                INC	IY	
                CP	(IY)	
                JR	Z,LOC3	
                OR	A		;0=TERMINATOR	
                JR	Z,LOC5		;FOUND (MAYBE)	
LOC4:           POP	IY	
                EX	DE,HL	
                JR	LOC2		;TRY NEXT ENTRY	
;
LOC5:           DEC	IY	
                LD	A,(IY)	
                CP	'('	
                JR	Z,LOCX		;FOUND	
                INC	IY	
                CALL	RANGE	
                JR	C,LOCX		;FOUND	
                CP	'('	
                JR	Z,LOC4		;KEEP LOOKING	
                LD	A,(IY-1)	
                CALL	RANGE1	
                JR	NC,LOC4		;KEEP LOOKING	
LOCX:           POP	DE	
TYPE:           LD	A,(IY-1)	
                CP	'$'	
                LD	D,129	
                RET	Z		;STRING	
                CP	'&'	
                LD	D,1	
                RET	Z		;BYTE	
                CP	'%'	
                LD	D,4	
                RET	Z		;INTEGER	
                INC	D	
                CP	A	
                RET	
;
LOC6:           INC	A		;SET NZ	
                RET	
;
;CREATE - CREATE NEW ENTRY, INITIALISE TO ZERO.
;   Inputs: HL, IY as returned from LOCATE (NZ).
;  Outputs: As LOCATE, GETDEF.
; Destroys: As LOCATE, GETDEF.
;
CREATE:         XOR	A	
                LD	DE,(FREE)	
                LD	(HL),D	
                DEC	HL	
                LD	(HL),E	
                EX	DE,HL	
                LD	(HL),A	
                INC	HL	
                LD	(HL),A	
                INC	HL	
LOC7:           INC	IY	
                CALL	RANGE		;END OF VARIABLE?	
                JR	C,LOC8	
                LD	(HL),A	
                INC	HL	
                CALL	RANGE1	
                JR	NC,LOC7	
                CP	'('	
                JR	Z,LOC8	
                LD	A,(IY+1)	
                CP	'('	
                JR	Z,LOC7	
                INC	IY	
LOC8:           LD	(HL),0		;TERMINATOR	
                INC	HL	
                PUSH	HL	
                CALL	TYPE	
                LD	A,(IY)	
                CP	'('	
                LD	A,2		;SIZE OF INDIRECT LINK	
                JR	Z,LOC9	
                LD	A,D	
                OR	A		;STRING?	
                JP	P,LOC9	
                LD	A,4	
LOC9:           LD	(HL),0		;INITIALISE TO ZERO	
                INC	HL	
                DEC	A	
                JR	NZ,LOC9	
                LD	(FREE),HL	
                CALL	CHECK	
                POP	HL	
                XOR	A	
                RET	
;
;LINNUM - GET LINE NUMBER FROM TEXT STRING
;   Inputs: IY = Text Pointer
;  Outputs: HL = Line number (zero if none)
;           IY updated
; Destroys: A,D,E,H,L,IY,F
;
LINNUM:         CALL	NXT	
                LD	HL,0	
LINNM1:         LD	A,(IY)	
                SUB	'0'	
                RET	C	
                CP	10	
                RET	NC	
                INC	IY	
                LD	D,H	
                LD	E,L	
                ADD	HL,HL		;*2	
                JR	C,TOOBIGmn	
                ADD	HL,HL		;*4	
                JR	C,TOOBIGmn	
                ADD	HL,DE		;*5	
                JR	C,TOOBIGmn	
                ADD	HL,HL		;*10	
                JR	C,TOOBIGmn	
                LD	E,A	
                LD	D,0	
                ADD	HL,DE		;ADD IN DIGIT	
                JR	NC,LINNM1	
TOOBIGmn:         LD	A,20	
                JP	ERROR_		;"Too big"	
;
;PAIR - GET PAIR OF LINE NUMBERS FOR RENUMBER/AUTO.
;   Inputs: IY = text pointer
;  Outputs: HL = first number (10 by default)
;           BC = second number (10 by default)
; Destroys: A,B,C,D,E,H,L,B',C',D',E',H',L',IY,F
;
PAIR:           CALL	LINNUM		;FIRST	
                LD	A,H	
                OR	L	
                JR	NZ,PAIR1	
                LD	L,10	
PAIR1:          CALL	TERMQ	
                INC	IY	
                PUSH	HL	
                LD	HL,10	
                CALL	NZ,LINNUM	;SECOND	
                EX	(SP),HL	
                POP	BC	
                LD	A,B	
                OR	C	
                RET	NZ	
                CALL	EXTERR	
                DB	"Silly"	
                DB	0	
;
;DLPAIR - GET PAIR OF LINE NUMBERS FOR DELETE/LIST.
;   Inputs: IY = text pointer
;  Outputs: HL = points to program text
;           BC = second number (0 by default)
; Destroys: A,B,C,D,E,H,L,IY,F
;
DLPAIR:         CALL	LINNUM	
                PUSH	HL	
                CALL	TERMQ	
                JR	Z,DLP1	
                CP	TIF	
                JR	Z,DLP1	
                INC	IY	
                CALL	LINNUM	
DLP1:           EX	(SP),HL	
                CALL	FINDL	
                POP	BC	
                RET	
;
;TEST FOR VALID CHARACTER IN VARIABLE NAME:
;   Inputs: IY addresses character
;  Outputs: Carry set if out-of-range.
; Destroys: A,F
;
RANGE:          LD	A,(IY)	
                CP	'$'	
                RET	C	
                CP	'&'+1	
                CCF	
                RET	NC	
                CP	'('	
                RET	Z	
RANGE1:         CP	'0'	
                RET	C	
                CP	'9'+1	
                CCF	
                RET	NC	
                CP	'@'		;V2.4	
                RET	Z	
RANGE2:         CP	'A'	
                RET	C	
                CP	'Z'+1	
                CCF	
                RET	NC	
                CP	'_'	
                RET	C	
                CP	'z'+1	
                CCF	
                RET	
;
;LEXAN - LEXICAL ANALYSIS.
;  Bit 0,C: 1=left, 0=right
;  Bit 3,C: 1=in HEX
;  Bit 4,C: 1=accept line number
;  Bit 5,C: 1=in variable, FN, PROC
;  Bit 6,C: 1=in REM, DATA, *
;  Bit 7,C: 1=in quotes
;   Inputs: IY addresses source string
;           DE addresses destination string
;           (must be page boundary)
;           C  sets initial mode
;  Outputs: DE, IY updated
;           A holds carriage return
;
LEXAN1:         LD	(DE),A		;TRANSFER TO BUFFER	
                INC	DE		;INCREMENT POINTERS	
                INC	IY	
LEXAN2:         LD	A,E		;MAIN ENTRY	
                CP	252		;TEST LENGTH	
                LD	A,19	
                JP	NC,ERROR_	;'String too long'	
                LD	A,(IY)	
                CP	CR	
                RET	Z		;END OF LINE	
                CALL	RANGE1	
                JR	NC,LEXAN3	
                RES	5,C		;NOT IN VARIABLE	
                RES	3,C		;NOT IN HEX	
LEXAN3:         CP	' '	
                JR	Z,LEXAN1	;PASS SPACES	
                CP	','	
                JR	Z,LEXAN1	;PASS COMMAS	
                CP	'G'	
                JR	C,LEXAN4	
                RES	3,C		;NOT IN HEX	
LEXAN4:         CP	'"'	
                JR	NZ,LEXAN5	
                RL	C	
                CCF			;TOGGLE C7	
                RR	C	
LEXAN5:         BIT	4,C	
                JR	Z,LEXAN6	
                RES	4,C	
                PUSH	BC	
                PUSH	DE	
                CALL	LINNUM		;GET LINE NUMBER	
                POP	DE	
                POP	BC	
                LD	A,H	
                OR	L	
                CALL	NZ,ENCODE	;ENCODE LINE NUMBER	
                JR	LEXAN2		;CONTINUE	
;
LEXAN6:         DEC	C	
                JR	Z,LEXAN7	;C=1 (LEFT)	
                INC	C	
                JR	NZ,LEXAN1	
                OR	A	
                CALL	P,LEX		;TOKENISE IF POSS.	
                JR	LEXAN8	
;
LEXAN7:         CP	'*'	
                JR	Z,LEXAN9	
                OR	A	
                CALL	P,LEX		;TOKENISE IF POSS.	
                CP	TOKLO	
                JR	C,LEXAN8	
                CP	TOKHI+1	
                JR	NC,LEXAN8	
                ADD	A,OFFSET	;LEFT VERSION	
LEXAN8:         CP	TREM	
                JR	Z,LEXAN9	
                CP	TDATA	
                JR	NZ,LEXANA	
LEXAN9:         SET	6,C		;QUIT TOKENISING	
LEXANA:         CP	TFN	
                JR	Z,LEXANB	
                CP	TPROC	
                JR	Z,LEXANB	
                CALL	RANGE2	
                JR	C,LEXANC	
LEXANB:         SET	5,C		;IN VARIABLE/FN/PROC	
LEXANC:         CP	'&'	
                JR	NZ,LEXAND	
                SET	3,C		;IN HEX	
LEXAND:         LD	HL,LIST1	
                PUSH	BC	
                LD	BC,LIST1L	
                CPIR	
                POP	BC	
                JR	NZ,LEXANE	
                SET	4,C		;ACCEPT LINE NUMBER	
LEXANE:         LD	HL,LIST2	
                PUSH	BC	
                LD	BC,LIST2L	
                CPIR	
                POP	BC	
                JR	NZ,LEXANF	
                SET	0,C		;ENTER LEFT MODE	
LEXANF:         JP	LEXAN1	
;
LIST1:          DB	TGOTO	
                DB	TGOSUB	
                DB	TRESTORE	
                DB	TTRACE	
LIST2:          DB	TTHEN	
                DB	TELSE	
LIST1L:         EQU	$-LIST1	
                DB	TREPEAT	
                DB	TERROR	
                DB	':'	
LIST2L:         EQU	$-LIST2	
;
;ENCODE - ENCODE LINE NUMBER INTO PSEUDO-BINARY FORM.
;   Inputs: HL=line number, DE=string pointer
;  Outputs: DE updated, BIT 4,C set.
; Destroys: A,B,C,D,E,H,L,F
;
ENCODE:         SET	4,C	
                EX	DE,HL	
                LD	(HL),TLINO	
                INC	HL	
                LD	A,D	
                AND	0C0H	
                RRCA	
                RRCA	
                LD	B,A	
                LD	A,E	
                AND	0C0H	
                OR	B	
                RRCA	
                RRCA	
                XOR	01010100B	
                LD	(HL),A	
                INC	HL	
                LD	A,E	
                AND	3FH	
                OR	'@'	
                LD	(HL),A	
                INC	HL	
                LD	A,D	
                AND	3FH	
                OR	'@'	
                LD	(HL),A	
                INC	HL	
                EX	DE,HL	
                RET	
;
;TEXT - OUTPUT MESSAGE.
;   Inputs: HL addresses text (terminated by nul)
;  Outputs: HL addresses character following nul.
; Destroys: A,H,L,F
;
REPORT:         LD	HL,(ERRTXT)	
TEXT_:          LD	A,(HL)	
                INC	HL	
                OR	A	
                RET	Z	
                CP	LF	
                JR	Z,TEXTLF	;Token for TINT	
                CALL	OUT	
                JR	TEXT_	
;
TEXTLF:         CALL	OUTCHR	
                JR	TEXT_	
;
;TELL - OUTPUT MESSAGE.
;   Inputs: Text follows subroutine call (term=nul)
; Destroys: A,F
;
TELL:           EX	(SP),HL		;GET RETURN ADDRESS	
                CALL	TEXT_	
                EX	(SP),HL	
                RET	
;
; NLIST - Check for end of list
;
NLIST:          CALL	NXT	
                CP	','		;ANOTHER VARIABLE?	
                JR	Z,NXT1	
                POP	BC		;DITCH RETURN ADDRESS	
                JP	XEQ	
;
NXT:            LD	A,(IY)	
                CP	' '	
                RET	NZ	
NXT1:           INC	IY	
                JR	NXT	
;
;                END	START	

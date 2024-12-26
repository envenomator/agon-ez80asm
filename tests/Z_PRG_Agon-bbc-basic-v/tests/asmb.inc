;
;Automatically created from original source on 2024-12-15 15:29:12
;
                .ASSUME ADL = 0	
;	.ORG 0x0000
;                SEGMENT CODE	
;
;BBC BASIC INTERPRETER - Z80 VERSION
;Z80 CPU ASSEMBLER MODULE - "ASMB"
;(C) COPYRIGHT R.T.RUSSELL 1981-2024
;
;THE NAME BBC BASIC IS USED WITH THE PERMISSION
;OF THE BRITISH BROADCASTING CORPORATION AND IS
;NOT TRANSFERRABLE TO A FORKED OR DERIVED WORK.
;
;VERSION 5.0, 14-05-2024
;
;                XDEF	ASSEM	
;
;                XREF	TABIT	
;                XREF	CRLF	
;                XREF	OUT	
;                XREF	VAR_	
;                XREF	ZERO	
;                XREF	STOREN	
;                XREF	ERROR_	
;                XREF	EXPRI	
;                XREF	EXPRS	
;
;                XREF	LISTON	
;                XREF	COUNT	
;                XREF	ACCS	
;                XREF	OC	
;                XREF	PC	
;
; CR             EQU	0DH	; in equs.inc
TAND:           EQU	80H	
TOR:            EQU	84H	
; TERROR:         EQU	85H	; in exec.asm
TCALL:          EQU	0D6H
TDEF:           EQU	0DDH 
;
;ASSEMBLER:
;LANGUAGE-INDEPENDENT CONTROL SECTION:
; Outputs: A=delimiter, carry set if syntax error.
;
ASSEM:          CALL	SKIP	
                INC	IY	
                CP	':'	
                JR	Z,ASSEM	
                CP	']'	
                RET	Z	
                CP	CR	
                RET	Z	
                DEC	IY	
                LD	IX,(PC)		;PROGRAM COUNTER	
                LD	HL,LISTON	
                BIT	6,(HL)	
                JR	Z,ASSEM0	
                LD	IX,(OC)		;ORIGIN of CODE	
ASSEM0:         PUSH	IX	
                PUSH	IY	
                CALL	ASMB	
                POP	BC	
                POP	DE	
                RET	C	
                CALL	SKIP	
                SCF	
                RET	NZ	
                DEC	IY	
ASSEM3:         INC	IY	
                LD	A,(IY)	
                CALL	TERM0	
                JR	NZ,ASSEM3	
                LD	A,(LISTON)	
                PUSH	IX	
                POP	HL	
                OR	A	
                SBC	HL,DE	
                EX	DE,HL		;DE= NO. OF BYTES	
                PUSH	HL	
                LD	HL,(PC)	
                PUSH	HL	
                ADD	HL,DE	
                LD	(PC),HL		;UPDATE PC	
                BIT	6,A	
                JR	Z,ASSEM5	
                LD	HL,(OC)	
                ADD	HL,DE	
                LD	(OC),HL		;UPDATE OC	
ASSEM5:         POP	HL		;OLD PC	
                POP	IX		;CODE HERE	
                BIT	4,A	
                JR	Z,ASSEM	
                LD	A,H	
                CALL	HEX	
                LD	A,L	
                CALL	HEXSP	
                XOR	A	
                CP	E	
                JR	Z,ASSEM2	
ASSEM1:         LD	A,(COUNT)	
                CP	17	
                LD	A,5	
                CALL	NC,TABIT	;NEXT LINE	
                LD	A,(IX)	
                CALL	HEXSP	
                INC	IX	
                DEC	E	
                JR	NZ,ASSEM1	
ASSEM2:         LD	A,18	
                CALL	TABIT	
                PUSH	IY	
                POP	HL	
                SBC	HL,BC	
ASSEM4:         LD	A,(BC)	
                CALL	OUT	
                INC	BC	
                DEC	L	
                JR	NZ,ASSEM4	
                CALL	CRLF	
                JP	ASSEM	
;
HEXSP:          CALL	HEX	
                LD	A,' '	
                JR	OUTCH1	
HEX:            PUSH	AF	
                RRCA	
                RRCA	
                RRCA	
                RRCA	
                CALL	HEXOUT	
                POP	AF	
HEXOUT:         AND	0FH	
                ADD	A,90H	
                DAA	
                ADC	A,40H	
                DAA	
OUTCH1:         JP	OUT	
;
;PROCESSOR-SPECIFIC TRANSLATION SECTION:
;
;REGISTER USAGE: B - TYPE OF MOST RECENT OPERAND
;                C - OPCODE BEING BUILT
;                D - (IX) OR (IY) FLAG
;                E - OFFSET FROM IX OR IY
;               HL - NUMERIC OPERAND VALUE
;               IX - CODE DESTINATION
;               IY - SOURCE TEXT POINTER
;   Inputs: A = initial character
;  Outputs: Carry set if syntax error.
;
ASMB:           CP	'.'	
                JR	NZ,ASMB1	
                INC	IY	
                PUSH	IX	
                CALL	VAR_	
                PUSH	AF	
                CALL	ZERO	
                EXX	
                LD	HL,(PC)	
                EXX	
                LD	A,(LISTON)	
                AND	20H	
                JR	NZ,ASMB0	
                LD	A,(IX)	
                OR	(IX+1)	
                LD	A,3	
                JP	NZ,ERROR_	;Multiple label	
ASMB0:          POP	AF	
                CALL	STOREN	
                POP	IX	
ASMB1:          CALL	SKIP	
                RET	Z	
                CP	TCALL	
                LD	C,0C4H	
                INC	IY	
                JP	Z,GRPC	
                DEC	IY	
                LD	HL,OPCODS	
                CALL	FIND	
                RET	C	
                LD	C,B	;ROOT OPCODE	
                LD	D,0	;CLEAR IX/IY FLAG	
;
;GROUP 0 - TRIVIAL CASES REQUIRING NO COMPUTATION
;GROUP 1 - AS GROUP 0 BUT WITH "ED" PREFIX
;
                SUB	39	
                JR	NC,GROUP2	
                CP	15-39	
                CALL	NC,ED	
                JR	BYTE0	
;
;GROUP 2 - BIT, RES, SET
;GROUP 3 - RLC, RRC, RL, RR, SLA, SRA, SRL
;
GROUP2:         SUB	10	
                JR	NC,GROUP4	
                CP	3-10	
                CALL	C,BIT	
                RET	C	
                CALL	REGLO	
                RET	C	
                CALL	CB	
                JR	BYTE0	
;
;GROUP 4 - PUSH, POP, EX (SP)
;
GROUP4:         SUB	3	
                JR	NC,GROUP5	
G4:             CALL	PAIRasm	
                RET	C	
                JR	BYTE0	
;
;GROUP 5 - SUB, AND, XOR, OR, CP
;GROUP 6 - ADD, ADC, SBC
;
GROUP5:         SUB	8+2	
                JR	NC,GROUP7	
                CP	5-8	
                LD	B,7	
                CALL	NC,OPND	
                LD	A,B	
                CP	7	
                JR	NZ,G6HL	
G6:             CALL	REGLO	
                LD	A,C	
                JR	NC,BIND1	
                XOR	46H	
                CALL	BIND	
DB:             CALL	NUMBER	
                JR	VAL8	
;
G6HL:           AND	3FH	
                CP	12	
                SCF	
                RET	NZ	
                LD	A,C	
                CP	80H	
                LD	C,9	
                JR	Z,G4	
                XOR	1CH	
                RRCA	
                LD	C,A	
                CALL	ED	
                JR	G4	
;
;GROUP 7 - INC, DEC
;
GROUP7:         SUB	2	
                JR	NC,GROUP8	
                CALL	REGHI	
                LD	A,C	
BIND1:          JP	NC,BIND	
                XOR	64H	
                RLCA	
                RLCA	
                RLCA	
                LD	C,A	
                CALL	PAIR1asm	
                RET	C	
BYTE0:          LD	A,C	
                JR	BYTE2	
;
;GROUP 8 - IN
;GROUP 9 - OUT
;
GROUP8:         SUB	2	
                JR	NC,GROUPA	
                CP	1-2	
                CALL	Z,CORN	
                EX	AF,AF'	
                CALL	REGHI	
                RET	C	
                EX	AF,AF'	
                CALL	C,CORN	
                INC	H	
                JR	Z,BYTE0	
                LD	A,B	
                CP	7	
                SCF	
                RET	NZ	
                LD	A,C	
                XOR	3	
                RLCA	
                RLCA	
                RLCA	
                CALL	BYTE	
                JR	VAL8	
;
;GROUP 10 - JR, DJNZ
;
GROUPA:         SUB	2	
                JR	NC,GROUPB	
                CP	1-2	
                CALL	NZ,COND_	
                LD	A,C	
                JR	NC,GRPA	
                LD	A,18H	
GRPA:           CALL	BYTE	
                CALL	NUMBER	
                LD	DE,(PC)	
                INC	DE	
                SCF	
                SBC	HL,DE	
                LD	A,L	
                RLA	
                SBC	A,A	
                CP	H	
TOOFAR:         LD	A,1	
                JP	NZ,ERROR_	;"Out of range"	
VAL8:           LD	A,L	
                JR	BYTE2	
;
;GROUP 11 - JP
;
GROUPB:         LD	B,A	
                JR	NZ,GROUPC	
                CALL	COND_	
                LD	A,C	
                JR	NC,GRPB	
                LD	A,B	
                AND	3FH	
                CP	6	
                LD	A,0E9H	
                JR	Z,BYTE2	
                LD	A,0C3H	
GRPB:           CALL	BYTE	
                JR	ADDR_	
;
;GROUP 12 - CALL
;
GROUPC:         DJNZ	GROUPD	
GRPC:           CALL	GRPE	
ADDR_:          CALL	NUMBER	
VAL16:          CALL	VAL8	
                LD	A,H	
                JR	BYTE2	
;
;GROUP 13 - RST
;
GROUPD:         DJNZ	GROUPE	
                CALL	NUMBER	
                AND	C	
                OR	H	
                JR	NZ,TOOFAR	
                LD	A,L	
                OR	C	
BYTE2:          JR	BYTE1	
;
;GROUP 14 - RET
;
GROUPE:         DJNZ	GROUPF	
GRPE:           CALL	COND_	
                LD	A,C	
                JR	NC,BYTE1	
                OR	9	
                JR	BYTE1	
;
;GROUP 15 - LD
;
GROUPF:         DJNZ	MISC	
                CALL	LDOP	
                JR	NC,LDA	
                CALL	REGHI	
                EX	AF,AF'	
                CALL	SKIP	
                CP	'('	
                JR	Z,LDIN	
                EX	AF,AF'	
                JP	NC,G6	
                LD	C,1	
                CALL	PAIR1asm	
                RET	C	
                LD	A,14	
                CP	B	
                LD	B,A	
                CALL	Z,PAIRasm	
                LD	A,B	
                AND	3FH	
                CP	12	
                LD	A,C	
                JR	NZ,GRPB	
                LD	A,0F9H	
                JR	BYTE1	
;
LDIN:           EX	AF,AF'	
                PUSH	BC	
                CALL	NC,REGLO	
                LD	A,C	
                POP	BC	
                JR	NC,BIND	
                LD	C,0AH	
                CALL	PAIR1asm	
                CALL	LD16	
                JR	NC,GRPB	
                CALL	NUMBER	
                LD	C,2	
                CALL	PAIRasm	
                CALL	LD16	
                RET	C	
                CALL	BYTE	
                JR	VAL16	
;
;OPT - SET OPTION
;
OPT:            DEC	B	
                JP	Z,DB	
                DJNZ	ADDR_	
                CALL	NUMBER	
                LD	HL,LISTON	
                LD	C,A	
                RLD	
                LD	A,C	
                RRD	
                RET	
;
LDA:            CP	4	
                CALL	C,ED	
                LD	A,B	
BYTE1:          JR	BYTE	
;
;MISC - DEFB, DEFW, DEFM
;
MISC:           DJNZ	OPT	
                PUSH	IX	
                CALL	EXPRS	
                POP	IX	
                LD	HL,ACCS	
DEFM1:          XOR	A	
                CP	E	
                RET	Z	
                LD	A,(HL)	
                INC	HL	
                CALL	BYTE	
                DEC	E	
                JR	DEFM1	
;
;SUBROUTINES:
;
LD16:           LD	A,B	
                JR	C,LD8	
                LD	A,B	
                AND	3FH	
                CP	12	
                LD	A,C	
                RET	Z	
                CALL	ED	
                LD	A,C	
                OR	43H	
                RET	
;
LD8:            CP	7	
                SCF	
                RET	NZ	
                LD	A,C	
                OR	30H	
                RET	
;
CORN:           PUSH	BC	
                CALL	OPND	
                BIT	5,B	
                POP	BC	
                JR	Z,NUMBER	
                LD	H,-1	
ED:             LD	A,0EDH	
                JR	BYTE	
;
CB:             LD	A,0CBH	
BIND:           CP	76H	
                SCF	
                RET	Z		;REJECT LD (HL),(HL)	
                CALL	BYTE	
                INC	D	
                RET	P	
                LD	A,E	
                JR	BYTE	
;
OPND:           PUSH	HL	
                LD	HL,OPRNDS	
                CALL	FIND	
                POP	HL	
                RET	C	
                BIT	7,B	
                RET	Z	
                BIT	3,B	
                PUSH	HL	
                CALL	Z,OFFSETasm	
                LD	E,L	
                POP	HL	
                LD	A,0DDH	
                BIT	6,B	
                JR	Z,OP1	
                LD	A,0FDH	
OP1:            OR	A	
                INC	D	
                LD	D,A	
                RET	M	
BYTE:           LD	(IX),A	
                INC	IX	
                OR	A	
                RET	
;
OFFSETasm:         LD	A,(IY)	
                CP	')'	
                LD	HL,0	
                RET	Z	
NUMBER:         CALL	SKIP	
                PUSH	BC	
                PUSH	DE	
                PUSH	IX	
                CALL	EXPRI	
                POP	IX	
                EXX	
                POP	DE	
                POP	BC	
                LD	A,L	
                OR	A	
                RET	
;
REG:            CALL	OPND	
                RET	C	
                LD	A,B	
                AND	3FH	
                CP	8	
                CCF	
                RET	
;
REGLO:          CALL	REG	
                RET	C	
                JR	ORC	
;
REGHI:          CALL	REG	
                RET	C	
                JR	SHL3	
;
COND_:          CALL	OPND	
                RET	C	
                LD	A,B	
                AND	1FH	
                SUB	16	
                JR	NC,SHL3	
                CP	-15	
                SCF	
                RET	NZ	
                LD	A,3	
                JR	SHL3	
;
PAIRasm:           CALL	OPND	
                RET	C	
PAIR1asm:          LD	A,B	
                AND	0FH	
                SUB	8	
                RET	C	
                JR	SHL3	
;
BIT:            CALL	NUMBER	
                CP	8	
                CCF	
                RET	C	
SHL3:           RLCA	
                RLCA	
                RLCA	
ORC:            OR	C	
                LD	C,A	
                RET	
;
LDOP:           LD	HL,LDOPS	
FIND:           CALL	SKIP	
EXIT:           LD	B,0	
                SCF	
                RET	Z	
                CP	TDEF	
                JR	Z,FIND0	
                CP	TOR+1	
                CCF	
                RET	C	
FIND0:          LD	A,(HL)	
                OR	A	
                JR	Z,EXIT	
                XOR	(IY)	
                AND	01011111B	
                JR	Z,FIND2	
FIND1:          BIT	7,(HL)	
                INC	HL	
                JR	Z,FIND1	
                INC	HL	
                INC	B	
                JR	FIND0	
;
FIND2:          PUSH	IY	
FIND3:          BIT	7,(HL)	
                INC	IY	
                INC	HL	
                JR	NZ,FIND5	
                CP	(HL)	
                CALL	Z,SKIP0	
                LD	A,(HL)	
                XOR	(IY)	
                AND	01011111B	
                JR	Z,FIND3	
FIND4:          POP	IY	
                JR	FIND1	
;
FIND5:          CALL	DELIM	
                CALL	NZ,SIGN	
                JR	NZ,FIND4	
FIND6:          LD	A,B	
                LD	B,(HL)	
                POP	HL	
                RET	
;
SKIP0:          INC	HL	
SKIP:           CALL	DELIM	
                RET	NZ	
                CALL	TERM	
                RET	Z	
                INC	IY	
                JR	SKIP	
;
SIGN:           CP	'+'	
                RET	Z	
                CP	'-'	
                RET	
;
DELIM:          LD	A,(IY)		;ASSEMBLER DELIMITER	
                CP	' '	
                RET	Z	
                CP	','	
                RET	Z	
                CP	')'	
                RET	Z	
TERM:           CP	';'		;ASSEMBLER TERMINATOR	
                RET	Z	
                CP	'\'	
                RET	Z	
TERM0:          CP	':'		;ASSEMBLER SEPARATOR	
                RET	NC	
                CP	CR	
                RET	
;
OPCODS:         DB	"NO"	
                DB	'P'+80H	
                DB	0	
                DB	"RLC"	
                DB	'A'+80H	
                DB	7	
                DB	"EX"	
                DB	0	
                DB	"AF"	
                DB	0	
                DB	"AF"	
                DB	27H+80H ; APOSTROPHE	
                DB	8	
                DB	"RRC"	
                DB	'A'+80H	
                DB	0FH	
                DB	"RL"	
                DB	'A'+80H	
                DB	17H	
                DB	"RR"	
                DB	'A'+80H	
                DB	1FH	
                DB	"DA"	
                DB	'A'+80H	
                DB	27H	
                DB	"CP"	
                DB	'L'+80H	
                DB	2FH	
                DB	"SC"	
                DB	'F'+80H	
                DB	37H	
                DB	"CC"	
                DB	'F'+80H	
                DB	3FH	
                DB	"HAL"	
                DB	'T'+80H	
                DB	76H	
                DB	"EX"	
                DB	'X'+80H	
                DB	0D9H	
                DB	"EX"	
                DB	0	
                DB	"DE"	
                DB	0	
                DB	'H'	
                DB	'L'+80H	
                DB	0EBH	
                DB	'D'	
                DB	'I'+80H	
                DB	0F3H	
                DB	'E'	
                DB	'I'+80H	
                DB	0FBH	
;
                DB	"NE"	
                DB	'G'+80H	
                DB	44H	
                DB	"IM"	
                DB	0	
                DB	'0'+80H	
                DB	46H	
                DB	"RET"	
                DB	'N'+80H	
                DB	45H	
                DB	"RET"	
                DB	'I'+80H	
                DB	4DH	
                DB	"IM"	
                DB	0	
                DB	'1'+80H	
                DB	56H	
                DB	"IM"	
                DB	0	
                DB	'2'+80H	
                DB	5EH	
                DB	"RR"	
                DB	'D'+80H	
                DB	67H	
                DB	"RL"	
                DB	'D'+80H	
                DB	6FH	
                DB	"LD"	
                DB	'I'+80H	
                DB	0A0H	
                DB	"CP"	
                DB	'I'+80H	
                DB	0A1H	
                DB	"IN"	
                DB	'I'+80H	
                DB	0A2H	
                DB	"OUT"	
                DB	'I'+80H	
                DB	0A3H	
                DB	"LD"	
                DB	'D'+80H	
                DB	0A8H	
                DB	"CP"	
                DB	'D'+80H	
                DB	0A9H	
                DB	"IN"	
                DB	'D'+80H	
                DB	0AAH	
                DB	"OUT"	
                DB	'D'+80H	
                DB	0ABH	
                DB	"LDI"	
                DB	'R'+80H	
                DB	0B0H	
                DB	"CPI"	
                DB	'R'+80H	
                DB	0B1H	
                DB	"INI"	
                DB	'R'+80H	
                DB	0B2H	
                DB	"OTI"	
                DB	'R'+80H	
                DB	0B3H	
                DB	"LDD"	
                DB	'R'+80H	
                DB	0B8H	
                DB	"CPD"	
                DB	'R'+80H	
                DB	0B9H	
                DB	"IND"	
                DB	'R'+80H	
                DB	0BAH	
                DB	"OTD"	
                DB	'R'+80H	
                DB	0BBH	
;
                DB	"BI"	
                DB	'T'+80H	
                DB	40H	
                DB	"RE"	
                DB	'S'+80H	
                DB	80H	
                DB	"SE"	
                DB	'T'+80H	
                DB	0C0H	
;
                DB	"RL"	
                DB	'C'+80H	
                DB	0	
                DB	"RR"	
                DB	'C'+80H	
                DB	8	
                DB	'R'	
                DB	'L'+80H	
                DB	10H	
                DB	'R'	
                DB	'R'+80H	
                DB	18H	
                DB	"SL"	
                DB	'A'+80H	
                DB	20H	
                DB	"SR"	
                DB	'A'+80H	
                DB	28H	
                DB	"SR"	
                DB	'L'+80H	
                DB	38H	
;
                DB	"PO"	
                DB	'P'+80H	
                DB	0C1H	
                DB	"PUS"	
                DB	'H'+80H	
                DB	0C5H	
                DB	"EX"	
                DB	0	
                DB	"(S"	
                DB	'P'+80H	
                DB	0E3H	
;
                DB	"SU"	
                DB	'B'+80H	
                DB	90H	
                DB	"AN"	
                DB	'D'+80H	
                DB	0A0H	
                DB	"XO"	
                DB	'R'+80H	
                DB	0A8H	
                DB	'O'	
                DB	'R'+80H	
                DB	0B0H	
                DB	'C'	
                DB	'P'+80H	
                DB	0B8H	
                DB	TAND	
                DB	0A0H	
                DB	TOR	
                DB	0B0H	
;
                DB	"AD"	
                DB	'D'+80H	
                DB	80H	
                DB	"AD"	
                DB	'C'+80H	
                DB	88H	
                DB	"SB"	
                DB	'C'+80H	
                DB	98H	
;
                DB	"IN"	
                DB	'C'+80H	
                DB	4	
                DB	"DE"	
                DB	'C'+80H	
                DB	5	
;
                DB	'I'	
                DB	'N'+80H	
                DB	40H	
                DB	"OU"	
                DB	'T'+80H	
                DB	41H	
;
                DB	'J'	
                DB	'R'+80H	
                DB	20H	
                DB	"DJN"	
                DB	'Z'+80H	
                DB	10H	
;
                DB	'J'	
                DB	'P'+80H	
                DB	0C2H	
;
                DB	"CAL"	
                DB	'L'+80H	
                DB	0C4H	
;
                DB	"RS"	
                DB	'T'+80H	
                DB	0C7H	
;
                DB	"RE"	
                DB	'T'+80H	
                DB	0C0H	
;
                DB	'L'	
                DB	'D'+80H	
                DB	40H	
;
                DB	TDEF & 7FH	
                DB	'M'+80H	
                DB	0	
;
                DB	TDEF & 7FH	
                DB	'B'+80H	
                DB	0	
;
                DB	"OP"	
                DB	'T'+80H	
                DB	0	
;
                DB	TDEF & 7FH	
                DB	'W'+80H	
                DB	0	
;
                DB	0	
;
OPRNDS:         DB	'B'+80H	
                DB	0	
                DB	'C'+80H	
                DB	1	
                DB	'D'+80H	
                DB	2	
                DB	'E'+80H	
                DB	3	
                DB	'H'+80H	
                DB	4	
                DB	'L'+80H	
                DB	5	
                DB	"(H"	
                DB	'L'+80H	
                DB	6	
                DB	'A'+80H	
                DB	7	
                DB	"(I"	
                DB	'X'+80H	
                DB	86H	
                DB	"(I"	
                DB	'Y'+80H	
                DB	0C6H	
;
                DB	'B'	
                DB	'C'+80H	
                DB	8	
                DB	'D'	
                DB	'E'+80H	
                DB	10	
                DB	'H'	
                DB	'L'+80H	
                DB	12	
                DB	'I'	
                DB	'X'+80H	
                DB	8CH	
                DB	'I'	
                DB	'Y'+80H	
                DB	0CCH	
                DB	'A'	
                DB	'F'+80H	
                DB	14	
                DB	'S'	
                DB	'P'+80H	
                DB	14	
;
                DB	'N'	
                DB	'Z'+80H	
                DB	16	
                DB	'Z'+80H	
                DB	17	
                DB	'N'	
                DB	'C'+80H	
                DB	18	
                DB	'P'	
                DB	'O'+80H	
                DB	20	
                DB	'P'	
                DB	'E'+80H	
                DB	21	
                DB	'P'+80H	
                DB	22	
                DB	'M'+80H	
                DB	23	
;
                DB	'('	
                DB	'C'+80H	
                DB	20H	
;
                DB	0	
;
LDOPS:          DB	'I'	
                DB	0	
                DB	'A'+80H	
                DB	47H	
                DB	'R'	
                DB	0	
                DB	'A'+80H	
                DB	4FH	
                DB	'A'	
                DB	0	
                DB	'I'+80H	
                DB	57H	
                DB	'A'	
                DB	0	
                DB	'R'+80H	
                DB	5FH	
                DB	"(BC"	
                DB	0	
                DB	'A'+80H	
                DB	2	
                DB	"(DE"	
                DB	0	
                DB	'A'+80H	
                DB	12H	
                DB	'A'	
                DB	0	
                DB	"(B"	
                DB	'C'+80H	
                DB	0AH	
                DB	'A'	
                DB	0	
                DB	"(D"	
                DB	'E'+80H	
                DB	1AH	
;
                DB	0	
;
FIN:            ; END	

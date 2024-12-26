;
;Automatically created from original source on 2024-12-15 15:29:12
;
                .ASSUME ADL = 0	
;	.ORG 0x0000
;                SEGMENT CODE	
;
;PATCH FOR BBC BASIC TO CP/M 2.2 & 3.0
;* ACORN COMPUTERS Z80 TUBE VERSION  *
;(C) COPYRIGHT R.T.RUSSELL, 02-01-1984
;VERSION 5.0, 12-07-2024
;
;                XREF	OSWRCH	
;                XREF	OSWORD	
;                XREF	OSBYTE	
;
; ESC            EQU	1BH	
TBY:            EQU	0FH	
; TTO:            EQU	0B8H in main.asm
TFILL:          EQU	03H	
;
;                XREF	ITEMI	
;                XREF	EXPRI	
;                XREF	COMMA	
;                XREF	TERMQ	
;                XREF	BRAKET	
;                XREF	EXTERR	
;                XREF	STOREN	
;                XREF	TRAP	
;                XREF	VAR_	
;                XREF	NXT	
;                XREF	XEQ	
;
;                XREF	ACCS	
;                XREF	COUNT	
;                XREF	WIDTH	
;                XREF	SCRAP	
;
;                XDEF	OSCALL	
;                XDEF	CLRSCN	
;                XDEF	PUTCSR	
;                XDEF	GETCSR	
;                XDEF	PUTIME	
;                XDEF	GETIME	
;                XDEF	OSKEY	
;
;                XDEF	CLG	
;                XDEF	MOVE	
;                XDEF	DRAW	
;                XDEF	PLOT	
;                XDEF	MODE	
;                XDEF	COLOUR	
;                XDEF	GCOL	
;                XDEF	ADVAL	
;                XDEF	SOUND	
;                XDEF	ENVEL	
;                XDEF	POINT	
;
;                XDEF	CIRCLE	
;                XDEF	ELLIPS	
;                XDEF	FILL	
;                XDEF	MOUSE	
;                XDEF	ORIGIN	
;                XDEF	RECTAN	
;                XDEF	LINE	
;                XDEF	TINT	
;                XDEF	WAIT	
;                XDEF	SYS	
;                XDEF	CSRON	
;                XDEF	CSROFF	
;
;                XDEF	PUTIMS	
;                XDEF	GETIMS	
;                XDEF	TINTFN	
;                XDEF	MODEFN	
;                XDEF	WIDFN	
;
;GETIME	- Read elapsed-time clock.
;  	  Outputs:  DEHL = elapsed time (centiseconds)
; 	  Destroys: A,D,E,H,L,F
;
GETIME:         LD	A,1	
                LD	HL,SCRAP	
                CALL	OSWORD	
                LD	HL,SCRAP	
                LD	E,(HL)	
                INC	HL	
                LD	D,(HL)	
                INC	HL	
                LD	A,(HL)	
                INC	HL	
                LD	H,(HL)	
                LD	L,A	
                EX	DE,HL	
                RET	
;
;GETIMS	- Read real-time clock as string.
;  	  Outputs:  TIME$ in string accumulator
;                   E = string length (25)
; 	  Destroys: A,B,C,D,E,H,L,F
;
GETIMS:         LD	A,14	
                LD	HL,SCRAP	
                LD	(HL),0	
                CALL	OSWORD	
                LD	HL,SCRAP	
                LD	DE,ACCS	
                LD	A,(HL)	
                CP	E	
                RET	Z	
                LD	BC,25	
                LDIR	
                RET	
;
;
;PUTIME	- Load elapsed-time clock.
;   	  Inputs:   DEHL = time to load (centiseconds)
; 	  Destroys: A,D,E,H,L,F
;
PUTIME:         PUSH	IX	
                LD	IX,SCRAP	
                LD	(IX+0),L	
                LD	(IX+1),H	
                LD	(IX+2),E	
                LD	(IX+3),D	
                LD	A,2	
                LD	HL,SCRAP	
                CALL	OSWORD	
                POP	IX	
                RET	
;
;PUTIMS	- Wtite real-time clock as string.
;  	  Inputs:   string in string accumulator
;                   E = string length
; 	  Destroys: A,B,C,D,E,H,L,F
;
PUTIMS:         LD	A,E		;Length	
                CP	26	
                RET	NC	
                LD	B,0	
                LD	C,A	
                LD	DE,SCRAP+1	
                LD	HL,ACCS	
                LDIR	
                LD	HL,SCRAP	
                LD	(HL),A	
                LD	A,15	
                JP	OSWORD	
;
;
;CLRSCN	- Clear screen.
; 	  Destroys: A,D,E,H,L,F
;
CLRSCN:         LD	A,0CH	
                JP	OSWRCH	
;
;
;OSKEY	- Sample keyboard with specified wait.
;   	  Inputs:   HL = Time to wait (centiseconds)
;  	  Outputs:  Carry reset indicates time-out.
;                   If carry set, A = character typed.
; 	  Destroys: A,D,E,H,L,F
;
OSKEY:          LD	A,129	
                CALL	OSBYTE	
                LD	A,H	
                OR	A	
                RET	NZ		;TIME-OUT, CARRY RESET	
                LD	A,L	
                SCF	
                RET			;NORMAL, CARRY SET	
;
;PUTCSR	- Move cursor to specified position.
;   	  Inputs:   DE = horizontal position (LHS=0)
;                   HL = vertical position (TOP=0)
; 	  Destroys: A,D,E,H,L,F
;
PUTCSR:         LD	A,1FH	
                CALL	OSWRCH	
                LD	A,E	
                CALL	OSWRCH	
                LD	A,L	
                JP	OSWRCH	
;
;GETCSR	- Return cursor coordinates.
;   	  Outputs:  DE = X coordinate (POS)
;                   HL = Y coordinate (VPOS)
;  	  Destroys: A,D,E,H,L,F
;
GETCSR:         LD	A,134	
                CALL	OSBYTE	
                LD	E,L	
                LD	L,H	
                LD	D,0	
                LD	H,D	
                RET	
;
;POINT - var=POINT(x,y)
;
POINT:          CALL	EXPRI	
                EXX	
                PUSH	HL	
                CALL	CEXPRI	
                EXX	
                POP	DE	
                CALL	BRAKET	
                LD	IX,SCRAP	
                LD	(IX+0),E	
                LD	(IX+1),D	
                LD	(IX+2),L	
                LD	(IX+3),H	
                LD	HL,SCRAP	
                LD	A,9	
                CALL	OSWORD	
                LD	A,(IX+4)	
                LD	L,A	
                ADD	A,1	
                SBC	A,A	
                LD	H,A	
RETEXX:         EXX	
                LD	H,A	
                LD	L,A	
                XOR	A	
                LD	C,A	
                RET	
;
;ADVAL - var=ADVAL(n)
;
ADVAL:          CALL	ITEMI	
                EXX	
                LD	A,128	
                CALL	OSBYTE	
                XOR	A	
                JR	RETEXX	
;
;MODEFN - var=MODE
;
MODEFN:         LD	A,135	
                CALL	OSBYTE	
                LD	L,H	
RETU8:          XOR	A	
                LD	H,A	
                JR	RETEXX	
;
;WIDFN - var=WIDTH
;
WIDFN:          LD	A,(WIDTH)	
                LD	L,A	
                JR	RETU8	
;
;ENVEL - ENVELOPE var,var,var,var,var,var,var,
;                 var,var,var,var,var,var,var
;
ENVEL:          LD	B,0	
                LD	IX,SCRAP	
                PUSH	BC	
                PUSH	IX	
ENVEL1:         CALL	EXPRI	
                EXX	
                POP	IX	
                POP	BC	
                LD	(IX),L	
                LD	A,B	
                CP	13	
                JR	Z,ENVEL2	
                INC	B	
                INC	IX	
                PUSH	BC	
                PUSH	IX	
                CALL	COMMA	
                JR	ENVEL1	
ENVEL2:         LD	HL,SCRAP	
                LD	A,8	
                CALL	OSWORD	
                JP	XEQ	
;
;SOUND - SOUND var,var,var,var
;
SOUND:          LD	B,0	
                LD	IX,SCRAP	
                PUSH	BC	
                PUSH	IX	
SOUND1:         CALL	EXPRI	
                EXX	
                POP	IX	
                POP	BC	
                LD	(IX+0),L	
                LD	(IX+1),H	
                INC	IX	
                INC	IX	
                INC	B	
                INC	B	
                LD	A,B	
                CP	8	
                JR	Z,SOUND2	
                PUSH	BC	
                PUSH	IX	
                CALL	COMMA	
                JR	SOUND1	
SOUND2:         LD	HL,SCRAP	
                LD	A,7	
                CALL	OSWORD	
                JP	XEQ	
;
;MODE - MODE n
;
MODE:           CALL	EXPRI	
                XOR	A	
                LD	(COUNT),A	
                EXX	
                LD	H,L	
                LD	L,22	
                CALL	WRCH2	
                JR	XEQGO1	
;
;CLG
;
CLG:            LD	A,16	
                CALL	OSWRCH	
                JR	XEQGO1	
;
;ORIGIN x,y
;
ORIGIN:         CALL    EXPRI	
                EXX	
                PUSH	HL	
                CALL    CEXPRI	
                EXX	
                POP	DE	
                LD	C,29	
                CALL	WRCH5	
                JR	XEQGO1	
;
;COLOUR n
;COLOUR n,p
;COLOUR n,r,g,b
;
COLOUR:         CALL	EXPRI		;n	
                EXX	
                LD	A,(IY)	
                CP	','	
                JR      Z,PALCOL	
                LD	H,L	
                LD	L,17	
                CALL	WRCH2	
                JR	XEQGO1	
;
PALCOL:         PUSH	HL	
                CALL	CEXPRI		;p or r	
                EXX	
                EX	DE,HL	
                LD	HL,0	
                LD	A,(IY)	
                CP	','	
                JR	NZ,PALET1	
                PUSH	DE	
                CALL	CEXPRI		;g	
                EXX	
                PUSH	HL	
                CALL	CEXPRI		;b	
                EXX	
                POP	DE	
                POP	BC	
                LD	A,L	
                POP	HL	
                LD	D,C		;r	
                LD	C,L		;n	
                LD	L,E		;g	
                LD	H,A		;b	
                LD	E,16	
                PUSH	BC	
PALET1:         POP	BC	
                LD	B,19	
                CALL	WRCH6	
                JR	XEQGO1	
;
;GCOL [a,]b
;
GCOL:           CALL	EXPRI	
                EXX	
                LD	E,0	
                LD	A,(IY)	
                CP	','	
                JR	NZ,GCOL0	
                PUSH	HL	
                CALL	CEXPRI	
                EXX	
                POP	DE	
GCOL0:          LD	H,L	
                LD	L,E	
                LD	D,18	
                CALL	WRCH3		;DLH	
XEQGO1:         JP	XEQ	
;
;CSRON  - Turn caret on
;CSROFF - Turn caret off
;
CSRON:          LD	C,1	
                JR	CSRGO	
;
CSROFF:         LD	C,0	
CSRGO:          LD	A,23	
                CALL	OSWRCH	
                LD	A,1	
                CALL	OSWRCH	
                LD	A,C	
                LD	B,8	
CSRGO1:         CALL	OSWRCH	
                XOR	A	
                DJNZ	CSRGO1	
                JR	XEQGO1	
;
;LINE x1,y1,x2,y2
;
LINE:           CALL	EXPRI	
                EXX	
                PUSH	HL	
                CALL	EXPR3	
                EX	(SP),HL		;HL <- x1, (SP) <- y2	
                PUSH	BC	
                EX	DE,HL	
                LD	C,4	
                CALL	VDU25	
                POP	DE	
                POP	HL	
                LD	C,5	
                JR	PLOT4A	
;
;CIRCLE [FILL] x,y,r
;
CIRCLE:         CP	TFILL	
                PUSH	AF	
                JR	NZ,CIRCL0	
                INC	IY	
CIRCL0:         CALL	EXPRI	
                EXX	
                PUSH	HL	
                CALL	CEXPRI	
                EXX	
                PUSH	HL	
                CALL	CEXPRI	
                EXX	
                POP	BC		;y	
                POP	DE		;x	
                PUSH	HL	
                LD	L,C	
                LD	H,B	
                LD	C,4		; PLOT 4 = MOVE	
                CALL	VDU25	
                POP	DE		;r	
                LD	HL,0	
                POP	AF	
                LD	C,145		; PLOT 145 = outline circle	
                JR	NZ,PLOT4A	
                LD	C,153		; PLOT 153 = filled circle	
PLOT4A:         JR	PLOT4	
;
;ELLIPSE [FILL] x,y,a,b
;
ELLIPS:         CP	TFILL	
                PUSH	AF	
                JR	NZ,ELLIP0	
                INC	IY	
ELLIP0:         CALL	EXPRI	
                EXX	
                PUSH	HL	
                CALL	EXPR3	
                EX	(SP),HL		;HL <- x, (SP) <- b	
                PUSH	BC	
                EX	DE,HL	
                LD	C,4		; PLOT 4 = Move absolute	
                CALL	VDU25	
                POP	DE		;a	
                PUSH	DE	
                LD	HL,0	
                LD	C,L		; PLOT 0 - Move relative	
                CALL	VDU25	
                POP	DE		;a	
                XOR	A	
                LD	L,A	
                LD	H,A	
                SBC	HL,DE	
                EX	DE,HL	
                POP	HL		;b	
                POP	AF	
                LD	C,193		; PLOT 193 = outline ellipse	
                JR	NZ,PLOT4	
                LD	C,201		; PLOT 201 = filled ellipse	
                JR	PLOT4	
;
;MOVE [BY} x,y
;DRAW [BY] x,y
;PLOT [BY] [n,]x,y
;FILL [BY] x,y
;
MOVE:           LD	C,4	
                JR	PLOT1	
;
DRAW:           LD	C,5	
                JR	PLOT1	
;
FILL:           LD	C,133	
                JR	PLOT1	
;
PLOT:           LD	C,69	
                CP	TBY	
                JR	Z,PLOT1	
                CALL	EXPRI	
                EXX	
                PUSH	HL	
                CALL	CEXPRI	
                EXX	
                LD	A,(IY)	
                CP	','	
                JR	Z,PLOT3	
                POP	DE	
                LD	C,69	
                JR	PLOT4	
;
PLOT1:          CP	TBY	
                JR	NZ,PLOT2	
                INC	IY	
                RES	2,C		;Change absolute to relative	
PLOT2:          PUSH	BC	
                CALL	EXPRI	
                EXX	
PLOT3:          PUSH	HL	
                CALL	CEXPRI	
                EXX	
                POP	DE	
                POP	BC	
PLOT4:          CALL	VDU25	
                JP	XEQ	
;
;RECTANGLE [FILL] x,y,w[,h] [TO xnew,ynew]
;
RECTAN:         CP	TFILL	
                PUSH	AF	
                JR	NZ,RECT0	
                INC	IY	
RECT0:          CALL	EXPRI	
                EXX	
                PUSH	HL	
                CALL	CEXPRI	
                EXX	
                PUSH	HL	
                CALL	CEXPRI	
                EXX	
                PUSH	HL	
                LD	A,(IY)	
                CP	','	
                JR	NZ,RECT1	
                CALL	CEXPRI	
                EXX	
RECT1:          POP	BC		;w	
                POP	DE		;y	
                EX	(SP),HL		;HL <- x, (SP) <- h	
                PUSH	BC	
                EX	DE,HL	
                LD	C,4	
                CALL	VDU25	
                LD	A,(IY)	
                CP	TTO	
                JR	Z,RECTTO	
                POP	DE		;w	
                POP	HL		;h	
                POP	AF	
                JR	NZ,OUTLIN	
                LD	C,97	
                JR	PLOT4	
;
;Block copy / move:
;
RECTTO:         INC	IY		; Bump over TO	
                CALL	EXPRI	
                EXX	
                PUSH	HL	
                CALL	CEXPRI	
                EXX	
                POP	BC		;newx	
                POP	DE		;w	
                EX	(SP),HL		;HL <- h, (SP) <- newy	
                PUSH	BC	
                LD	C,0	
                CALL	VDU25	
                POP	DE		;newx	
                POP	HL		;newy	
                POP	AF	
                LD	C,190		; PLOT 190 - Block copy	
                JR	NZ,PLOT4B	
                DEC	C		; PLOT 189 - Block move	
PLOT4B:         JR	PLOT4	
;
;Outline rectangle:
;
OUTLIN:         LD	C,9		; PLOT 9 - draw relative	
                PUSH	HL	
                LD	HL,0	
                CALL	VDU25		; side 1	
                POP	HL	
                PUSH	DE	
                LD	DE,0	
                CALL	VDU25		; side 2	
                POP	DE	
                PUSH	HL	
                XOR	A	
                LD	L,A	
                LD	H,A	
                SBC	HL,DE	
                EX	DE,HL	
                LD	L,A	
                LD	H,A	
                CALL 	VDU25		; side 3	
                POP	DE	
                XOR	A	
                LD	L,A	
                LD	H,A	
                SBC	HL,DE	
                LD	E,A	
                LD	D,A	
                JR	PLOT4B	
;
;MOUSE x, y, b
;
MOUSE:          LD	A,128	
                LD	HL,9	
                CALL	OSBYTE	
                PUSH	HL	
                LD	A,128	
                LD	HL,8	
                CALL	OSBYTE	
                PUSH	HL	
                LD	A,128	
                LD	HL,7	
                CALL	OSBYTE	
                PUSH	HL	
                CALL	VAR_	
                POP	HL	
                CALL	STOREI	
                CALL	COMMA	
                CALL	NXT	
                CALL	VAR_	
                POP	HL	
                CALL	STOREI	
                CALL	COMMA	
                CALL	NXT	
                CALL	VAR_	
                POP	HL	
                CALL	STOREI	
XEQGO2:         JP	XEQ	
;
;WAIT [n]
;
WAIT:           CALL	TERMQ	
                JR	Z,XEQGO2	
                CALL	EXPRI	
                EXX	
                LD	B,H	
                LD	C,L	
                CALL	GETIME	
                ADD	HL,BC	
                LD	BC,0	
                EX	DE,HL	
                ADC	HL,BC	
                EX	DE,HL	
WAIT1:          CALL	TRAP	
                PUSH	DE	
                PUSH	HL	
                CALL	GETIME	
                POP	BC	
                OR	A	
                SBC	HL,BC	
                LD	H,B	
                LD	L,C	
                EX	DE,HL	
                POP	BC	
                SBC	HL,BC	
                JR	NC,XEQGO2	
                EX	DE,HL	
                LD	D,B	
                LD	E,C	
                JR	WAIT1	
;
;OSCALL - Trap call to FFxx
;
OSCALL:         POP	HL		;DITCH RETURN ADDRESS	
                LD	HL,OSRET	
                PUSH	HL		;NEW RETURN ADDRESS	
                LD	A,(IX+4)	;A%	
                LD	E,(IX+20)	;E%	
                LD	H,(IX+100)	;Y%	
                LD	L,(IX+96)	;X%	
                JP	(IY)	
OSRET:          PUSH	AF	
                LD	A,L		;F  H  L  A	
                LD	L,H		;|  |  |  |	
                EXX			;|  |  |  |	
                POP	BC		;|  |  |  |	
                LD	H,A		;|  |  |  |	
                LD	L,B		;H  L  H' L'	
                LD	A,C	
                EXX	
                LD	H,A	
                RET	
;
VDU25:          LD	B,25	
WRCH6:          LD	A,B	
                CALL	OSWRCH	
WRCH5:          LD	A,C	
                CALL	OSWRCH	
WRCH4:          LD	A,E	
                CALL	OSWRCH	
WRCH3:          LD	A,D	
                CALL	OSWRCH	
WRCH2:          LD	A,L	
                CALL	OSWRCH	
                LD	A,H	
                JP	OSWRCH	
;
EXPR3:          CALL	CEXPRI	
                EXX	
                PUSH	HL	
                CALL	CEXPRI	
                EXX	
                PUSH	HL	
                CALL	CEXPRI	
                EXX	
                POP	BC		;x2	
                POP	DE		;y1	
                RET	
;
CEXPRI:         CALL	COMMA	
                JP	EXPRI	
;
STOREI:         BIT	7,A	
                JR	NZ,EEK	
                BIT	6,A	
                JR	NZ,EEK	
                EXX	
                LD	HL,0	
                LD	C,L	
                JP	STOREN	
;
EEK:            LD	A,50	
                CALL	EXTERR	
                DB	13H		;'Bad '	
                DB	04H		;'MOUSE'	
                DB	20H	
                DB	15H		;'variable'	
                DB	0	
;
TINT:           	
TINTFN:         	
SYS:            	
                XOR	A	
                CALL	EXTERR	
                DB	"Sorry"	
                DB	0	
;
;                END	

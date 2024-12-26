;
;Automatically created from original source on 2024-12-15 15:29:12
;
                .ASSUME ADL = 0	
;	.ORG 0x0000
            ;    DEFINE LORAM, SPACE = ROM	
            ;    SEGMENT LORAM	
;
;                XDEF	FLAGS	
;                XDEF	OSWRCHPT	
;                XDEF	OSWRCHCH	
;                XDEF	OSWRCHFH	
;                XDEF	KEYDOWN	
;                XDEF	KEYASCII	
;                XDEF	KEYCOUNT	
;                XDEF	SCRAP	
;                XDEF	BUFFER	
;                XDEF	LISTON	
;                XDEF	PAGE_	
;
end_binary: ;  for 05_assemble.py to know where to truncate the binary file
                ALIGN 256	
FLAGS:          DS	1	
OSWRCHPT:       DS	2	
OSWRCHCH:       DS	1	
OSWRCHFH:       DS	1	
KEYDOWN:        DS	1	
KEYASCII:       DS	1	
KEYCOUNT:       DS	1	
SCRAP:          DS	31	
;
                ALIGN 256	
;
;RAM MODULE FOR BBC BASIC INTERPRETER
;FOR USE WITH VERSION 5.0 OF BBC BASIC
;(C) COPYRIGHT R.T.RUSSELL 1981-2024
;
;                XDEF	ACCS	
;                XDEF	BUFFER	
;                XDEF	ONERSP	
;                XDEF	LIBASE	
;                XDEF	PAGE_	
;                XDEF	LOMEM	
;                XDEF	FREE	
;                XDEF	HIMEM	
;                XDEF	RANDOM	
;                XDEF	COUNT	
;                XDEF	WIDTH	
;                XDEF	ERL	
;                XDEF	ERR	
;                XDEF	ERRTRP	
;                XDEF	ERRTXT	
;                XDEF	TRACEN	
;                XDEF	AUTONO	
;                XDEF	INCREM	
;                XDEF	LISTON	
;                XDEF	DATPTR	
;                XDEF	FNPTR	
;                XDEF	PROPTR	
;                XDEF	STAVAR	
;                XDEF	OC	
;                XDEF	PC	
;                XDEF	DYNVAR	
;                XDEF	CURLIN	
;                XDEF	USER	
;
;n.b. ACCS, BUFFER & STAVAR must be on page boundaries.
;
ACCS:           DS	256		;STRING ACCUMULATOR	
BUFFER:         DS	256		;STRING INPUT BUFFER	

STAVAR:         DS	27*4		;STATIC VARIABLES	
; OC:             EQU	STAVAR+15*4	;CODE ORIGIN (O%)	; restored from equs.inc
; PC:             EQU	STAVAR+16*4	;PROGRAM COUNTER (P%)	; restored from equs.inc
OC:             EQU	15*4+STAVAR	;CODE ORIGIN (O%)	; restored from equs.inc
PC:             EQU	16*4+STAVAR	;PROGRAM COUNTER (P%)	; restored from equs.inc
DYNVAR:         DS	54*2		;DYN. VARIABLE POINTERS	
FNPTR:          DS	2		;DYN. FUNCTION POINTER	
PROPTR:         DS	2		;DYN. PROCEDURE POINTER	
;
PAGE_:          DS	2		;START OF USER PROGRAM	
LOMEM:          DS	2		;START OF DYN. STORAGE	
FREE:           DS	2		;FIRST FREE-SPACE BYTE	
HIMEM:          DS	2		;FIRST BYTE ABOVE STACK	
LIBASE:         DS	2		;START OF FIRST LIBRARY	
;
TRACEN:         DS	2		;TRACE FLAG & NUMBER	
AUTONO:         DS	2		;AUTO FLAG & NUMBER	
ERRTRP:         DS	2		;ON ERROR STMT POINTER \	
ONERSP:         DS	2		;ON ERROR LOCAL STKPTR /	
ERRTXT:         DS	2		;ERROR MESSAGE POINTER	
DATPTR:         DS	2		;DATA POINTER	
ERL:            DS	2		;LINE NO OF LAST ERROR	
CURLIN:         DS	2		;POINTER TO CURRENT LINE	
RANDOM:         DS	5		;RANDOM NUMBER	
COUNT:          DS	1		;PRINT POSITION	
WIDTH:          DS	1		;PRINT WIDTH	
ERR:            DS	1		;ERROR NUMBER	
LISTON:         DS	1		;LISTO & OPT FLAG	
INCREM:         DS	1		;AUTO INCREMENT	
VDU_BUFFER:		EQU	ACCS		; Storage for VDU commands ; originally in equs.inc
;
USER:           ; END	

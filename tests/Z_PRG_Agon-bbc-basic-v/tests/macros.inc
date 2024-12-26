;
; Title:	BBC Basic Interpreter - Z80 version
;		Useful macros
; Author:	Dean Belfield
; Created:	04/12/2024
; Last Updated:	04/12/2024
;
; Modinfo:

        MACRO EXREG rp1, rp2
        PUSH    rp1 
        POP     rp2
        ENDMACRO 

        MACRO ADD8U_DE ; reg
        ADD     A, E 
        LD      E, A 
        ADC     A, D
        SUB     E
        LD      D, A 
        ENDMACRO 

        MACRO ADD8U_HL ; reg
        ADD     A, L 
        LD      L, A 
        ADC     A, H
        SUB     L
        LD      H, A 
        ENDMACRO 

        MACRO VDU VAL
        LD      A, VAL
        CALL    OSWRCH
        ENDMACRO

        MACRO SET_GPIO REG, VAL
        IN0     A, (REG)
        OR      VAL
        OUT0    (REG), A
        ENDMACRO
                
        MACRO RES_GPIO REG, VAL
        PUSH    BC
        LD      A, VAL
        CPL
        LD      C, A
        IN0     A, (REG)
        AND     C
        OUT0    (REG), A
        POP     BC
        ENDMACRO

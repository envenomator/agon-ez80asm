; line with label, opcode, tokens
; objective: correct parsing of different number systems
;
; Basic 1-byte number tests
.org $40000
; Hexadecimal, only spasm format
    ld a, 0Ah
    ld a, 0Ah
    ld a, 00Ah ; test overflow test in operand parsing
    ld a, 0Ah
    ld a, 0Ah
    ld a, 00Ah
    ld a, $A
    ld a, $0A
    ld a, $00A
    ld a, 0Ah
    ld a, 0Ah
    ld a, 00Ah
; Binary
    ld a, 0b
    ld a, 1b
    ld a, 01b
    ld a, 10b
    ld a, 11111111b
    ld a, 00000000b
    ld a, 000000001b ; test overflow test in operand parsing
    ld a, %0
    ld a, %1
    ld a, %01
    ld a, %10
    ld a, %11111111
    ld a, %00000000
    ld a, %000000001 ; test overflow test in operand parsing
    ld a, 0b
    ld a, 1b
    ld a, 01b
    ld a, 10b
    ld a, 11111111b
    ld a, 00000000b
    ld a, 000000001b ; test overflow test in operand parsing
; Decimal
    ld a, 0
    ld a, 128
    ld a, 255

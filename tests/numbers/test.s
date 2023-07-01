; line with label, opcode, tokens
; objective: correct parsing of different number systems
;
; Basic 1-byte number tests
.org $40000
; Hexadecimal
    ld a, 0xA
    ld a, 0x0A
    ld a, 0x00A ; test overflow test in operand parsing
    ld a, #A
    ld a, #0A
    ld a, #00A
    ld a, $A
    ld a, $0A
    ld a, $00A
    ld a, Ah
    ld a, 0Ah
    ld a, 00Ah
    ld a, 0bh  ; weird case, needs priority before binary
    ld a, 0b0h ; idem
    ld a, 0b1h ; idem
; Binary
    ld a, 0b0
    ld a, 0b1
    ld a, 0b01
    ld a, 0b10
    ld a, 0b11111111
    ld a, 0b00000000
    ld a, 0b000000001 ; test overflow test in operand parsing
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
    ld a, 1
    ld a, 2
    ld a, 3
    ld a, 4
    ld a, 5
    ld a, 6
    ld a, 7
    ld a, 8
    ld a, 9
    ld a, 128
    ld a, 255

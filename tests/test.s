.assume ADL=1
@@:
    adc.s hl, sp
    jp.sis @b
    ld a, (@f)
labela: asciiz "Test 1 2 3"
@@: dw 0xaabbcc
    ld de, (@b)
    ld sp, (@local1)
@local1:
    jp @local1
labelb:
@local1:
    ds 2, 0ah
    ds 3, 1010b
    jp @local1
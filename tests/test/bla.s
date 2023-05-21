    .include "bla.inc"
    ld a,b
    .align 16
label:ld a,b
    .fillbyte 0x02;bla
    .align 16
    ld a,b
    .fillbyte 0x1
    .align 16
    ld a,b

    def 5

    macro sum arg1,arg2
@1:    ld a,b
    jp @1
    endmacro

    org $41000
@1:
    jp @1

    sum 10,15
@1:
    jp @1

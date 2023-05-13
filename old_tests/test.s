hex:
    ld a,b
@@:
hex:
    macro sum arg1,arg2
@@: 
    ld a, arg1
    add a, arg2
    jp nz, @p
    endmacro

    jp @p
@@:
    jp @p
    ld a,b
@@:
    jp @p
@@:
    jp @p

    sum 10,15



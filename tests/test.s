    ld a,b
@@:
    macro sum arg1,arg2
@@: 
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



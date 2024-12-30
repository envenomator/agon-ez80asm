; Testing translation of macro arguments to relocate/fillbyte/align
    macro test address,fill,boundary
    fillbyte fill
    align boundary
    relocate address
    ld a,0
    jp $
    endrelocate
    endmacro

    jp $
    test $50000,01,$100
    jp $

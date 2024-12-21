; Testing issue with argumentless macro command, followed by immediate comment without spaces
; Will error up until v1.11
    macro test
    ld a, 0
    endmacro

    test;gone

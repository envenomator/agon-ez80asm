; Test conditional macros
;

    .if 1

    macro test
    .db 0
    endmacro

    .else

    macro test
    .db 1
    endmacro

    .endif

    test

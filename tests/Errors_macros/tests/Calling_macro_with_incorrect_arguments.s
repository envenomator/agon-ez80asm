    .macro test; macro without arguments
    ld a,b
    .endmacro

    test 15 ; this should error
    .macro test one; macro with one argument
    ld a, one
    .endmacro

    test 15, 20 ; this should error
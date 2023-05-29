    .macro test one; macro with one argument
    ld a, one
    .endmacro

    test ; this should error
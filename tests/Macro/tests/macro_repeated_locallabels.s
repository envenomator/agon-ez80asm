    .macro test
    ld a, 10
@local:
    dec a
    jp z, @local
    .endmacro

    test
    test
    test
    test
    test


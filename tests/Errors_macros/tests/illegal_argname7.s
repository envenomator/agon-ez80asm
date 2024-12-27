; Testing illegal argument name (number)

    macro test 1b
    ld a, 1b
    endmacro

    test 0

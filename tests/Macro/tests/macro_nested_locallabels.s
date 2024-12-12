; senseless program to test nested @local labels in macros
    .macro proc
    .db 0 ; START PROC
@local:
    dec a
    jp z, @local
    .db 0xff ; END PROC
    .endmacro

    .macro test
    ld a, 0 ; START TEST
@local:
    dec a
    proc
    jp z, @local
    ld hl, @local
    proc
    ld hl, @local ; END TEST
    .endmacro

@local:
    test
    jp @local

start:
    ld hl, sample
    ld a, (hl)
sample:
    incbin "sample.bin"
    incbin "sample.bin"
    ld a,b

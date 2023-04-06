MAX_ARGS:   equ 2

_main:
    ld hl, test_bitmap
    ld bc, end-test_bitmap
    rst.lil $18
    ret

test_bitmap:
    db 23,27,0,0
    incbin "sample.bin"
end:

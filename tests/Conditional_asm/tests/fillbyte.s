; Test conditional fillbyte

    .if 1
    fillbyte 0x00
    .else
    fillbyte 0x01
    .endif

    .db 0xff
    .blkb 16; use fillbyte, should be 16x 0x00 if properly implemented
    .db 0xff

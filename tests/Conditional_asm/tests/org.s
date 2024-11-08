; Test conditional org

    .org 0x40000
    .db 0xff

    .if 1
    .org 0x40010
    .else
    .org 0x40020
    .endif

    .db 0xff

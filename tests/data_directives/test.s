; we'll use the directive without ., or the reference ZDS assembler barks at us
; byte tests
        db 5
        db 1,2,3
        db 10;comment
        byte 5
        byte 1,2,3
        byte 10;comment
        ascii 5
        ascii 1,2,3
        ascii 10;comment
        defb 5
        defb 1,2,3
        defb 10;comment
label:
; 16-bit word tests
        dw 5
        dw 1,2,3
        dw 10;comment
        dw ffffh
        dw label
; 24-bit word tests
        dl 5
        dl 1,2,3
        dl 10;comment
        dl ffffh
        dl label
        dw24 5
        dw24 1,2,3
        dw24 10;comment
        dw24 ffffh
        dw24 label
; define space
        ds 10
        ds 10;uninitialized. Will be different between assemblers
        ds 10,0xFF
; define blocks
        blkb 16, 0xFF
        blkb 10h, 0xFF
        blkb 16;uninitialized. Will be different between assemblers
        blkw 16, 0xAAFF
        blkw 10h, 0xAAFF
        blkw 16;uninitialized. Will be different between assemblers
        blkp 16, 0xAABBFF
        blkp 10h, 0xAABBFF
        blkp 10h, label
        blkp 16;uninitialized. Will be different between assemblers
        blkl 16, 0xAABBCCFF
        blkl 10h, 0xAABBCCFF
        blkl 16;uninitialized. Will be different between assemblers

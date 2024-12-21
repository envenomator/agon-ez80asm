; Test avoiding spaces at end of file using mixed DS/ALIGN statements
        ld hl, endval ; should correctly load address after all DS/ALIGN statements
        ld hl, 0x40700; should be exactly the same as previous
start:  ds 256        ; should produce fillbytes
        ld hl, label
        ld hl, 0x40300; should be exactly the same as previous
        align 512   ; should produce fillbytes
        ds 256      ; should produce fillbytes
label:  ld a,b      ; last output byte
        align 256   ; shouldn't produce output
        ds 256      ; shouldn't produce output
        align 256   ; shouldn't produce output
        ds 512      ; shoudln't produce output
endval:

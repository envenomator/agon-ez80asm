; Testing relocation addresses using the .relocate directive
.org $40000
start:
    jp $50000

    .align $100     ; to facilitate manual byte counting in preparation of this test
.relocate $50000
    ld a,0          ; 50000 / 50001
    ld a,0          ; 50002 / 50003
rellabel:           ; 50004
    ld a,0          ; 50004 / 50005
@local:             ; 50006
    dw24 $          ; 50006 / 50007 / 50008 - content should be 50006 - the relocated current address
    jp rellabel     ; 50009 / 5000A / 5000B / 5000C - should jump to 50004
    jp @local       ; 5000D / 5000E / 5000F / 50010 - should jump to 50006
    jp start        ; 50011 / 50012 / 50013 / 50014 - should jump to 40000
    jp outside      ; 50015 / 50016 / 50017 / 50018 - should jump to 40124

    val: EQU 0      ; EQU should be zero
    dw24 val        ; 50019 / 5001A / 5001B value should be 0
    ina: equ $      ; 5001C - nastigram - should be relocated address 5001C
    jp ina          ; 5001C / 5001D / 5001E / 5001F - should jump to 5001C
    jp $            ; 50020 / 50021 / 50022 / 50023 - should jump to 50020
.endrelocate

outside:            ; real address 40124, would have been 50024 if endrelocate wasn't given
    jp outside      ; 40124 / 40125 / 40126 / 40127
    oad: equ $
    dw24 oad        ; 40128 / 40129 / 4012A value should be 40128
    jp $            ; 4012B / 4012C / 4012D / 4012E - should jump to 4012B

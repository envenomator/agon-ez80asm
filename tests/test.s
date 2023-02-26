.assume adl=1
.org $0010
label: .equ label2
@1:
    ld a, @1
label2:
    ld a, label
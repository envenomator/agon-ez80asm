.assume ADL=1
    ld a,b
    ld a,b
test1:
test2:
@local:
test3: .equ test1+5
    ld a, test3+2

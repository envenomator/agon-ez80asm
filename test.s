nop
labela:
nop
ld.s a,(ix+5)
jp nz,labelb
ld a,b
db 1,2,3,1010b,5
labelb:
    nop
jp.sis labela
adl 1
 nop
 
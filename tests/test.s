	ld a, 0xff
	ld a, $ff
	ld a, fah
labela:
	ld a, b
.assume adl=0
labelb: dw 0x15, labela, $aabbcc
.assume adl=1
labelc: dw labela
	.assume adl=1
	jp nz, $aabbcc
	jp z, $aabbcc
	jp nc, $aabbcc
	jp c, $aabbcc
	jp po, $aabbcc
	jp pe, $aabbcc
	jp p, $aabbcc
	jp m, $aabbcc
labela:
	jp (hl)
	jp (ix)
	jp (iy)
	jp $aabbcc
	jr nz, labela
	jr z, labela
	jr nc,labela
	jr c, labela
	jr labela

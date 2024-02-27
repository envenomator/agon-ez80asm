	.assume adl=1
	jp nz, aabbcch
	jp z, aabbcch
	jp nc, aabbcch
	jp c, aabbcch
	jp po, aabbcch
	jp pe, aabbcch
	jp p, aabbcch
	jp m, aabbcch
labela:
	jp (hl)
	jp (ix)
	jp (iy)
	jp aabbcch
	jr nz, labela
	jr z, labela
	jr nc,labela
	jr c, labela
	jr labela

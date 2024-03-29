	.assume adl=1
	mlt bc
	mlt de
	mlt hl
	mlt sp
	neg
	nop
	or a, (hl)
	or a, ixh
	or a, ixl
	or a, iyh
	or a, iyl
	or a, (ix+5)
	or a, (iy+5)
	or a, 5
	or a, a
	or a, b
	or a, c
	or a, d
	or a, e
	or a, h
	or a, l
	otd2r
	otdm
	otdmr
	otdr
	otdrx
	oti2r
	otim
	otimr
	otir
	otirx
	out (bc), a
	out (bc), b
	out (bc), c
	out (bc), d
	out (bc), e
	out (bc), h
	out (bc), l
	out (c), a
	out (c), b
	out (c), c
	out (c), d
	out (c), e
	out (c), h
	out (c), l
	out (5), a
	out0 (5), a
	out0 (5), b
	out0 (5), c
	out0 (5), d
	out0 (5), e
	out0 (5), h
	out0 (5), l
	outd
	outd2
	outi
	outi2
	pea ix+5
	pea iy+5
	pop af
	pop ix
	pop iy
	pop bc
	pop de
	pop hl
	push af
	push ix
	push iy
	push bc
	push de
	push hl
	res 0, (hl)
	res 1, (hl)
	res 2, (hl)
	res 3, (hl)
	res 4, (hl)
	res 5, (hl)
	res 6, (hl)
	res 7, (hl)
	res 0, (ix+5)
	res 1, (ix+5)
	res 2, (ix+5)
	res 3, (ix+5)
	res 4, (ix+5)
	res 5, (ix+5)
	res 6, (ix+5)
	res 7, (ix+5)
	res 0, (iy+5)
	res 1, (iy+5)
	res 2, (iy+5)
	res 3, (iy+5)
	res 4, (iy+5)
	res 5, (iy+5)
	res 6, (iy+5)
	res 7, (iy+5)
	res 0, a
	res 1, a
	res 2, a
	res 3, a
	res 4, a
	res 5, a
	res 6, a
	res 7, a
	res 0, b
	res 1, b
	res 2, b
	res 3, b
	res 4, b
	res 5, b
	res 6, b
	res 7, b
	res 0, c
	res 1, c
	res 2, c
	res 3, c
	res 4, c
	res 5, c
	res 6, c
	res 7, c
	res 0, d
	res 1, d
	res 2, d
	res 3, d
	res 4, d
	res 5, d
	res 6, d
	res 7, d
	res 0, e
	res 1, e
	res 2, e
	res 3, e
	res 4, e
	res 5, e
	res 6, e
	res 7, e
	res 0, h
	res 1, h
	res 2, h
	res 3, h
	res 4, h
	res 5, h
	res 6, h
	res 7, h
	res 0, l
	res 1, l
	res 2, l
	res 3, l
	res 4, l
	res 5, l
	res 6, l
	res 7, l
	ret
	ret nz
	ret z
	ret nc
	ret c
	ret po
	ret pe
	ret p
	ret m
	reti
	retn
	rl (hl)
	rl (ix+5)
	rl (iy+5)
	rl a
	rl b
	rl c
	rl d
	rl e
	rl h
	rl l
	rla
	rlc (hl)
	rlc (ix+5)
	rlc (iy+5)
	rlc a
	rlc b
	rlc c
	rlc d
	rlc e
	rlc h
	rlc l
	rlca
	rld
	rr (hl)
	rr (ix+5)
	rr (iy+5)
	rr a
	rr b
	rr c
	rr d
	rr e
	rr h
	rr l
	rra
	rrc (hl)
	rrc (ix+5)
	rrc (iy+5)
	rrc a
	rrc b
	rrc c
	rrc d
	rrc e
	rrc h
	rrc l
	rrca
	rrd
	rsmix
	rst 0
	rst 08h
	rst 10h
	rst 18h
	rst 20h
	rst 28h
	rst 30h
	rst 38h
	sbc a, (hl)
	sbc a, ixh
	sbc a, ixl
	sbc a, iyh
	sbc a, iyl
	sbc a, (ix+5)
	sbc a, (iy+5)
	sbc a, 5
	sbc a, a
	sbc a, b
	sbc a, c
	sbc a, d
	sbc a, e
	sbc a, h
	sbc a, l
	sbc hl, bc
	sbc hl, de
	sbc hl, hl
	sbc hl, sp
	scf
	set 0, (hl)
	set 1, (hl)
	set 2, (hl)
	set 3, (hl)
	set 4, (hl)
	set 5, (hl)
	set 6, (hl)
	set 7, (hl)
	set 0, (ix+5)
	set 1, (ix+5)
	set 2, (ix+5)
	set 3, (ix+5)
	set 4, (ix+5)
	set 5, (ix+5)
	set 6, (ix+5)
	set 7, (ix+5)
	set 0, (iy+5)
	set 1, (iy+5)
	set 2, (iy+5)
	set 3, (iy+5)
	set 4, (iy+5)
	set 5, (iy+5)
	set 6, (iy+5)
	set 7, (iy+5)
	set 0, a
	set 0, b
	set 0, c
	set 0, d
	set 0, e
	set 0, h
	set 0, l
	set 1, a
	set 1, b
	set 1, c
	set 1, d
	set 1, e
	set 1, h
	set 1, l
	set 2, a
	set 2, b
	set 2, c
	set 2, d
	set 2, e
	set 2, h
	set 2, l
	set 3, a
	set 3, b
	set 3, c
	set 3, d
	set 3, e
	set 3, h
	set 3, l
	set 4, a
	set 4, b
	set 4, c
	set 4, d
	set 4, e
	set 4, h
	set 4, l
	set 5, a
	set 5, b
	set 5, c
	set 5, d
	set 5, e
	set 5, h
	set 5, l
	set 6, a
	set 6, b
	set 6, c
	set 6, d
	set 6, e
	set 6, h
	set 6, l
	set 7, a
	set 7, b
	set 7, c
	set 7, d
	set 7, e
	set 7, h
	set 7, l
	sla (hl)
	sla (ix+5)
	sla (iy+5)
	sla a
	sla b
	sla c
	sla d
	sla e
	sla h
	sla l
	slp
	sra (hl)
	sra (ix+5)
	sra (iy+5)
	sra a
	sra b
	sra c
	sra d
	sra e
	sra h
	sra l
	srl (hl)
	srl (ix+5)
	srl (iy+5)
	srl a
	srl b
	srl c
	srl d
	srl e
	srl h
	srl l
	stmix
	sub a, (hl)
	sub a, ixh
	sub a, ixl
	sub a, iyh
	sub a, iyl
	sub a, (ix+5)
	sub a, (iy+5)
	sub a, 5
	sub a, a
	sub a, b
	sub a, c
	sub a, d
	sub a, e
	sub a, h
	sub a, l
	tst a, (hl)
	tst a, 5
	tst a, a
	tst a, b
	tst a, c
	tst a, d
	tst a, e
	tst a, h
	tst a, l
	tstio 5
	xor a, (hl)
	xor a, ixh
	xor a, ixl
	xor a, iyh
	xor a, iyl
	xor a, (ix+5)
	xor a, (iy+5)
	xor a, 5
	xor a, a
	xor a, b
	xor a, c
	xor a, d
	xor a, e
	xor a, h
	xor a, l

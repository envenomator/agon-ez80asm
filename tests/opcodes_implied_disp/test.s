	.assume adl=1
	add a, (ix)
	add a, (iy)
	add (ix)
	add (iy)
	and a, (ix)
	and a, (iy)
	and (ix)
	and (iy)
	bit 0, (ix)
	bit 0, (iy)
	cp a, (ix)
	cp a, (iy)
	cp (ix)
	cp (iy)
	dec (ix)
	dec (iy)
	inc (ix)
	inc (iy)
	ld (ix), hl
	ld (iy), hl
	ld a, (ix)
	ld a, (iy)
	ld (ix), 1
	ld (iy), 1
	ld (ix), b
	ld (iy), b
	ld b, (ix)
	ld b, (iy)
	ld hl, (ix)
	ld hl, (iy)
	or a, (ix)
	or a, (iy)
	or (ix)
	or (iy)
	res 0, (ix)
	res 0, (iy)
	rl (ix)
	rl (iy)
	rlc (ix)
	rlc (iy)
	rr (ix)
	rr (iy)
	rrc (ix)
	rrc (iy)
	sbc a, (ix)
	sbc a, (iy)
	sbc (ix)
	sbc (iy)
	set 0, (ix)
	set 0, (iy)
	sla (ix)
	sla (iy)
	sra (ix)
	sra (iy)
	srl (ix)
	srl (iy)
	sub a, (ix)
	sub a, (iy)
	sub (ix)
	sub (iy)
	xor a, (ix)
	xor a, (iy)
	xor (ix)
	xor (iy)
	lea hl, ix
	lea hl, iy
	lea ix, ix
	lea ix, iy
	lea iy, ix
	lea iy, iy
	pea ix
	pea iy
	ld ix, (ix)
	ld iy, (iy)
	ld iy, (ix)
	ld ix, (iy)
	ld (ix), ix
	ld (iy), iy
	ld (ix), iy
	ld (iy), ix

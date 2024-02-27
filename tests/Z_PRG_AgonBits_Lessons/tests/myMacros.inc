; --------------------------------
;
;	 SOME USEFUL MACROS
;
; --------------------------------

	macro CLS
	ld a, 12
	rst.lil $10
	endmacro

	macro SET_COLOUR value
	ld a, 17					; set text colour
	rst.lil $10
	ld a, value					; colour to use
	rst.lil $10
	endmacro

	macro SET_BG_COLOUR value
	ld a, 17					; set text colour
	rst.lil $10
	ld a, value					; colour to use
	add a, 128
	rst.lil $10
	endmacro

	macro TAB_TO x,y
	ld a, 31					; move to...
	rst.lil $10
	ld a, x						; X position
	rst.lil $10
	ld a, y						; Y position
	rst.lil $10
	endmacro

	macro SET_MODE mode
	ld a, 22					; set mode...
	rst.lil $10
	ld a, mode					; to mode
	rst.lil $10
	endmacro

	macro MOSCALL arg1
	ld a, arg1
	rst.lil $08
	endmacro














































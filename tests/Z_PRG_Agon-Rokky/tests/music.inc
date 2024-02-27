TRACK_RESET:    EQU 0
TRACK_SET_FREQ: EQU 1
TRACK_SET_VOL:  EQU 2
TRACK_SET_WAVE: EQU 3
TRACK_DELAY:    EQU 4


track_free:
    di
    ld hl, (old_int_handler + 1)
    ld e, $32
    MOSCALL mos_setint
    ld hl, reset_sound
    ld bc, end_reset_sound-reset_sound
    rst.lil $18
    ei
    ret

track_init:
    di
    ld hl, track_tick
    ld e, $32
    ld hl, vsync_int
    MOSCALL mos_setint
    ld (old_int_handler + 1), hl
    call track_reinit
    ei
    ret

;; Initializes track and sets vblank interrupt
track_reinit:
    xor a
    ld (delay_frames), a

    ld hl, track
    ld (current_position), hl

    ld hl, reset_sound
    ld bc, end_reset_sound-reset_sound
    rst.lil $18
    ret
reset_sound:
    db $17, $00, $85, $00, $09
    db $17, $00, $85, $01, $09
    db $17, $00, $85, $02, $09
    db $17, $00, $85, $03, $09

    db $17, $00, $85, $00, $08
    db $17, $00, $85, $01, $08
    db $17, $00, $85, $02, $08
    db $17, $00, $85, $03, $08

    db $17, $00, $85, $00, $02, $00
    db $17, $00, $85, $01, $02, $00
    db $17, $00, $85, $02, $02, $00
    db $17, $00, $85, $03, $02, $00
end_reset_sound:

vsync_int:
    di
    push af
    push bc
    push de
    push hl
    push ix
    push iy

    ;; GPIO related stuff :-)
    in0 a,($9a)
    or 2
    out0 ($9a), a

    call track_tick
    pop iy
    pop ix
    pop hl
    pop de
    pop bc
    pop af
old_int_handler:
    jp 0
    reti.l

;; Happens every interrupt :-) 
track_tick:
    ld a, (delay_frames)
    and a
    jr z, @play
    
    dec a
    ld (delay_frames), a
    ret
@play:
    ;; Reading command channel
    ld hl, (current_position)
    ld a, (hl)
    ld (track_channel), a
    inc a
    ;; Reading command
    inc hl
    ld a, (hl)
    ;; Pointing to next byte
    inc hl
    ld (current_position), hl

    ;; If RESET command - reinit our track 
    and a 
    jp z, track_reinit

    cp TRACK_SET_FREQ
    jp z, track_freq

    cp TRACK_SET_VOL
    jp z, track_vol

    cp TRACK_SET_WAVE
    jp z, track_wave

    CP TRACK_DELAY
    jp z, track_delay

    ret

;; Sets delay for some frame count
track_delay:
    ld a, (hl)
    ld (delay_frames), a
    inc hl
    ld (current_position), hl
    ret

;; Changes frequency that will be played
track_freq:
    ld hl, (current_position)
    ld hl, (hl)
    ld a, l
    ld (@note), a
    ld a, h
    ld (@note+1), a

    ld hl, (current_position)
    inc hl
    inc hl
    ld (current_position), hl
    
    call track_command_header
    ld hl, @cmd
    ld bc, @end-@cmd
    rst.lil $18
    jp track_tick
@cmd:
    db 3
@note:
    dw 0
@end:

;; Changes track volume
track_vol:
    call track_command_header
    ld hl, (current_position)
    ld a, (hl)
    ld (@vol), a

    inc hl
    ld (current_position), hl

    ld hl, @cmd
    ld bc,  2
    rst.lil $18
    jp track_tick
@cmd:
    db 2
@vol:
    db 0
@end:

;; Changes track wave form
track_wave:
    ld a, (hl)
    ld (@wave), a

    inc hl
    ld (current_position), hl

    call track_command_header
    ld hl, @cmd
    ld bc, 2
    rst.lil $18
    jp track_tick
@cmd:
    db 4
@wave:
    db 0
@end:

track_command_header:
    ld hl, @cmd
    ld bc, 4
    rst.lil $18
    ret
@cmd:
    db 23, $00, $85

;; Player variables starts here(and also track_channel is last byte of command header too)
track_channel:
    db  $00
;; Wait cycles to next frame of music
delay_frames:
    db  0
;; Pointer to current position in song
current_position:
    dl track
;; And just track that you'll play 
track:
    incbin "music.binary"

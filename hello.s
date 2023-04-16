    .assume  adl = 1
    .org $40000
    
argv_ptrs_max:    equ  16
      
	jp  _start  

_exec_name:    db  "HELLO.BIN", 0

      align  64
      
      db  "MOS"
      db  00h
      db  01h
;
; And the code follows on immediately after the header
;
_start:    push  af    
      PUSH  BC
      push  de
      push  ix
      push  iy
      
      ld  ix, argv_ptrs  
      push  ix
      call  _parse_params  
      pop  ix      
      ld  b, 0    
      call  _main    
      
      pop  iy    
      pop  ix
      pop  de
      pop  bc
      pop  af
      ld hl,0
      ret

_main:
      ld hl, string
      call prstr
      ret
            
_parse_params:    
            ld  bc, _exec_name
      ld  (ix+0), BC
      inc  ix
      inc  ix
      inc  ix
      call  _skip_spaces
;
      ld  bc, 1
      ld  b, argv_ptrs_max - 1
;
_parse_params_1:  
      push  bc        
      push  hl    
      call  _get_token    
      ld  a, c    
      pop  de    
      pop  bc      
      or  A      
      ret  Z
;
      ld  (ix+0), de  
      push  HL    
      pop  de
      call  _skip_spaces  
      xor  a
      ld  (de), a    
      inc  ix
      inc  ix
      inc  ix    
      inc  C    
      ld  a, c  
      cp  b
      jr  c, _parse_params_1  
      ret

_get_token:    ld  c, 0    
@@:      ld  a, (hl)    
      or  a    
      ret   z
      cp  13    
      ret  z
      cp  ' '    
      ret  z
      inc  hl    
      inc   c    
      JR  @B
  
_skip_spaces:    ld  a, (hl)
      cp  ' '    
      ret  nz
      inc  hl  
      jr  _skip_spaces

prstr:
      ld a,(hl)
      OR a
      ret z
      rst.lil 10h
      inc   hl
      jr    prstr

; Storage for the argv array pointers
;
argv_ptrs:    blkp  argv_ptrs_max, 0      
string:       db "Hello ez80ASM!\r\n",0

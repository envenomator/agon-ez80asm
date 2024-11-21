; Test macro with a 32-character name
; Test macro with a 64+2 char substitute for filedata
; Test macro argument maximum length of 32

 .macro A2345678901234567890123456789012 arg, filedata, longarg8901234567890123456789012
 .db 23, 27, 0
 .db arg
 .db 23, 27, 1
 .dw 40, 40
 incbin filedata
 .db longarg8901234567890123456789012

 .endmacro

 A2345678901234567890123456789012 0, "longfilename3456789012345678901234567890123456789012345678901234", 0xFF

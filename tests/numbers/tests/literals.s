label1:  
        db 'a'
        db 'a',0
        db 'a', 'b', 0
        db 'a' , 'b' , 0
        db 'b';comment
        db 'b'	; comment
charliterals:
        db '\n'
        db '\r'
        db '\t'
        db '\b'
        db '\\'
        db '\'
        db "\'" ; ZDS assembler can't handle '\'', we'll test this later in string
        db '\"'
label2:
        db "A string",0
        db "\""
        db "\"\""
        db "\'"
        db "\'\'"
        db "\"\'"
        db "\'\""
        db "\"A double quoted string\"",0
        db "\'A single quoted string\'",0
        db "\"A weirdly quoted string\'",0
        db "\nString with\n newlines\n",0
        db "\rString with\r returns\r",0
        db "\tString with\t tabs\t",0
        db "\bString with\b backspaces\b",0
        db "\\String with\\ backslashed\\",0
        ; Strings with terminators, to test the various getToken* routines
        db ","
        db " , "
        db ";"
        db " ; "
        db "="
        db " = "
        db ",;="
        db ",=;"
        db ";,="
        db ";=,"
        db "=,;"
        db "=;,"
        db ",","=",";"
        db ",","=",";";comment
        ; Strings with operators, to test correct handling of transparency
        db "!#$%&()*+'-./0123456789:;<=>?<<>>[\\]^_`{|}"
        ; Strings with all normal characters
        db "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        db "abcdefghijklmnopqrstuvwxyz"
        ; literals with terminators
        db ','
        db ';'
        db '='
        ;db "";empty string - ZDS assembler can't handle this

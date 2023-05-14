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
        db "\'" ; ZDS assembler can't handle '\'', we'll test this later in string
        db '\"'
label2:
        db "A string",0
        db "\"A double quoted string\"",0
        db "\'A single quoted string\'",0
        db "\"A weirdly quoted string\'",0
        db "\nString with\n newlines\n",0
        db "\rString with\r returns\r",0
        db "\tString with\t tabs\t",0
        db "\bString with\b backspaces\b",0
        db "\\String with\\ backslashed\\",0
        ;db "";empty string - ZDS assembler can't handle this
        

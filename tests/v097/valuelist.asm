    macro testmacro repl
    db 15+repl,repl+repl
    endmacro

    testmacro 15

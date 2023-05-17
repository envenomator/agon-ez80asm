    macro initdata foo,bar
    db foo
    dw bar
    endmacro

    initdata 42,$1337

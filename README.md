# agon-ez80asm
## Supported Assembler directives
All directives can be used with, or without leading dot (.)

---
### ADL mode
    .assume ADL = expression
Set ADL status for the ez80 CPU. Expression is the value for the ADL mode, either 0 or 1

---
### Define byte(s)
    .db | .defb [value | string] [, ...]

Expression is a list of values or strings, separated by a comma. Strings of bytes need to be surrounded by double quotes. Within a string, these escape characters are supported:

\n \r \\\ \\"

Strings are not terminated by a null character. The .asciiz directive will do that.

Values can be in hex, binary, decimal, or an ascii value.

Examples

    .db 0x15
    .db 0x15,0x30
    .db 'Y'
    .db "Hello world"
    .db "Hello world",0
    .db "Hello world\n",0

---
### Define space
    .ds | .defs num [,val]

Defines storage locations. It takes one or two arguments, num and val. It reserves num bytes of space and initializes them to val. If val is omitted, it defaults to 0.

Example: reserve 10 bytes of uninitialized space

    .ds 10

Example: reserve 10 bytes, each set to 0xFF

    .ds 10, 0xFF

---

## ASCII string
    .ascii stringexpression

Store an ASCII string as bytes. Takes just one argument.

Example:

    .ascii "Test"

Results in these bytes stored: 0x74, 0x65, 0x73, 0x74

---

## ASCII zero-terminated string
    .asciiz stringexpression

Store an ASCII zero-terminated string. Takes just one argument.

Example:

    .asciiz "Test"

Results in these bytes stored: 0x74, 0x65, 0x73, 0x74, 0x00

---


## Define label
    <label> .equ value

Store a label with the given value.


---

# agon-ez80asm
## Global labels
Labels are defined as first token on a line, terminated by a colon (':'). No space is allowed between the label and the colon for the label to be recognized. 

Global labels cannot have the same name as ez80 mnemonics, nor assembler directives.

Global labels are case-sensitive.
## Local labels
Local labels start with the '@' symbol and define a temporary label between two global labels. As soon as a new global label starts, all local labels become out of scope and a new scoped local label space starts.

64 local labels are currently supported in each scope.

Local labels are case-sensitive.

## Supported Assembler directives
All directives can be used with, or without leading dot (.)

---
### ADL mode
    .assume ADL = expression
Sets ADL status for the ez80 CPU. Expression is the value for the ADL mode, either 0 or 1

---
### Define byte(s)
    .db | .defb [value | string] [, ...]

Stores bytes as a predefined value. Expression is a list of values or strings, separated by a comma. Values can be in hex, binary, decimal, or an ascii value. Strings of bytes need to be surrounded by double quotes. Within a string, these escape characters are supported and converted to their respective ascii number:

\n \r \t \e \b \\\ \\' \\"

Strings are not automatically terminated by a null character. You can use either an additional ,0 value after the specified string, or use the .asciiz directive.

Examples

    .db 0x15
    .db 0x15,0x30
    .db 'Y'
    .db "Hello world"
    .db "Hello world",0
    .db "Hello world\n",0

---
### Define word(s)
    .dw | .defb [value | label] [, ...]

Stores the expression as a word in memory, according to the current ADL mode. Expression is a list of values or labels, separated by a comma.

- ADL mode 0: store word as 2 bytes
- ADL mode 1: store word as 3 bytes

Storing a value larger than 2 byte using ADL mode 0 will throw an error.

Examples

    .dw 0xAA00, 0x4000
    .dw labelA, labelB
    .dw labelC

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

Store an ASCII string as bytes. Takes just one argument. Will not zero-terminate the string

Example:

    .ascii "Test"

Results in these bytes stored: 0x74, 0x65, 0x73, 0x74

---

## ASCII zero-terminated string
    .asciiz stringexpression

Store an ASCII zero-terminated string as bytes. Takes just one argument.

Example:

    .asciiz "Test"

Results in these bytes stored: 0x74, 0x65, 0x73, 0x74, 0x00

---

## Define label
    <label> .equ value

Store a label with the given value.

---

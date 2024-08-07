# agon-ez80asm
## Installation and requirements
### Latest version
Download the latest [release](https://github.com/envenomator/agon-ez80asm/releases) and place the file 'ez80asm.bin' in the /bin/ folder of your microSD card. 

The assembler requires at least Console8 MOS 2.2.0, which supports a /bin/ folder for executable binaries.

### Older release versions (up to and including 1.4)
Download an older release and place both 'ez80asm.bin' and 'ez80asm.ldr' in the /mos/ folder of your microSD card.
These versions require at least MOS 1.03.

## Usage

    ez80asm <filename> [output filename] [OPTION]
    -v List version information
    -h List help information
    -o Org start address in hexadecimal format, default is 40000
    -b Fillbyte in hexadecimal format, default is FF
    -a ADL mode 1/0, default is 1
    -l Listing to file with .lst extension
    -s Export symbols
    -d Direct listing to console
    -c No color codes in output (version 1.3+)
    -x Display assembly statistics (version 1.1+)

The given filename will be assembled into these files:
- filename.bin -- output executable file
- filename.lst -- output assembler listing (optionally selected by -l flag)
- filename.symbols -- symbol export when the -s option is used

## Defaults

The compiler defaults to ADL=1 mode. This can be overridden using the .ASSUME directive.

The default address is 0x40000.

## Source file format
Lines in the source file should have the following format:

    [Label:] [Instruction] [Operand] [;Comment]

All fields are optional.

- Labels are defined as first token on a line, terminated by a colon (':')
- Instructions are either ez80 opcodes, or assembler directives. Opcodes without a label should start with whitespace. Assembler directives can start with a dot (.)
- Comments start with a semi-colon (;). A line with just a comment can start without whitespace

Examples

    LABEL:                  ; Label without operator
    LABEL:LD A,B            ; Instruction after label without whitespace
    LABEL: LD A,B           ; Instruction after label with whitespace
     LD A,B                     ; Instruction without label, starting with whitespace
    ; comment on empty line

## Global labels

Global labels are reachable in the entire source file and cannot have the same name as ez80 mnemonics, condition names, number values, nor assembler directives.

- Global labels are case-sensitive.
- Global labels cannot start with a '$' character.
- Numbers cannot be used as a label name
- Register names can be used as a label name, however doing so is not recommended.

### EQU labels
By using the EQU directive, global labels can be defined as a number constant instead of an address. These constants can be used in places where a number is expected in an operand. They can also be used with other directives; however the directive determines if such a constant can be defined before and/or after the directive itself. See the table below for additional details:

|        **Directive**        | **EQU allowed before directive** | **EQU allowed after directive** |
|:---------------------------:|:--------------------------------:|:-------------------------------:|
|            ALIGN            |                Yes               |                No               |
|          ASSUME ADL         |                Yes               |                No               |
|   DB/DEFB/ASCII/ASCIZ/BYTE  |                Yes               |               Yes               |
|     DW/DEFW/DL/DW24/DW32    |                Yes               |               Yes               |
| DS/DEFS/BLKB/BLKW/BLKP/BLKL |                Yes               |     No - number, Yes - value     |
|             EQU             |                Yes               |                No               |
|           FILLBYTE          |                Yes               |                No               |
|        INCLUDE/INCBIN       |                No                |                No               |
|              IF             |                Yes               |                No               |
|             ORG             |                Yes               |                No               |


For example, the DB directive accepts labels defined after the DB directive; this doesn't impact the output and the addressing following the directive. Only the number of arguments determines the number of bytes to be defined. The actual content will be parsed during pass 2, when all global labels are known to the assembler. The same is true for data directives like dw/dl etc.

There are a number of directives where a label, given as an argument, can only be used when the label has been defined previously, before the directive. As an example, the BLKB directive specifies a number of bytes and their value. As the number of bytes impacts the output size and addressing, this needs to be defined in pass 1, so any label used for the number of bytes needs to be defined before using it in a BLKB statement.

## Local labels
Local labels start with the '@' symbol, terminated by a colon (':') and define a temporary label between two global labels. As soon as a new global label starts, all local labels become out of scope and a new scoped local label space starts.

- Local labels are case-sensitive.
- Up to version 1.3, 64 local labels are currently supported in each scope. Version 1.4+ allows an infinite number, limited by the amount of available memory

Up to version 1.3, the 'scope' of local labels was tracked using a file. This proved to be relatively costly method performance-wise. Starting version 1.4, all local labels are prefixed with their scope and placed in the memory label table. Local labels defined before a global label definition, are prefixed with their source filename. Local labels defined within the scope of a global label are prefixed with their global label.
While this makes it possible to immediately reference them from outside of their immediate scope by using their prefix, it is not recommended to do so; this method might change in a future release.

## Anonymous labels
Anonymous labels are labeled '@@', terminated by a colon (':') and define a temporary label. As soon as a new anonymous label is defined, the previous is no longer reachable.
Code can use @f/@n or @b/@p, to get the label in the FORWARD/NEXT, or BACKWARD/PREVIOUS direction.

## Numeric constants

The following formats are supported:

- Hexadecimal - constants starting with 0x, 0X, $, or #
- Hexadecimal - constants ending with h or H
- Binary - constants starting with 0b, 0B or %
- Binary - constants ending with b, B
- Decimal

Examples:

    $0A     ; Hex
    0x0A    ; Hex
    #0A     ; Hex
    0Ah     ; Hex
    0Bh     ; Hex (not binary - 'h' suffix takes priority over '0B' prefix)
    0b1010  ; Bin
    1010b   ; Bin
    %1010   ; bin
    10      ; Dec

$ represents the current program counter

## Character constants (literals)
Character constants are surrounded by single quotes. Supported escape sequences are:

\\a - (0x07) - Alert, beep (v1.3+)

\\b - (0x08) - Backspace

\\e - (0x1B) - Escape (v1.3+)

\\f - (0x0C) - Formfeed (v1.3+)

\\n - (0x0A) - Newline

\\r - (0x0D) - Carriage return

\\t - (0x09) - Horizontal tab

\\v - (0x0B) - Vertical tab (v1.3+)

\\\ - (0x5C) - backslash

\\' - (0x27) - Single quotation mark

\\" - (0x22) - Double quotation mark

\\? - (0x3F) - Question mark (v1.3+)

Examples:

    DB 'a'  ; Defines byte with hex value 61
    DB 'B'  ;                   hex value 41
    DB '\n' ;                   hex value 0A

## String expressions
Strings are defined as literal characters surrounded by double quotes. The supported character escape sequence constants are supported in strings as well.

Examples:

    DB "Test"   ; Outputs hex values 54, 65, 73, 74
    DB "Test",0 ; Outputs hex values 54, 65, 73, 74, 0
    DB "Test\n" ; Outputs hex values 54, 65, 73, 74, 0A


## Supported Assembler directives
All assembler directives can be used with, or without leading dot (.)



| Directive              | Description                                                                        | Usage                                                                                                                                                                                                                                                                                                                                                                     |
|------------------------|------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| ALIGN                  |  Select alignment for the program counter, pad bytes up to new boundary alignment  | ALIGN \<boundary\>  The boundary, expressed in bytes, must be a power of two (1,2,4,8 etc)  Example: ALIGN 256 ; align next instruction at boundary of 256                                                                                                                                                                                                                |
| ASSUME ADL             | Sets default ADL status (0 or 1)                                                   | Example: ASSUME ADL=1 ; set ADL mode to 1                                                                                                                                                                                                                                                                                                                                 |
| BLKB/BLKW/ BLKP/BLKL   | Allocate a block with a number of Bytes/ 16-bit words/24-bit Pointers/32-bit words | BLKx \<number\> [, value]  Example: BLKB 16 ; allocate 16 uninitialized bytes Example: BLKB 16, 0xFF ; allocate 16 bytes and initialize them to 0xFF                                                                                                                                                                                                                      |
| DB / DEFB ASCII / BYTE | Define byte(s)                                                                     | DB \| DEFB \| ASCII \| BYTE \<value \| string\> [, ...]  Reserve and initialize a byte, or a list of bytes. Within a string, these escape characters are supported and converted to their respective ascii code: \ \r \t \b \\\ \\' \\"  Strings are not automatically terminated by a null byte. You can use either an additional ',0' value, or use the ASCIZ directive |
| ASCIZ                  | Same as above, but terminated with a 0 Used for zero-terminated strings mostly     | ASCIZ \<value \| string\> [, ...]                                                                                                                                                                                                                                                                                                                                         |
| DW / DEFW              | Define 16-bit word(s)                                                              | DW \| DEFW \<value\> [, ...]  Reserve and initialize a word/long value, or a list of values                                                                                                                                                                                                                                                                               |
| DL / DW24              | Define 24-bit word(s)                                                              | DL \| DW24 \<value\> [, ...]                                                                                                                                                                                                                                                                                                                                              |
| DW32                   | Define 32-bit word(s)                                                              | DW32 \<value\> [, ...]                                                                                                                                                                                                                                                                                                                                              |
| DS / DEFS              | Defines storage location in bytes                                                  | DS \| DEFS \<number\> Example: DS 10 ; reserve 10 byte. If a .DS space is defined between opcodes, the fillbyte value is used in output to the binary file.                                                                                                                                                                                                                             |
| EQU                    | Assign symbolic name to label                                                      | Example: LABEL: EQU 0xFF                                                                                                                                                                                                                                                                                                                                                  |
| FILLBYTE               | Change the byte value that is used for filling unused space                        | FILLBYTE \<value\>                                                                                                                                                                                                                                                                                                                                                        |
| INCBIN                 | Include binary file                                                                | Allows the insertion of binary data from another file  Example: INCBIN "sprite.bin"                                                                                                                                                                                                                                                                                       |
| INCLUDE                | Include file in source code                                                        | Allows the insertion of source code from another file into the current source file during assembly. The included file is assembled into the current source file immediately after the directive. When the EOF (End of File) of the included file is reached, the assembly resumes on the line after the INCLUDE directive  Example: INCLUDE "example.inc"                 |
| MACRO / ENDMACRO       | Define a macro, see below for detailed explanation                                 | MACRO [arg1, arg2 ...]  [macro body] ENDMACRO                                                                                                                                                                                                                                                                                                                             |
| ORG                    | Define location counter origin.                                        | Sets the assembler location counter to a specified value. The directive must be followed by an integer constant, which is the value of the new origin. Example: ORG $40000. Starting release 1.9, when the location counter is advanced, the intervening bytes are filled with the defined fillbyte.ORG may only increase the location counter, or leave it unchanged; you cannot use ORG to move the location counter backwards.                                                                                                                                                                                                |

## Macros
The 'macro' directive defines a macro, optionally followed by a maximum of 8 arguments. The following lines will be stored as the macro-body, until the 'endmacro' directive is encountered. A macro has to be defined before use.

Macro names have a maximum length of 16 characters, arguments can be 32 character maximum.

Example macro definition without arguments:

        macro addhla ; start of macro definition
        add a,l
        jr nc, @1
        inc h
    @1:
        ld l,a
        endmacro    ; end of macro definition

Global labels are not allowed in macro definitions, but local/anonymous labels are.
Nested macros are not currently supported.

Example macro with arguments:

        macro pointless arg1, arg2
        add a, arg1
        add l, arg2
        endmacro

Example macro usage / expansion later in the code:

        pointless 10, 15
        ; will expand to add a, 10
        ;                add l, 15

## Simple conditional assembly support
The assembler supports simple conditional assembly using if/else/endif assembler directives. There is no support for nested conditions.

The 'if' directive assesses the value of the given value, usually a label. When this is non-zero, the lines following the directive are assembled up to the 'else' or 'endif' statement. If the value is zero, any lines after an appearing 'else' statement are assembled up to the 'endif' statement.

## Known limitations

- Operators in expressions currently have no precendence other than left-to-right and parenthesis aren't supported yet. So be careful using more than one operator in an expression

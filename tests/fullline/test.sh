#!/bin/bash
# Positive test
# return 0 on succesfull test
# return 1 on issue during test
# return 2 on error in test SETUP
#
if [ -f "test.s" ]; then
    $ASMBIN test.s > asm.output
    if [ $? -eq 1 ]; then 
        echo "Assembler error(s)"
        exit 1
    else
        echo "Assembler done"
    fi
    if [ -f "reference.bin" ]; then
        diff test.bin reference.bin >/dev/null
        if [ $? -eq 1 ]; then 
            echo "Binary output incorrect"
            exit 1
        else
            echo "Binary output correct"
        fi
    else
        echo "No binary check performed"
    fi
else
    echo "test.s not present"
    exit 2
fi
exit 0

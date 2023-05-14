#!/bin/bash
# Positive test
# return 0 on succesfull test
# return 1 on issue during test
# return 2 on error in test SETUP
#
CHECKBIN=1
#
if [ -f "test.s" ]; then
    $ASMBIN test.s > asm.output
    if [ $? -eq 1 ]; then 
        echo "Assembler error(s)"
        exit 1
    else
        echo "Assembler done"
    fi
    if [ $CHECKBIN -eq 1 ]; then
        echo "Performing binary check"
        if [ -f "reference.bin" ]; then
            diff test.bin reference.bin >/dev/null
            if [ $? -eq 1 ]; then 
                echo "Binary output incorrect"
                exit 1
            else
                echo "Binary output correct"
            fi
        else
            echo "Missing reference.bin"
            exit 2
        fi
    fi
else
    echo "test.s not present"
    exit 2
fi
exit 0

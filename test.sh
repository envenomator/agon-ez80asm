#!/bin/bash
export ASMBIN=../../bin/asm

pass=1
testresult=2

cd tests
for FILE in *; do
    if [ -d "$FILE" ]; then
        cd "$FILE"
        if [ -f "test.sh" ]; then
            echo -n "Testing \"$FILE\" - "
            ./test.sh > ./test.output
            testresult=$?
            if [ $testresult -eq 1 ]; then
                echo "FAIL"
                pass=0
            else
                if [ $testresult -eq 2 ]; then
                    echo "ERROR"
                    exit 1
                fi
                echo "OK"
            fi
        else
            echo "Testscript missing for $FILE"
            exit 1
        fi
        cd ..
    fi
done
cd ..

if [ $pass -eq 1 ]; then echo "All tests passed"
else echo "Test(s) failed"
fi

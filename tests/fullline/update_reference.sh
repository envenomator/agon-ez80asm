#!/bin/bash
echo "Assembling source file"
spasm -E test.s
echo "Creating reference.bin"
mv test.bin reference.bin
echo "Done"

#!/bin/bash
echo "Assembling source file"
spasm -E reference.s
echo "Creating reference.bin"
echo "Done"

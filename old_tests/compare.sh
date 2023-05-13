#!/bin/bash
spasm -E tests/a.txt > /dev/null
sleep 0.5
sha256sum tests/a.bin
rm tests/a.bin
bin/asm tests/a.txt > /dev/null
sleep 0.5
sha256sum tests/a.bin
echo
spasm -E tests/b.txt > /dev/null
sleep 0.5
sha256sum tests/b.bin
rm tests/a.bin
bin/asm tests/b.txt > /dev/null
sleep 0.5
sha256sum tests/b.bin
echo
spasm -E tests/c.txt > /dev/null
sleep 0.5
sha256sum tests/c.bin
rm tests/c.bin
bin/asm tests/c.txt > /dev/null
sleep 0.5
sha256sum tests/c.bin
echo
spasm -E tests/d-i.txt > /dev/null
sleep 0.5
sha256sum tests/d-i.bin
rm tests/d-i.bin
bin/asm tests/d-i.txt > /dev/null
sleep 0.5
sha256sum tests/d-i.bin
echo
spasm -E tests/j.txt > /dev/null
sleep 0.5
sha256sum tests/j.bin
rm tests/j.bin
bin/asm tests/j.txt > /dev/null
sleep 0.5
sha256sum tests/j.bin
echo
spasm -E tests/l.txt > /dev/null
sleep 0.5
sha256sum tests/l.bin
rm tests/l.bin
bin/asm tests/l.txt > /dev/null
sleep 0.5
sha256sum tests/l.bin
echo
spasm -E tests/m.txt > /dev/null
sleep 0.5
sha256sum tests/m.bin
rm tests/m.bin
bin/asm tests/m.txt > /dev/null
sleep 0.5
sha256sum tests/m.bin
echo
spasm -E tests/test.s > /dev/null
sleep 0.5
sha256sum tests/test.bin
rm tests/test.bin
bin/asm tests/test.s > /dev/null
sleep 0.5
sha256sum tests/test.bin
echo


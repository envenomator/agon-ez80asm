#!/bin/bash
spasm -E tests/a.txt > /dev/null
sleep 0.5
sha256sum tests/a.bin
rm tests/a.bin
bin/project tests/a.txt > /dev/null
sleep 0.5
sha256sum tests/a.bin
echo
spasm -E tests/b.txt > /dev/null
sleep 0.5
sha256sum tests/b.bin
rm tests/a.bin
bin/project tests/b.txt > /dev/null
sleep 0.5
sha256sum tests/b.bin
echo
spasm -E tests/c.txt > /dev/null
sleep 0.5
sha256sum tests/c.bin
rm tests/c.bin
bin/project tests/c.txt > /dev/null
sleep 0.5
sha256sum tests/c.bin
echo
spasm -E tests/d-i.txt > /dev/null
sleep 0.5
sha256sum tests/d-i.bin
rm tests/d-i.bin
bin/project tests/d-i.txt > /dev/null
sleep 0.5
sha256sum tests/d-i.bin
echo
spasm -E tests/j.txt > /dev/null
sleep 0.5
sha256sum tests/j.bin
rm tests/j.bin
bin/project tests/j.txt > /dev/null
sleep 0.5
sha256sum tests/j.bin
echo
spasm -E tests/l.txt > /dev/null
sleep 0.5
sha256sum tests/l.bin
rm tests/l.bin
bin/project tests/l.txt > /dev/null
sleep 0.5
sha256sum tests/l.bin
echo
spasm -E tests/m.txt > /dev/null
sleep 0.5
sha256sum tests/m.bin
rm tests/m.bin
bin/project tests/m.txt > /dev/null
sleep 0.5
sha256sum tests/m.bin
echo
spasm -E tests/test.s > /dev/null
sleep 0.5
sha256sum tests/test.bin
rm tests/test.bin
bin/project tests/test.s > /dev/null
sleep 0.5
sha256sum tests/test.bin
echo


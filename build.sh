#!/bin/bash
# Build script for Linux/Unix
# Builds the test extraction program

cd test || exit 1

echo "Building test program..."

# Try different compilers in order of preference
if command -v gcc &> /dev/null; then
    gcc test.c -o test -std=c99 -Wall -I..
    echo "Build successful! Run with: cd test && ./test"
elif command -v clang &> /dev/null; then
    clang test.c -o test -std=c99 -Wall -I..
    echo "Build successful! Run with: cd test && ./test"
else
    echo "Error: No C compiler found. Please install gcc or clang."
    exit 1
fi


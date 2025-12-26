#!/bin/bash
# Build script for Linux/Unix

echo "Building test program..."

# Try different compilers in order of preference
if command -v gcc &> /dev/null; then
    gcc test.c -o test -std=c99 -Wall
    echo "Build successful! Run with: ./test"
elif command -v clang &> /dev/null; then
    clang test.c -o test -std=c99 -Wall
    echo "Build successful! Run with: ./test"
else
    echo "Error: No C compiler found. Please install gcc or clang."
    exit 1
fi


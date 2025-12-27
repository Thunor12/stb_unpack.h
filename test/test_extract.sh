#!/bin/bash
# Test script for TAR extraction in WSL/Linux

set -e  # Exit on error

# Remove old archive if it exists
rm -f output/archive.tar

# Create a fresh TAR archive with test_file.txt (change to input dir so archive doesn't include "input/" prefix)
(cd input && tar cf ../output/archive.tar test_file.txt) >/dev/null 2>&1

if [ ! -f build/test ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

rm -rf output/out
./build/test >/dev/null 2>&1

if [ -f "output/out/test_file.txt" ]; then
    if cmp -s input/test_file.txt output/out/test_file.txt >/dev/null 2>&1; then
        exit 0
    else
        echo "✗ File contents do not match!"
        echo "Original file:"
        cat input/test_file.txt
        echo ""
        echo "Extracted file:"
        cat output/out/test_file.txt
        exit 1
    fi
else
    echo "✗ File was not extracted!"
    echo "Files in 'output/out' directory:"
    ls -la output/out/ 2>/dev/null || echo "  (directory doesn't exist or is empty)"
    exit 1
fi


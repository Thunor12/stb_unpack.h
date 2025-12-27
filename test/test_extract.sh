#!/bin/bash
# Test script for TAR extraction in WSL/Linux

set -e  # Exit on error

# Remove old archive if it exists
rm -f archive.tar

# Create a fresh TAR archive with test_file.txt
tar cf archive.tar test_file.txt >/dev/null 2>&1

if [ ! -f test ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

rm -rf out
./test >/dev/null 2>&1

if [ -f "out/test_file.txt" ]; then
    if cmp -s test_file.txt out/test_file.txt >/dev/null 2>&1; then
        exit 0
    else
        echo "✗ File contents do not match!"
        echo "Original file:"
        cat test_file.txt
        echo ""
        echo "Extracted file:"
        cat out/test_file.txt
        exit 1
    fi
else
    echo "✗ File was not extracted!"
    echo "Files in 'out' directory:"
    ls -la out/ 2>/dev/null || echo "  (directory doesn't exist or is empty)"
    exit 1
fi


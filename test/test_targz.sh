#!/bin/bash
# Test script for .tar.gz extraction and creation

set -e  # Exit on error

if [ ! -f test_targz ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

echo "Hello from .tar.gz test!" > test_targz_input.txt
echo "This file will be compressed." >> test_targz_input.txt
echo "Multiple lines of content." >> test_targz_input.txt

rm -f test_archive.tar.gz
./test_targz -c test_archive.tar.gz test_targz_input.txt >/dev/null 2>&1

if [ ! -f test_archive.tar.gz ]; then
    echo "✗ Failed to create .tar.gz archive!"
    exit 1
fi

rm -rf targz_out
mkdir -p targz_out

if ! ./test_targz test_archive.tar.gz targz_out >/dev/null 2>&1; then
    echo "✗ Extraction failed!"
    exit 1
fi

if [ -f "targz_out/test_targz_input.txt" ]; then
    if cmp -s test_targz_input.txt targz_out/test_targz_input.txt >/dev/null 2>&1; then
        exit 0
    else
        echo "✗ File contents do not match!"
        echo "Original file:"
        cat test_targz_input.txt
        echo ""
        echo "Extracted file:"
        cat targz_out/test_targz_input.txt
        exit 1
    fi
else
    echo "✗ File was not extracted!"
    echo "Files in targz_out directory:"
    ls -la targz_out/ 2>/dev/null || echo "  (directory doesn't exist or is empty)"
    exit 1
fi


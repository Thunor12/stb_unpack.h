#!/bin/bash
# Test script for .tar.gz extraction and creation

set -e  # Exit on error

if [ ! -f build/test_targz ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

echo "Hello from .tar.gz test!" > input/test_targz_input.txt
echo "This file will be compressed." >> input/test_targz_input.txt
echo "Multiple lines of content." >> input/test_targz_input.txt

rm -f output/test_archive.tar.gz
(cd input && ../build/test_targz -c ../output/test_archive.tar.gz test_targz_input.txt) >/dev/null 2>&1

if [ ! -f output/test_archive.tar.gz ]; then
    echo "✗ Failed to create .tar.gz archive!"
    exit 1
fi

rm -rf output/targz_out
mkdir -p output/targz_out

if ! ./build/test_targz output/test_archive.tar.gz output/targz_out >/dev/null 2>&1; then
    echo "✗ Extraction failed!"
    exit 1
fi

if [ -f "output/targz_out/test_targz_input.txt" ]; then
    if cmp -s input/test_targz_input.txt output/targz_out/test_targz_input.txt >/dev/null 2>&1; then
        exit 0
    else
        echo "✗ File contents do not match!"
        echo "Original file:"
        cat input/test_targz_input.txt
        echo ""
        echo "Extracted file:"
        cat output/targz_out/test_targz_input.txt
        exit 1
    fi
else
    echo "✗ File was not extracted!"
    echo "Files in output/targz_out directory:"
    ls -la output/targz_out/ 2>/dev/null || echo "  (directory doesn't exist or is empty)"
    exit 1
fi


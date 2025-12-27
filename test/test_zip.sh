#!/bin/bash
# Test script for .zip extraction and creation

set -e  # Exit on error

if [ ! -f build/test_zip ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

echo "Hello from .zip test!" > input/test_zip_input.txt
echo "This file will be compressed." >> input/test_zip_input.txt
echo "Multiple lines of content." >> input/test_zip_input.txt

rm -f output/test_archive.zip
(cd input && ../build/test_zip -c ../output/test_archive.zip test_zip_input.txt) >/dev/null 2>&1

if [ ! -f output/test_archive.zip ]; then
    echo "✗ Failed to create .zip archive!"
    exit 1
fi

rm -rf output/zip_out
mkdir -p output/zip_out

if ! ./build/test_zip output/test_archive.zip output/zip_out >/dev/null 2>&1; then
    echo "✗ Extraction failed!"
    exit 1
fi

if [ -f "output/zip_out/test_zip_input.txt" ]; then
    if cmp -s input/test_zip_input.txt output/zip_out/test_zip_input.txt >/dev/null 2>&1; then
        exit 0
    else
        echo "✗ File contents do not match!"
        echo "Original file:"
        cat input/test_zip_input.txt
        echo ""
        echo "Extracted file:"
        cat output/zip_out/test_zip_input.txt
        exit 1
    fi
else
    echo "✗ File was not extracted!"
    echo "Files in output/zip_out directory:"
    ls -la output/zip_out/ 2>/dev/null || echo "  (directory doesn't exist or is empty)"
    exit 1
fi


#!/bin/bash
# Test script for .tar.gz extraction and creation

set -e  # Exit on error

echo "=== Checking for test_targz executable ==="
if [ ! -f test_targz ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

echo ""
echo "=== Creating test file ==="
echo "Hello from .tar.gz test!" > test_targz_input.txt
echo "This file will be compressed." >> test_targz_input.txt
echo "Multiple lines of content." >> test_targz_input.txt

echo ""
echo "=== Creating .tar.gz archive ==="
rm -f test_archive.tar.gz
./test_targz -c test_archive.tar.gz test_targz_input.txt

if [ ! -f test_archive.tar.gz ]; then
    echo "✗ Failed to create .tar.gz archive!"
    exit 1
fi

echo "Archive created: test_archive.tar.gz"
echo "Archive size: $(stat -c%s test_archive.tar.gz 2>/dev/null || stat -f%z test_archive.tar.gz 2>/dev/null) bytes"

echo ""
echo "=== Extracting .tar.gz archive ==="
rm -rf targz_out
mkdir -p targz_out

if ./test_targz test_archive.tar.gz targz_out; then
    echo "✓ Extraction succeeded!"
else
    echo "✗ Extraction failed!"
    exit 1
fi

echo ""
echo "=== Verifying extracted file ==="
if [ -f "targz_out/test_targz_input.txt" ]; then
    echo "✓ File extracted successfully!"
    echo ""
    echo "Original file:"
    cat test_targz_input.txt
    echo ""
    echo "Extracted file:"
    cat targz_out/test_targz_input.txt
    echo ""
    
    # Compare with original
    if cmp -s test_targz_input.txt targz_out/test_targz_input.txt; then
        echo "✓ File contents match original perfectly!"
        echo ""
        echo "=== Test PASSED ==="
        exit 0
    else
        echo "✗ File contents do not match!"
        echo "=== Test FAILED ==="
        exit 1
    fi
else
    echo "✗ File was not extracted!"
    echo "Files in targz_out directory:"
    ls -la targz_out/ 2>/dev/null || echo "  (directory doesn't exist or is empty)"
    echo "=== Test FAILED ==="
    exit 1
fi


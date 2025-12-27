#!/bin/bash
# Test script for TAR extraction in WSL/Linux

set -e  # Exit on error

echo "=== Creating test TAR archive ==="
# Remove old archive if it exists
rm -f archive.tar

# Create a fresh TAR archive with test_file.txt
tar cf archive.tar test_file.txt

echo "Archive created: archive.tar"
echo "Contents:"
tar -tf archive.tar

echo ""
echo "=== Checking for test executable ==="
if [ ! -f test ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

echo ""
echo "=== Cleaning old extraction ==="
rm -rf out

echo ""
echo "=== Running extraction test ==="
./test

echo ""
echo "=== Verifying extraction ==="
if [ -f "out/test_file.txt" ]; then
    echo "✓ File extracted successfully!"
    echo "Contents of extracted file:"
    cat out/test_file.txt
    echo ""
    
    # Compare with original
    if cmp -s test_file.txt out/test_file.txt; then
        echo "✓ File contents match original!"
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
    echo "Files in 'out' directory:"
    ls -la out/ 2>/dev/null || echo "  (directory doesn't exist or is empty)"
    echo "=== Test FAILED ==="
    exit 1
fi


#!/bin/bash
# Test script to compare our TAR creation with the actual tar command

set -e  # Exit on error

echo "=== Checking for test_create executable ==="
if [ ! -f test_create ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

echo ""
echo "=== Creating test file ==="
echo "Hello, this is a test file!" > test_input.txt
echo "It has multiple lines." >> test_input.txt
echo "And some content." >> test_input.txt

echo ""
echo "=== Creating TAR with our function ==="
rm -f our_archive.tar
./test_create our_archive.tar test_input.txt

echo ""
echo "=== Creating TAR with tar command ==="
rm -f tar_archive.tar
tar cf tar_archive.tar test_input.txt

echo ""
echo "=== Comparing archives ==="
echo "Our archive size: $(stat -c%s our_archive.tar 2>/dev/null || stat -f%z our_archive.tar 2>/dev/null) bytes"
echo "Tar archive size: $(stat -c%s tar_archive.tar 2>/dev/null || stat -f%z tar_archive.tar 2>/dev/null) bytes"

if cmp -s our_archive.tar tar_archive.tar; then
    echo "✓ Archives are identical!"
    echo ""
    echo "=== Test PASSED ==="
    exit 0
else
    echo "✗ Archives differ!"
    echo ""
    echo "=== Hex dump comparison (first 512 bytes) ==="
    echo "Our archive:"
    hexdump -C our_archive.tar | head -n 20
    echo ""
    echo "Tar archive:"
    hexdump -C tar_archive.tar | head -n 20
    echo ""
    echo "=== Differences ==="
    diff <(hexdump -C our_archive.tar) <(hexdump -C tar_archive.tar) | head -n 30 || true
    echo ""
    echo "=== Test FAILED ==="
    exit 1
fi


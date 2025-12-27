#!/bin/bash
# Test script to compare our TAR creation with the actual tar command

set -e  # Exit on error

if [ ! -f build/test_create ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

echo "Hello, this is a test file!" > input/test_input.txt
echo "It has multiple lines." >> input/test_input.txt
echo "And some content." >> input/test_input.txt

# Set a fixed modification time so both archives use the same timestamp
touch -t 202001010000.00 input/test_input.txt

rm -f output/our_archive.tar output/tar_archive.tar
rm -rf output/our_extracted output/tar_extracted

# Create archives (change to input dir so archives don't include "input/" prefix)
(cd input && ../build/test_create ../output/our_archive.tar test_input.txt) >/dev/null 2>&1
(cd input && tar cf ../output/tar_archive.tar test_input.txt) >/dev/null 2>&1

# Extract both archives
mkdir -p output/our_extracted output/tar_extracted
tar xf output/our_archive.tar -C output/our_extracted >/dev/null 2>&1
tar xf output/tar_archive.tar -C output/tar_extracted >/dev/null 2>&1

# Compare extracted files (more robust than byte-for-byte archive comparison)
if [ -f "output/our_extracted/test_input.txt" ] && [ -f "output/tar_extracted/test_input.txt" ]; then
    if cmp -s output/our_extracted/test_input.txt output/tar_extracted/test_input.txt >/dev/null 2>&1; then
        # Also verify against original
        if cmp -s input/test_input.txt output/our_extracted/test_input.txt >/dev/null 2>&1; then
            exit 0
        else
            echo "✗ Extracted file doesn't match original!"
            exit 1
        fi
    else
        echo "✗ Extracted files differ!"
        echo "Our archive extracted:"
        cat output/our_extracted/test_input.txt
        echo ""
        echo "Tar archive extracted:"
        cat output/tar_extracted/test_input.txt
        exit 1
    fi
else
    echo "✗ Failed to extract files!"
    echo "Our extracted: $([ -f output/our_extracted/test_input.txt ] && echo 'exists' || echo 'missing')"
    echo "Tar extracted: $([ -f output/tar_extracted/test_input.txt ] && echo 'exists' || echo 'missing')"
    exit 1
fi


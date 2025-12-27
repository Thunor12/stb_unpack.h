#!/bin/bash
# Test script to compare our TAR creation with the actual tar command

set -e  # Exit on error

if [ ! -f test_create ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

echo "Hello, this is a test file!" > test_input.txt
echo "It has multiple lines." >> test_input.txt
echo "And some content." >> test_input.txt

# Set a fixed modification time so both archives use the same timestamp
touch -t 202001010000.00 test_input.txt

rm -f our_archive.tar tar_archive.tar
rm -rf our_extracted tar_extracted

# Create archives
./test_create our_archive.tar test_input.txt >/dev/null 2>&1
tar cf tar_archive.tar test_input.txt >/dev/null 2>&1

# Extract both archives
mkdir -p our_extracted tar_extracted
tar xf our_archive.tar -C our_extracted >/dev/null 2>&1
tar xf tar_archive.tar -C tar_extracted >/dev/null 2>&1

# Compare extracted files (more robust than byte-for-byte archive comparison)
if [ -f "our_extracted/test_input.txt" ] && [ -f "tar_extracted/test_input.txt" ]; then
    if cmp -s our_extracted/test_input.txt tar_extracted/test_input.txt >/dev/null 2>&1; then
        # Also verify against original
        if cmp -s test_input.txt our_extracted/test_input.txt >/dev/null 2>&1; then
            exit 0
        else
            echo "✗ Extracted file doesn't match original!"
            exit 1
        fi
    else
        echo "✗ Extracted files differ!"
        echo "Our archive extracted:"
        cat our_extracted/test_input.txt
        echo ""
        echo "Tar archive extracted:"
        cat tar_extracted/test_input.txt
        exit 1
    fi
else
    echo "✗ Failed to extract files!"
    echo "Our extracted: $([ -f our_extracted/test_input.txt ] && echo 'exists' || echo 'missing')"
    echo "Tar extracted: $([ -f tar_extracted/test_input.txt ] && echo 'exists' || echo 'missing')"
    exit 1
fi


#!/bin/bash
# Test that our TAR archives can be extracted by the standard tar tool

set -e  # Exit on error

if [ ! -f build/test_create ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

echo "Hello, this is a test file!" > input/test_compat_input.txt
echo "It has multiple lines." >> input/test_compat_input.txt
echo "And some content for testing." >> input/test_compat_input.txt
echo "The quick brown fox jumps over the lazy dog." >> input/test_compat_input.txt

rm -f output/our_compat_archive.tar
(cd input && ../build/test_create ../output/our_compat_archive.tar test_compat_input.txt) >/dev/null 2>&1

if [ ! -f output/our_compat_archive.tar ]; then
    echo "✗ Failed to create archive!"
    exit 1
fi

rm -rf output/tar_extracted
mkdir -p output/tar_extracted
cd output/tar_extracted

if ! tar xf ../our_compat_archive.tar >/dev/null 2>&1; then
    echo "✗ tar extraction failed!"
    cd ../..
    exit 1
fi

cd ../..

if [ -f "output/tar_extracted/test_compat_input.txt" ]; then
    if cmp -s input/test_compat_input.txt output/tar_extracted/test_compat_input.txt >/dev/null 2>&1; then
        exit 0
    else
        echo "✗ File contents do not match!"
        echo "Original file:"
        cat input/test_compat_input.txt
        echo ""
        echo "Extracted file:"
        cat output/tar_extracted/test_compat_input.txt
        exit 1
    fi
else
    echo "✗ File was not extracted!"
    echo "Files in output/tar_extracted directory:"
    ls -la output/tar_extracted/ 2>/dev/null || echo "  (directory doesn't exist or is empty)"
    exit 1
fi


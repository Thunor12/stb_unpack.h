#!/bin/bash
# Test that our TAR archives can be extracted by the standard tar tool

set -e  # Exit on error

if [ ! -f test_create ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

echo "Hello, this is a test file!" > test_compat_input.txt
echo "It has multiple lines." >> test_compat_input.txt
echo "And some content for testing." >> test_compat_input.txt
echo "The quick brown fox jumps over the lazy dog." >> test_compat_input.txt

rm -f our_compat_archive.tar
./test_create our_compat_archive.tar test_compat_input.txt >/dev/null 2>&1

if [ ! -f our_compat_archive.tar ]; then
    echo "✗ Failed to create archive!"
    exit 1
fi

rm -rf tar_extracted
mkdir -p tar_extracted
cd tar_extracted

if ! tar xf ../our_compat_archive.tar >/dev/null 2>&1; then
    echo "✗ tar extraction failed!"
    cd ..
    exit 1
fi

cd ..

if [ -f "tar_extracted/test_compat_input.txt" ]; then
    if cmp -s test_compat_input.txt tar_extracted/test_compat_input.txt >/dev/null 2>&1; then
        exit 0
    else
        echo "✗ File contents do not match!"
        echo "Original file:"
        cat test_compat_input.txt
        echo ""
        echo "Extracted file:"
        cat tar_extracted/test_compat_input.txt
        exit 1
    fi
else
    echo "✗ File was not extracted!"
    echo "Files in tar_extracted directory:"
    ls -la tar_extracted/ 2>/dev/null || echo "  (directory doesn't exist or is empty)"
    exit 1
fi


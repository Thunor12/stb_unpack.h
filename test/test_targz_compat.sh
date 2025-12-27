#!/bin/bash
# Test that our .tar.gz archives can be extracted by standard tools

set -e  # Exit on error

if [ ! -f build/test_targz ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

echo "Compatibility test for .tar.gz" > input/test_targz_compat.txt
echo "This archive should be readable by standard tools." >> input/test_targz_compat.txt
echo "Testing gzip compression compatibility." >> input/test_targz_compat.txt

rm -f output/our_targz_archive.tar.gz
(cd input && ../build/test_targz -c ../output/our_targz_archive.tar.gz test_targz_compat.txt) >/dev/null 2>&1

if [ ! -f output/our_targz_archive.tar.gz ]; then
    echo "✗ Failed to create archive!"
    exit 1
fi

rm -rf output/tar_extracted_targz
mkdir -p output/tar_extracted_targz
cd output/tar_extracted_targz

if ! tar xzf ../our_targz_archive.tar.gz >/dev/null 2>&1; then
    echo "✗ tar extraction failed!"
    cd ../..
    exit 1
fi

cd ../..

if [ -f "output/tar_extracted_targz/test_targz_compat.txt" ]; then
    if cmp -s input/test_targz_compat.txt output/tar_extracted_targz/test_targz_compat.txt >/dev/null 2>&1; then
        exit 0
    else
        echo "✗ File contents do not match!"
        echo "Original file:"
        cat input/test_targz_compat.txt
        echo ""
        echo "Extracted file:"
        cat output/tar_extracted_targz/test_targz_compat.txt
        exit 1
    fi
else
    echo "✗ File was not extracted!"
    echo "Files in output/tar_extracted_targz directory:"
    ls -la output/tar_extracted_targz/ 2>/dev/null || echo "  (directory doesn't exist or is empty)"
    exit 1
fi


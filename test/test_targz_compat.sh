#!/bin/bash
# Test that our .tar.gz archives can be extracted by standard tools

set -e  # Exit on error

if [ ! -f test_targz ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

echo "Compatibility test for .tar.gz" > test_targz_compat.txt
echo "This archive should be readable by standard tools." >> test_targz_compat.txt
echo "Testing gzip compression compatibility." >> test_targz_compat.txt

rm -f our_targz_archive.tar.gz
./test_targz -c our_targz_archive.tar.gz test_targz_compat.txt >/dev/null 2>&1

if [ ! -f our_targz_archive.tar.gz ]; then
    echo "✗ Failed to create archive!"
    exit 1
fi

rm -rf tar_extracted_targz
mkdir -p tar_extracted_targz
cd tar_extracted_targz

if ! tar xzf ../our_targz_archive.tar.gz >/dev/null 2>&1; then
    echo "✗ tar extraction failed!"
    cd ..
    exit 1
fi

cd ..

if [ -f "tar_extracted_targz/test_targz_compat.txt" ]; then
    if cmp -s test_targz_compat.txt tar_extracted_targz/test_targz_compat.txt >/dev/null 2>&1; then
        exit 0
    else
        echo "✗ File contents do not match!"
        echo "Original file:"
        cat test_targz_compat.txt
        echo ""
        echo "Extracted file:"
        cat tar_extracted_targz/test_targz_compat.txt
        exit 1
    fi
else
    echo "✗ File was not extracted!"
    echo "Files in tar_extracted_targz directory:"
    ls -la tar_extracted_targz/ 2>/dev/null || echo "  (directory doesn't exist or is empty)"
    exit 1
fi


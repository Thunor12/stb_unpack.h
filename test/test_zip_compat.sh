#!/bin/bash
# Test that our .zip archives can be extracted by standard tools

set -e  # Exit on error

if [ ! -f build/test_zip ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

echo "Compatibility test for .zip" > input/test_zip_compat.txt
echo "This archive should be readable by standard tools." >> input/test_zip_compat.txt
echo "Testing ZIP compression compatibility." >> input/test_zip_compat.txt

rm -f output/our_zip_archive.zip
(cd input && ../build/test_zip -c ../output/our_zip_archive.zip test_zip_compat.txt) >/dev/null 2>&1

if [ ! -f output/our_zip_archive.zip ]; then
    echo "✗ Failed to create archive!"
    exit 1
fi

rm -rf output/zip_extracted
mkdir -p output/zip_extracted
cd output/zip_extracted

if ! unzip -q ../our_zip_archive.zip >/dev/null 2>&1; then
    echo "✗ unzip extraction failed!"
    cd ../..
    exit 1
fi

cd ../..

if [ -f "output/zip_extracted/test_zip_compat.txt" ]; then
    if cmp -s input/test_zip_compat.txt output/zip_extracted/test_zip_compat.txt >/dev/null 2>&1; then
        exit 0
    else
        echo "✗ File contents do not match!"
        echo "Original file:"
        cat input/test_zip_compat.txt
        echo ""
        echo "Extracted file:"
        cat output/zip_extracted/test_zip_compat.txt
        exit 1
    fi
else
    echo "✗ File was not extracted!"
    echo "Files in output/zip_extracted directory:"
    ls -la output/zip_extracted/ 2>/dev/null || echo "  (directory doesn't exist or is empty)"
    exit 1
fi


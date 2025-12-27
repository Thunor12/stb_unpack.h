#!/bin/bash
# Test that our .tar.gz archives can be extracted by standard tools

set -e  # Exit on error

echo "=== Checking for test_targz executable ==="
if [ ! -f test_targz ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

echo ""
echo "=== Creating test file ==="
echo "Compatibility test for .tar.gz" > test_targz_compat.txt
echo "This archive should be readable by standard tools." >> test_targz_compat.txt
echo "Testing gzip compression compatibility." >> test_targz_compat.txt

echo ""
echo "=== Creating .tar.gz archive with our function ==="
rm -f our_targz_archive.tar.gz
./test_targz -c our_targz_archive.tar.gz test_targz_compat.txt

if [ ! -f our_targz_archive.tar.gz ]; then
    echo "✗ Failed to create archive!"
    exit 1
fi

echo "Archive created: our_targz_archive.tar.gz"
echo "Archive size: $(stat -c%s our_targz_archive.tar.gz 2>/dev/null || stat -f%z our_targz_archive.tar.gz 2>/dev/null) bytes"

echo ""
echo "=== Extracting with standard tar tool ==="
rm -rf tar_extracted_targz
mkdir -p tar_extracted_targz
cd tar_extracted_targz

if tar xzf ../our_targz_archive.tar.gz; then
    echo "✓ tar extraction succeeded!"
else
    echo "✗ tar extraction failed!"
    cd ..
    exit 1
fi

cd ..

echo ""
echo "=== Verifying extracted file ==="
if [ -f "tar_extracted_targz/test_targz_compat.txt" ]; then
    echo "✓ File extracted successfully!"
    echo ""
    
    # Compare with original
    if cmp -s test_targz_compat.txt tar_extracted_targz/test_targz_compat.txt; then
        echo "✓ File contents match original perfectly!"
        echo ""
        echo "=== Test PASSED ==="
        echo "Our .tar.gz archives are compatible with standard tar/gzip tools!"
        exit 0
    else
        echo "✗ File contents do not match!"
        echo "=== Test FAILED ==="
        exit 1
    fi
else
    echo "✗ File was not extracted!"
    echo "Files in tar_extracted_targz directory:"
    ls -la tar_extracted_targz/ 2>/dev/null || echo "  (directory doesn't exist or is empty)"
    echo "=== Test FAILED ==="
    exit 1
fi


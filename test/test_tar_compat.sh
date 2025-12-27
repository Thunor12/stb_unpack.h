#!/bin/bash
# Test that our TAR archives can be extracted by the standard tar tool

set -e  # Exit on error

echo "=== Checking for test_create executable ==="
if [ ! -f test_create ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

echo ""
echo "=== Creating test file ==="
echo "Hello, this is a test file!" > test_compat_input.txt
echo "It has multiple lines." >> test_compat_input.txt
echo "And some content for testing." >> test_compat_input.txt
echo "The quick brown fox jumps over the lazy dog." >> test_compat_input.txt

echo ""
echo "=== Creating TAR archive with our function ==="
rm -f our_compat_archive.tar
./test_create our_compat_archive.tar test_compat_input.txt

if [ ! -f our_compat_archive.tar ]; then
    echo "✗ Failed to create archive!"
    exit 1
fi

echo "Archive created: our_compat_archive.tar"
echo "Archive size: $(stat -c%s our_compat_archive.tar 2>/dev/null || stat -f%z our_compat_archive.tar 2>/dev/null) bytes"

echo ""
echo "=== Extracting with standard tar tool ==="
rm -rf tar_extracted
mkdir -p tar_extracted
cd tar_extracted

if tar xf ../our_compat_archive.tar; then
    echo "✓ tar extraction succeeded!"
else
    echo "✗ tar extraction failed!"
    cd ..
    exit 1
fi

cd ..

echo ""
echo "=== Verifying extracted file ==="
if [ -f "tar_extracted/test_compat_input.txt" ]; then
    echo "✓ File extracted successfully!"
    echo ""
    echo "Original file:"
    cat test_compat_input.txt
    echo ""
    echo "Extracted file:"
    cat tar_extracted/test_compat_input.txt
    echo ""
    
    # Compare with original
    if cmp -s test_compat_input.txt tar_extracted/test_compat_input.txt; then
        echo "✓ File contents match original perfectly!"
        echo ""
        echo "=== Test PASSED ==="
        echo "Our TAR archives are compatible with standard tar tool!"
        exit 0
    else
        echo "✗ File contents do not match!"
        echo "=== Test FAILED ==="
        exit 1
    fi
else
    echo "✗ File was not extracted!"
    echo "Files in tar_extracted directory:"
    ls -la tar_extracted/ 2>/dev/null || echo "  (directory doesn't exist or is empty)"
    echo "=== Test FAILED ==="
    exit 1
fi


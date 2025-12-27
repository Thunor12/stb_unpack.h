#!/bin/bash
# Comprehensive test suite for .zip functionality
# Tests directories, multiple files, nested structures, edge cases

if [ ! -f build/test_zip ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

TEST_DIR="output/zip_comprehensive"
rm -rf "$TEST_DIR"
mkdir -p "$TEST_DIR"

FAILED=0
PASSED=0

test_extract() {
    local name="$1"
    local archive="$2"
    local expected_dir="$3"
    
    echo "Testing: $name"
    
    rm -rf "${TEST_DIR}/out"
    mkdir -p "${TEST_DIR}/out"
    
    if ./build/test_zip "$archive" "${TEST_DIR}/out" >/dev/null 2>&1; then
        # Compare extracted files with expected
        local match=1
        if [ -d "$expected_dir" ]; then
            # Use diff to compare directories
            if diff -r "$expected_dir" "${TEST_DIR}/out" >/dev/null 2>&1; then
                echo "  ✓ PASSED"
                PASSED=$((PASSED + 1))
                return 0
            else
                echo "  ✗ FAILED: Directory contents don't match"
                FAILED=$((FAILED + 1))
                return 1
            fi
        else
            echo "  ✗ FAILED: Expected directory not found"
            FAILED=$((FAILED + 1))
            return 1
        fi
    else
        echo "  ✗ FAILED: Extraction failed"
        FAILED=$((FAILED + 1))
        return 1
    fi
}

# Test 1: Single file
echo "=== Test 1: Single file ==="
mkdir -p "input/zip_test1"
echo "Hello, World!" > "input/zip_test1/single.txt"
(cd input/zip_test1 && zip -q ../../${TEST_DIR}/single.zip single.txt) >/dev/null 2>&1
test_extract "Single file" "${TEST_DIR}/single.zip" "input/zip_test1"

# Test 2: Multiple files in root
echo "=== Test 2: Multiple files in root ==="
mkdir -p "input/zip_test2"
echo "File 1 content" > "input/zip_test2/file1.txt"
echo "File 2 content" > "input/zip_test2/file2.txt"
echo "File 3 content" > "input/zip_test2/file3.txt"
(cd input/zip_test2 && zip -q ../../${TEST_DIR}/multiple.zip file1.txt file2.txt file3.txt) >/dev/null 2>&1
test_extract "Multiple files" "${TEST_DIR}/multiple.zip" "input/zip_test2"

# Test 3: Directory with files
echo "=== Test 3: Directory with files ==="
mkdir -p "input/zip_test3/mydir"
echo "File in subdirectory" > "input/zip_test3/mydir/subfile.txt"
echo "Root file" > "input/zip_test3/root.txt"
(cd input/zip_test3 && zip -qr ../../${TEST_DIR}/directory.zip .) >/dev/null 2>&1
test_extract "Directory with files" "${TEST_DIR}/directory.zip" "input/zip_test3"

# Test 4: Nested directories
echo "=== Test 4: Nested directories ==="
mkdir -p "input/zip_test4/level1/level2/level3"
echo "Deep file" > "input/zip_test4/level1/level2/level3/deep.txt"
echo "Level 1 file" > "input/zip_test4/level1/file1.txt"
echo "Level 2 file" > "input/zip_test4/level1/level2/file2.txt"
echo "Root file" > "input/zip_test4/root.txt"
(cd input/zip_test4 && zip -qr ../../${TEST_DIR}/nested.zip .) >/dev/null 2>&1
test_extract "Nested directories" "${TEST_DIR}/nested.zip" "input/zip_test4"

# Test 5: Mixed files and directories
echo "=== Test 5: Mixed files and directories ==="
mkdir -p "input/zip_test5/dir1" "input/zip_test5/dir2/subdir"
echo "File 1" > "input/zip_test5/file1.txt"
echo "File 2" > "input/zip_test5/file2.txt"
echo "Dir1 file" > "input/zip_test5/dir1/dir1file.txt"
echo "Subdir file" > "input/zip_test5/dir2/subdir/subfile.txt"
echo "Dir2 file" > "input/zip_test5/dir2/dir2file.txt"
(cd input/zip_test5 && zip -qr ../../${TEST_DIR}/mixed.zip .) >/dev/null 2>&1
test_extract "Mixed files and directories" "${TEST_DIR}/mixed.zip" "input/zip_test5"

# Test 6: Empty directory
echo "=== Test 6: Empty directory ==="
mkdir -p "input/zip_test6/emptydir"
echo "Root file" > "input/zip_test6/root.txt"
(cd input/zip_test6 && zip -qr ../../${TEST_DIR}/emptydir.zip .) >/dev/null 2>&1
test_extract "Empty directory" "${TEST_DIR}/emptydir.zip" "input/zip_test6"

# Test 7: Binary files
echo "=== Test 7: Binary files ==="
mkdir -p "${TEST_DIR}/temp/zip_test7"
head -c 1024 /dev/urandom > "${TEST_DIR}/temp/zip_test7/binary1.bin" 2>/dev/null || \
    python3 -c "import os; open('${TEST_DIR}/temp/zip_test7/binary1.bin', 'wb').write(os.urandom(1024))" 2>/dev/null || \
    (dd if=/dev/zero of="${TEST_DIR}/temp/zip_test7/binary1.bin" bs=1024 count=1 2>/dev/null || echo -ne '\x00\x01\x02\x03' > "${TEST_DIR}/temp/zip_test7/binary1.bin")
head -c 512 /dev/urandom > "${TEST_DIR}/temp/zip_test7/binary2.bin" 2>/dev/null || \
    python3 -c "import os; open('${TEST_DIR}/temp/zip_test7/binary2.bin', 'wb').write(os.urandom(512))" 2>/dev/null || \
    (dd if=/dev/zero of="${TEST_DIR}/temp/zip_test7/binary2.bin" bs=512 count=1 2>/dev/null || echo -ne '\x04\x05\x06\x07' > "${TEST_DIR}/temp/zip_test7/binary2.bin")
echo "Text file" > "${TEST_DIR}/temp/zip_test7/text.txt"
(cd "${TEST_DIR}/temp/zip_test7" && zip -q ../../../../${TEST_DIR}/binary.zip binary1.bin binary2.bin text.txt) >/dev/null 2>&1
test_extract "Binary files" "${TEST_DIR}/binary.zip" "${TEST_DIR}/temp/zip_test7"

# Test 8: Large number of files
echo "=== Test 8: Large number of files ==="
mkdir -p "input/zip_test8"
for i in {1..20}; do
    echo "Content of file $i" > "input/zip_test8/file${i}.txt"
done
(cd input/zip_test8 && zip -q ../../${TEST_DIR}/manyfiles.zip *.txt) >/dev/null 2>&1
test_extract "Large number of files" "${TEST_DIR}/manyfiles.zip" "input/zip_test8"

# Test 9: Files with special characters in names
echo "=== Test 9: Special characters in filenames ==="
mkdir -p "input/zip_test9"
echo "File with dashes" > "input/zip_test9/file-with-dashes.txt"
echo "File with underscores" > "input/zip_test9/file_with_underscores.txt"
echo "File with dots" > "input/zip_test9/file.with.dots.txt"
(cd input/zip_test9 && zip -q ../../${TEST_DIR}/special.zip *.txt) >/dev/null 2>&1
test_extract "Special characters" "${TEST_DIR}/special.zip" "input/zip_test9"

# Test 10: Empty file
echo "=== Test 10: Empty file ==="
mkdir -p "input/zip_test10"
echo -n "" > "input/zip_test10/empty.txt"
echo "Non-empty file" > "input/zip_test10/nonempty.txt"
(cd input/zip_test10 && zip -q ../../${TEST_DIR}/empty.zip *.txt) >/dev/null 2>&1
test_extract "Empty file" "${TEST_DIR}/empty.zip" "input/zip_test10"

# Summary
echo ""
echo "=== Test Summary ==="
echo "Passed: $PASSED"
echo "Failed: $FAILED"

if [ $FAILED -eq 0 ]; then
    echo ""
    echo "✓ All comprehensive ZIP tests passed!"
    exit 0
else
    echo ""
    echo "✗ Some tests failed!"
    exit 1
fi


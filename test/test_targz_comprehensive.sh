#!/bin/bash
# Comprehensive test suite for .tar.gz functionality
# Tests various file sizes, types, directories, edge cases

if [ ! -f build/test_targz ]; then
    echo "✗ Test executable not found!"
    echo "Please build the test programs first using: make"
    exit 1
fi

TEST_DIR="output/comprehensive"
rm -rf "$TEST_DIR"
mkdir -p "$TEST_DIR"

FAILED=0
PASSED=0

test_case() {
    local name="$1"
    local archive="$2"
    local expected_file="$3"
    
    echo "Testing: $name"
    
    rm -rf "${TEST_DIR}/out"
    mkdir -p "${TEST_DIR}/out"
    
    if ./build/test_targz "$archive" "${TEST_DIR}/out" >/dev/null 2>&1; then
        if [ -f "${TEST_DIR}/out/$expected_file" ]; then
            if cmp -s "input/comprehensive/$expected_file" "${TEST_DIR}/out/$expected_file" >/dev/null 2>&1; then
                echo "  ✓ PASSED"
                PASSED=$((PASSED + 1))
                return 0
            else
                echo "  ✗ FAILED: File contents don't match"
                FAILED=$((FAILED + 1))
                return 1
            fi
        else
            echo "  ✗ FAILED: File not extracted"
            FAILED=$((FAILED + 1))
            return 1
        fi
    else
        echo "  ✗ FAILED: Extraction failed"
        FAILED=$((FAILED + 1))
        return 1
    fi
}

# Test 1: Empty file
echo "=== Test 1: Empty file ==="
echo -n "" > "input/comprehensive/empty.txt"
(cd input/comprehensive && ../../build/test_targz -c "../../${TEST_DIR}/empty.tar.gz" empty.txt) >/dev/null 2>&1
test_case "Empty file" "${TEST_DIR}/empty.tar.gz" "empty.txt"

# Test 2: Small text file
echo "=== Test 2: Small text file ==="
echo "Hello, World!" > "input/comprehensive/small.txt"
(cd input/comprehensive && ../../build/test_targz -c "../../${TEST_DIR}/small.tar.gz" small.txt) >/dev/null 2>&1
test_case "Small text file" "${TEST_DIR}/small.tar.gz" "small.txt"

# Test 3: Large text file (multiple blocks)
echo "=== Test 3: Large text file ==="
head -c 10000 /dev/urandom | base64 > "input/comprehensive/large.txt" 2>/dev/null || \
    python3 -c "import random, string; print(''.join(random.choices(string.ascii_letters, k=10000)))" > "input/comprehensive/large.txt" 2>/dev/null || \
    (for i in {1..1000}; do echo "This is line $i with some content to make the file larger."; done > "input/comprehensive/large.txt")
(cd input/comprehensive && ../../build/test_targz -c "../../${TEST_DIR}/large.tar.gz" large.txt) >/dev/null 2>&1
test_case "Large text file" "${TEST_DIR}/large.tar.gz" "large.txt"

# Test 4: Binary file
echo "=== Test 4: Binary file ==="
head -c 512 /dev/urandom > "input/comprehensive/binary.bin" 2>/dev/null || \
    python3 -c "import os; open('input/comprehensive/binary.bin', 'wb').write(os.urandom(512))" 2>/dev/null || \
    (dd if=/dev/zero of="input/comprehensive/binary.bin" bs=512 count=1 2>/dev/null || echo -ne '\x00\x01\x02\x03' > "input/comprehensive/binary.bin")
(cd input/comprehensive && ../../build/test_targz -c "../../${TEST_DIR}/binary.tar.gz" binary.bin) >/dev/null 2>&1
test_case "Binary file" "${TEST_DIR}/binary.tar.gz" "binary.bin"

# Test 5: File with special characters in name
echo "=== Test 5: Special characters in filename ==="
echo "Content with special chars" > "input/comprehensive/file_with-dashes.txt"
(cd input/comprehensive && ../../build/test_targz -c "../../${TEST_DIR}/special.tar.gz" file_with-dashes.txt) >/dev/null 2>&1
test_case "Special characters" "${TEST_DIR}/special.tar.gz" "file_with-dashes.txt"

# Test 6: Very long filename (TAR format limits to 100 chars, so it will be truncated)
echo "=== Test 6: Long filename ==="
LONG_NAME="very_long_filename_$(printf 'a%.0s' {1..80}).txt"
echo "Long filename test" > "input/comprehensive/${LONG_NAME}"
(cd input/comprehensive && ../../build/test_targz -c "../../${TEST_DIR}/long.tar.gz" "${LONG_NAME}") >/dev/null 2>&1

rm -rf "${TEST_DIR}/out"
mkdir -p "${TEST_DIR}/out"

if ./build/test_targz "${TEST_DIR}/long.tar.gz" "${TEST_DIR}/out" >/dev/null 2>&1; then
    # Find the actual extracted filename (might be truncated)
    EXTRACTED_NAME=$(ls "${TEST_DIR}/out/" 2>/dev/null | head -1)
    if [ -n "$EXTRACTED_NAME" ] && [ -f "${TEST_DIR}/out/${EXTRACTED_NAME}" ]; then
        if cmp -s "input/comprehensive/${LONG_NAME}" "${TEST_DIR}/out/${EXTRACTED_NAME}" >/dev/null 2>&1; then
            echo "  ✓ PASSED (filename: $EXTRACTED_NAME)"
            PASSED=$((PASSED + 1))
        else
            echo "  ✗ FAILED: File contents don't match"
            FAILED=$((FAILED + 1))
        fi
    else
        echo "  ✗ FAILED: No file extracted"
        FAILED=$((FAILED + 1))
    fi
else
    echo "  ✗ FAILED: Extraction failed"
    FAILED=$((FAILED + 1))
fi

# Test 7: File exactly 512 bytes (one TAR block)
echo "=== Test 7: File exactly 512 bytes ==="
head -c 512 /dev/urandom > "input/comprehensive/exact512.bin" 2>/dev/null || \
    python3 -c "import os; open('input/comprehensive/exact512.bin', 'wb').write(os.urandom(512))" 2>/dev/null || \
    (dd if=/dev/zero of="input/comprehensive/exact512.bin" bs=512 count=1 2>/dev/null || printf '%*s' 512 | tr ' ' '\x00' > "input/comprehensive/exact512.bin")
(cd input/comprehensive && ../../build/test_targz -c "../../${TEST_DIR}/exact512.tar.gz" exact512.bin) >/dev/null 2>&1
test_case "Exactly 512 bytes" "${TEST_DIR}/exact512.tar.gz" "exact512.bin"

# Test 8: File with newlines and special content
echo "=== Test 8: Multi-line content ==="
cat > "input/comprehensive/multiline.txt" << 'EOF'
Line 1
Line 2 with special chars: !@#$%^&*()
Line 3 with unicode: café résumé
Line 4 with tabs:	tab	here
Line 5: End
EOF
(cd input/comprehensive && ../../build/test_targz -c "../../${TEST_DIR}/multiline.tar.gz" multiline.txt) >/dev/null 2>&1
test_case "Multi-line content" "${TEST_DIR}/multiline.tar.gz" "multiline.txt"

# Summary
echo ""
echo "=== Test Summary ==="
echo "Passed: $PASSED"
echo "Failed: $FAILED"

if [ $FAILED -eq 0 ]; then
    echo ""
    echo "✓ All comprehensive tests passed!"
    exit 0
else
    echo ""
    echo "✗ Some tests failed!"
    exit 1
fi


#!/bin/bash
# Run all tests for stb_unpack.h

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=========================================="
echo "Running all tests for stb_unpack.h"
echo "=========================================="
echo ""

# Track test results
PASSED=0
FAILED=0

run_test() {
    local test_name="$1"
    local test_script="$2"
    
    echo "----------------------------------------"
    echo "Running: $test_name"
    echo "----------------------------------------"
    
    if bash "$test_script"; then
        echo ""
        echo "✓ $test_name: PASSED"
        ((PASSED++))
        return 0
    else
        echo ""
        echo "✗ $test_name: FAILED"
        ((FAILED++))
        return 1
    fi
}

# Run tests
run_test "TAR Extraction Test" "./test_extract.sh"
echo ""

run_test "TAR Creation Test" "./test_create.sh"
echo ""

run_test "TAR Compatibility Test" "./test_tar_compat.sh"
echo ""

# Summary
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo "Passed: $PASSED"
echo "Failed: $FAILED"
echo ""

if [ $FAILED -eq 0 ]; then
    echo "✓ All tests passed!"
    exit 0
else
    echo "✗ Some tests failed!"
    exit 1
fi


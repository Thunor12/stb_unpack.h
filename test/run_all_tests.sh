#!/bin/bash
# Run all tests for stb_unpack.h

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Track test results
PASSED=0
FAILED=0

run_test() {
    local test_name="$1"
    local test_script="$2"
    local output
    local exit_code
    
    # Capture output and exit code
    output=$(bash "$test_script" 2>&1)
    exit_code=$?
    
    if [ $exit_code -eq 0 ]; then
        echo "✓ $test_name: PASSED"
        PASSED=$((PASSED + 1))
        return 0
    else
        echo "✗ $test_name: FAILED"
        echo ""
        echo "$output"
        echo ""
        FAILED=$((FAILED + 1))
        return 1
    fi
}

# Run tests
run_test "TAR Extraction Test" "./test_extract.sh"
run_test "TAR Creation Test" "./test_create.sh"
run_test "TAR Compatibility Test" "./test_tar_compat.sh"

# Check if miniz is available for .tar.gz tests
if [ -f "../miniz.c" ] && [ -f "../miniz.h" ]; then
    run_test ".tar.gz Test" "./test_targz.sh"
    run_test ".tar.gz Compatibility Test" "./test_targz_compat.sh"
fi

# Summary
if [ $FAILED -eq 0 ]; then
    echo ""
    echo "✓ All tests passed! ($PASSED/$((PASSED + FAILED)))"
    exit 0
else
    echo ""
    echo "✗ Some tests failed! ($PASSED passed, $FAILED failed)"
    exit 1
fi


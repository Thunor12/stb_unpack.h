# Test Suite

This directory contains all test files and test-related scripts for `stb_unpack.h`.

## Test Files

- `test.c` - Basic extraction test program
- `test_create.c` - TAR creation test program
- `test_input.txt` - Small test file for basic tests
- `test_file.txt` - Larger test file (10MB)
- `test_compat_input.txt` - Test file for compatibility tests

## Test Scripts

- `test_extract.sh` - Tests TAR extraction functionality
- `test_create.sh` - Tests TAR creation and compares with `tar` command
- `test_tar_compat.sh` - Tests that our TAR archives can be extracted by standard `tar` tool
- `run_all_tests.sh` - Runs all tests and provides a summary

## Running Tests

**From repository root (recommended):**
```bash
make test          # Run all tests
make test-extract  # Run extraction test only
make test-create   # Run creation test only
make test-compat   # Run compatibility test only
```

**From test directory:**
```bash
cd test
./run_all_tests.sh
```

Or run individual tests:
```bash
cd test
./test_extract.sh
./test_create.sh
./test_tar_compat.sh
```

## Test Outputs

Test outputs (archives, extracted files, executables) are created in the `test/` directory and can be safely deleted. They are automatically ignored by git (see `.gitignore`).

To clean up:
```bash
make clean
# or
cd test && rm -f *.tar test test_create out/* tar_extracted/*
```

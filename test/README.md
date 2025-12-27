# Test Suite

This directory contains the comprehensive test suite for `stb_unpack.h`.

## Test Structure

```
test/
├── src/              # Test source files (C programs)
│   ├── test.c       # Basic TAR extraction test
│   ├── test_create.c # TAR creation test
│   ├── test_targz.c # .tar.gz creation and extraction test
│   ├── test_zip.c   # ZIP creation and extraction test
│   └── test_runner.c # Main test runner (executes all tests)
├── input/           # Test input files (static test data)
├── build/           # Compiled test executables (gitignored)
└── output/          # Test outputs - archives and extracted files (gitignored)
```

## Running Tests

**From repository root (recommended):**
```bash
./nob test    # Build and run all tests
./nob build   # Build test programs only
```

**From test directory:**
```bash
cd test
../nob test   # Run tests
```

## Test Programs

### test.c
Basic TAR extraction test. Creates a TAR archive using the system `tar` command, then extracts it using `stbup_tar_extract_stream`.

### test_create.c
TAR creation test. Creates a TAR archive using `stbup_tar_create_file`, then verifies it can be extracted by the standard `tar` tool.

### test_targz.c
.tar.gz creation and extraction test. Supports two modes:
- **Create mode**: `./test_targz -c archive.tar.gz file.txt`
- **Extract mode**: `./test_targz archive.tar.gz output_dir`

### test_zip.c
ZIP creation and extraction test. Supports two modes:
- **Create mode**: `./test_zip -c archive.zip file.txt`
- **Extract mode**: `./test_zip archive.zip output_dir`

### test_runner.c
Main test runner that executes all tests. This is a C program (not a shell script) for cross-platform compatibility. It:
- Detects if miniz is available (embedded or external)
- Checks for external tools (tar, unzip, zip) and skips tests if unavailable
- Runs all 9 test suites
- Reports pass/fail/skip status for each test
- Provides a summary at the end

## Test Coverage

The test suite includes:

1. **TAR Extraction Test** - Verifies basic TAR extraction
2. **TAR Creation Test** - Verifies TAR creation and compatibility
3. **TAR Compatibility Test** - Ensures our TARs work with standard tools
4. **.tar.gz Test** - Basic .tar.gz creation and extraction
5. **.tar.gz Compatibility Test** - Ensures our .tar.gz files work with standard tools
6. **.tar.gz Comprehensive Test** - Additional edge cases
7. **.zip Test** - Basic ZIP creation and extraction
8. **.zip Compatibility Test** - Ensures our ZIPs work with standard tools
9. **.zip Comprehensive Test** - Additional edge cases

## Test Behavior

- **Passed tests**: Indicate the feature works correctly
- **Failed tests**: Indicate a bug or regression (CI will fail)
- **Skipped tests**: Indicate required external tools are missing (warning only, CI won't fail)

Skipped tests are expected on systems without `tar`, `unzip`, or `zip` commands. The core functionality is still tested even when these tools are unavailable.

## Cleaning Test Outputs

```bash
./nob clean    # Removes test/build/ and test/output/ directories
```

Or manually:
```bash
rm -rf test/build test/output
```

## Adding New Tests

To add a new test:

1. Create a test program in `test/src/` (e.g., `test_new_feature.c`)
2. Add a test function to `test/src/test_runner.c`
3. Add the test to the `tests` array in `test_runner.c`
4. Update `nob.c` to build the new test executable
5. Run `./nob test` to verify

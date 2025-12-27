# stb_unpack.h

A single-header, dependency-free C library for extracting and creating archive formats (TAR, .tar.gz, ZIP).

## Features

- ✅ **Single header file** - Just `#define STB_UNPACK_IMPLEMENTATION` and include
- ✅ **Zero dependencies** - Compression via embedded miniz library
- ✅ **Cross-platform** - Windows, Linux, macOS
- ✅ **Multiple formats** - TAR, .tar.gz, and ZIP support
- ✅ **Extract & Create** - Both extraction and archive creation
- ✅ **Portable** - Works with any C99 compiler

## Quick Start

### Using the Library

```c
#define STB_UNPACK_IMPLEMENTATION
#include "stb_unpack.h"

int main(void) {
    // Extract a .tar.gz archive
    if (!stbup_targz_extract("archive.tar.gz", "output_dir")) {
        printf("Extraction failed!\n");
        return 1;
    }
    
    // Create a TAR archive
    if (!stbup_tar_create_file("archive.tar", "file.txt")) {
        printf("Archive creation failed!\n");
        return 1;
    }
    
    return 0;
}
```

### Building and Testing

The project uses `nob.c`, a single-file build system (similar to `make`):

```bash
# Build and run all tests (default)
./nob

# Build test programs only
./nob build

# Run tests only (assumes programs are built)
./nob test

# Build example program
./nob example

# Clean build artifacts and test outputs
./nob clean

# Show help
./nob help
```

**On Windows:**
```powershell
# Compile nob first
gcc nob.c -o nob.exe

# Then use it
./nob.exe build
./nob.exe test
```

## API Reference

### TAR Archives

```c
// Extract a TAR archive from memory
int stbup_tar_extract_stream(const void *tar_data, size_t tar_size, const char *out_dir);

// Create a TAR archive from a single file
int stbup_tar_create_file(const char *archive_path, const char *file_path);
```

### .tar.gz Archives

```c
// Extract a .tar.gz archive
int stbup_targz_extract(const char *archive_path, const char *out_dir);

// Create a .tar.gz archive from a single file
int stbup_targz_create_file(const char *archive_path, const char *file_path);
```

### ZIP Archives

```c
// Extract a ZIP archive
int stbup_zip_extract(const char *archive_path, const char *out_dir);

// Create a ZIP archive from a single file
int stbup_zip_create_file(const char *archive_path, const char *file_path);
```

All functions return `1` on success, `0` on failure.

## Project Structure

```
stb_unpack.h/
├── stb_unpack.h          # Main library (single header)
├── nob.c                 # Build system
├── nob.h                 # Build system header
├── test/                 # Test suite
│   ├── src/              # Test source files
│   │   ├── test.c        # Basic TAR extraction test
│   │   ├── test_create.c # TAR creation test
│   │   ├── test_targz.c  # .tar.gz test
│   │   ├── test_zip.c    # ZIP test
│   │   └── test_runner.c # Main test runner (runs all tests)
│   ├── input/            # Test input files
│   ├── build/            # Compiled test executables (gitignored)
│   └── output/           # Test outputs (gitignored)
├── example/              # Example programs
│   ├── extract_src.c    # Example: extract .tar.gz archive
│   └── README.md        # Example documentation
├── scripts/              # Utility scripts
│   ├── fuse_miniz.sh    # Embed miniz into stb_unpack.h
│   └── README.md        # Script documentation
└── .github/workflows/    # CI/CD pipeline
    └── ci.yml           # GitHub Actions workflow
```

## Testing

The test suite is comprehensive and runs automatically via `./nob test`:

- **TAR Extraction Test** - Verifies TAR extraction works correctly
- **TAR Creation Test** - Verifies TAR creation and compares with standard `tar`
- **TAR Compatibility Test** - Ensures our TARs can be read by standard tools
- **.tar.gz Test** - Tests .tar.gz creation and extraction
- **.tar.gz Compatibility Test** - Ensures our .tar.gz files work with standard tools
- **.tar.gz Comprehensive Test** - Additional edge cases for .tar.gz
- **.zip Test** - Tests ZIP creation and extraction
- **.zip Compatibility Test** - Ensures our ZIPs can be read by standard tools
- **.zip Comprehensive Test** - Additional edge cases for ZIP

Tests that require external tools (tar, unzip, zip) will be skipped with a warning if those tools are not available. This allows the test suite to run on systems without these tools while still verifying functionality when they are present.

## Continuous Integration

The project includes a GitHub Actions CI pipeline (`.github/workflows/ci.yml`) that:

- Runs on both Linux and Windows
- Builds the `nob` build system
- Compiles all test programs
- Runs the full test suite
- Fails on compilation errors or test failures
- Warns (but doesn't fail) on skipped tests

The pipeline runs automatically on:
- Pushes to `main` or `master` branches
- Pull requests to `main` or `master` branches

## Updating miniz

The library embeds miniz for compression support. To update to a new miniz version:

```bash
# Download new miniz release (e.g., miniz-3.2.0.tar.gz)
./scripts/fuse_miniz.sh miniz-3.2.0.tar.gz

# Test the update
./nob clean
./nob test

# Review changes
git diff stb_unpack.h

# Commit
git commit -m "Update miniz to 3.2.0"
```

See `scripts/README.md` for more details.

## Design Philosophy

**stb_unpack.h** follows the stb-style library approach:

- **Single header** - One file, no separate compilation units
- **Dependency-free** - Everything embedded, no external libraries
- **Portable** - Works on Windows, Linux, macOS with any C99 compiler
- **Simple API** - Easy to use, hard to misuse
- **Good enough** - Focuses on common use cases, not edge cases

## Limitations

This library is designed for common use cases and intentionally does **not** support:

- ❌ Multi-file TAR archives (single file only)
- ❌ Symlinks
- ❌ Permission/ownership preservation
- ❌ Encrypted ZIP files
- ❌ Streaming APIs (extract-to-disk only)
- ❌ Exotic formats (rar, 7z, etc.)

For full-featured archive support, use `libarchive` or similar libraries.

## License

See `LICENSE` file for details.

## Contributing

1. Make your changes
2. Run `./nob test` to ensure all tests pass
3. Submit a pull request

The CI pipeline will automatically verify your changes on both Linux and Windows.

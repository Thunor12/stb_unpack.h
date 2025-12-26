# stb_unpack.h
an stb C lib to extract archives

## Building and Running Tests

All tests are located in the `test/` directory.

### Quick Start (Recommended)

**Using Make:**
```bash
make              # Build test programs
make test         # Build and run all tests
make test-extract # Build and run extraction test only
make test-create  # Build and run creation test only
make test-compat  # Build and run compatibility test only
make clean        # Clean build artifacts
make help         # Show all available targets
```

**Using build scripts (wrappers around make):**
```bash
./build.sh        # Linux/Unix - builds test programs
build.bat         # Windows - builds test programs
```

### Running Tests Directly

If you prefer to run test scripts directly:
```bash
cd test
chmod +x *.sh
./run_all_tests.sh        # Run all tests
./test_extract.sh         # Extraction test only
./test_create.sh          # Creation test only
./test_tar_compat.sh      # Compatibility test only
```

### Manual Compilation

If you don't have `make`:
```bash
cd test
gcc test.c -o test -std=c99 -Wall -I..
gcc test_create.c -o test_create -std=c99 -Wall -I..
```

# What is stb_unpack.h

**stb_unpack.h** is a small, self-contained, stb-style C header library for **extracting simple archive formats to disk**, with **no external dependencies**.

It is designed to be:
- **Single-header**
- **Portable** (Windows / Linux / macOS)
- **nob-friendly**
- **Allocator-agnostic**
- **Easy to vendor, audit, and hack**

The goal is not to be a full-featured archive tool, but a **reliable, minimal building block** for bootstrapping, build systems, tools, and games.

---

## Project Status

🚧 **Work in progress**

Current implementation includes:
- ✅ Arena allocator
- ✅ Portable filesystem abstraction
- ✅ Streaming TAR parser/extractor
- ✅ TAR archive creator

Planned features are outlined below.

---

## Motivation

Many small C projects need to:
- download dependencies
- unpack assets
- bootstrap toolchains
- extract vendor libraries

…but existing solutions often:
- depend on large system libraries (`libarchive`, `zlib`, `libcurl`)
- require platform-specific build steps
- are hard to vendor cleanly

**stb_unpack.h** aims to fill that gap with a pragmatic, stb-inspired approach:
> *Small, focused, dependency-free, and good enough.*

---

## Design Goals

- **Single header**
  - `#define STB_UNPACK_IMPLEMENTATION`
- **No external dependencies**
  - Compression handled via embedded code (copied code from miniz)
- **Extract-to-disk only**
  - No in-memory archive APIs
- **Portable filesystem layer**
  - Abstracted `mkdir`, file writing, path handling
- **Arena-based memory management**
  - Works with static buffers, stack memory, or heap
- **Safe by default**
  - Prevents path traversal (`../`, absolute paths)
- **Format auto-detection**
  - No reliance on file extensions

---

## Planned Feature Set (v1.0)

When finished, **stb_unpack.h** will support:

### Archive formats
- ✅ `.tar` (extract & create)
- ⏳ `.tar.gz` (planned)
- ⏳ `.zip` (planned, non-encrypted)

### Extraction
- ✅ Extract archives **to disk**
- ✅ Recursive directory creation
- ✅ Optional overwrite control
- ❌ No symlinks (initially)
- ❌ No permission/ownership preservation

### Compression
- gzip / deflate (via embedded copied code from miniz)
- zip decompression (via embedded copied code from miniz)

### Memory
- Arena allocator (user-provided or internal)
- No mandatory dynamic allocation
- Custom allocators supported via macros

### Portability
- Windows
- Linux
- macOS
- No reliance on POSIX-only APIs

---

## Non-Goals

This library intentionally does **not** aim to be:
- a replacement for `tar`, `unzip`, or `libarchive`
- a streaming archive API
- a secure archive validator
- a tool for exotic formats (`rar`, `7z`, encrypted zip)
- a metadata-preserving extractor

If you need those features, use a full archive library.

---

## Example Usage (planned)

```c
#define STB_UNPACK_IMPLEMENTATION
#include "stb_unpack.h"

int main(void)
{
    stbup_options opt = {
        .out_dir = "deps",
        .overwrite = 0
    };

    if (!stbup_extract("raylib.tar.gz", &opt)) {
        printf("error: %s\n", stbup_last_error());
        return 1;
    }
    return 0;
}
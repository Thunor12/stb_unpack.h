# stb_unpack.h
an stb C lib to extract archives

## Building the test

### Windows
Run the build script:
```cmd
build.bat
```

Or manually compile with:
```cmd
gcc test.c -o test.exe -std=c99
```

### Linux/Unix
Run the build script:
```bash
chmod +x build.sh
./build.sh
```

Or manually compile with:
```bash
gcc test.c -o test -std=c99
```

## Running the test

Make sure you have an `archive.tar` file in the current directory, then run:
- Windows: `test.exe`
- Linux: `./test`

The test will extract the archive to the `out` directory.

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
- Arena allocator
- Portable filesystem abstraction
- Streaming TAR parser

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
- ✅ `.tar`
- ✅ `.tar.gz`
- ✅ `.zip` (non-encrypted)

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

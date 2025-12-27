# Scripts

## fuse_miniz.sh

Fuses miniz directly into `stb_unpack.h` to create a truly single-header, dependency-free library.

### Usage

```bash
# Use miniz.h and miniz.c from the repository root
./scripts/fuse_miniz.sh

# Use miniz from a directory
./scripts/fuse_miniz.sh /path/to/miniz/directory

# Use miniz from a release archive (.tar.gz or .zip)
./scripts/fuse_miniz.sh miniz-3.1.0.tar.gz
./scripts/fuse_miniz.sh miniz-3.1.0.zip
```

### How it works

1. Extracts `miniz.h` and `miniz.c` from the provided source (directory, archive, or current repo)
2. Patches them into `stb_unpack.h` at the marked locations:
   - `STBUP_MINIZ_HEADER_START` / `STBUP_MINIZ_HEADER_END` - for miniz.h
   - `STBUP_MINIZ_IMPL_START` / `STBUP_MINIZ_IMPL_END` - for miniz.c (inside `STB_UNPACK_IMPLEMENTATION` block)
3. The script modifies `stb_unpack.h` in place (use git to track changes)

### Updating to a new miniz version

1. Download the new miniz release (e.g., `miniz-3.2.0.tar.gz`)
2. Run: `./scripts/fuse_miniz.sh miniz-3.2.0.tar.gz`
3. Test the updated `stb_unpack.h` with: `make test`
4. Review changes with: `git diff stb_unpack.h`
5. Commit the changes: `git commit -m "Update miniz to 3.2.0"`

### Notes

- The script automatically handles include guards and implementation defines
- Original miniz files (`miniz.h`, `miniz.c`) are not modified
- This is a source update operation only - the Makefile automatically detects embedded miniz and skips separate compilation


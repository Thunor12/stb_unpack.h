# Example: Extract Source Archive

This example demonstrates how to use `stb_unpack.h` to extract a `.tar.gz` archive.

## Building

```bash
make example
```

This will create `example/extract_src` executable.

## Usage

```bash
cd example
./extract_src
```

This will extract `raylib-5.5_linux_amd64.tar.gz` to `src/` directory.

You can also specify custom archive and output directory:

```bash
./extract_src <archive.tar.gz> <output_directory>
```

## Example

```bash
# Extract the default raylib archive
./extract_src

# Extract a custom archive
./extract_src my_archive.tar.gz my_output_dir
```


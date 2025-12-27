#!/bin/bash
# Script to fuse miniz into stb_unpack.h
# Usage: ./scripts/fuse_miniz.sh [miniz_release_path]
#
# If miniz_release_path is provided, it should be:
# - A directory containing miniz.h and miniz.c
# - A .tar.gz or .zip archive containing miniz.h and miniz.c
#
# If not provided, uses miniz.h and miniz.c from the current directory

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
STB_UNPACK_H="$REPO_ROOT/stb_unpack.h"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

error() {
    echo -e "${RED}Error: $1${NC}" >&2
    exit 1
}

info() {
    echo -e "${GREEN}$1${NC}"
}

warn() {
    echo -e "${YELLOW}Warning: $1${NC}"
}

# Extract miniz files from release
extract_miniz() {
    local release_path="$1"
    local temp_dir=$(mktemp -d)
    
    if [ -z "$release_path" ]; then
        # Use miniz files from current directory
        if [ ! -f "$REPO_ROOT/miniz.h" ] || [ ! -f "$REPO_ROOT/miniz.c" ]; then
            error "miniz.h and miniz.c not found in repository root. Please provide a miniz release path."
        fi
        cp "$REPO_ROOT/miniz.h" "$temp_dir/"
        cp "$REPO_ROOT/miniz.c" "$temp_dir/"
        echo "$temp_dir"
        return
    fi
    
    if [ -d "$release_path" ]; then
        # Directory containing miniz files
        if [ ! -f "$release_path/miniz.h" ] || [ ! -f "$release_path/miniz.c" ]; then
            error "miniz.h and miniz.c not found in $release_path"
        fi
        cp "$release_path/miniz.h" "$temp_dir/"
        cp "$release_path/miniz.c" "$temp_dir/"
        echo "$temp_dir"
    elif [ -f "$release_path" ]; then
        # Archive file
        local ext="${release_path##*.}"
        if [ "$ext" = "gz" ] || [ "$ext" = "tgz" ]; then
            # .tar.gz
            tar -xzf "$release_path" -C "$temp_dir" --strip-components=1 2>/dev/null || \
            tar -xzf "$release_path" -C "$temp_dir" 2>/dev/null || \
            error "Failed to extract $release_path"
        elif [ "$ext" = "zip" ]; then
            # .zip
            unzip -q "$release_path" -d "$temp_dir" 2>/dev/null || \
            error "Failed to extract $release_path"
        else
            error "Unknown archive format: $ext"
        fi
        
        # Find miniz.h and miniz.c in extracted files
        local miniz_h=$(find "$temp_dir" -name "miniz.h" -type f | head -1)
        local miniz_c=$(find "$temp_dir" -name "miniz.c" -type f | head -1)
        
        if [ -z "$miniz_h" ] || [ -z "$miniz_c" ]; then
            error "miniz.h and miniz.c not found in extracted archive"
        fi
        
        # Copy to temp_dir root
        cp "$miniz_h" "$temp_dir/miniz.h"
        cp "$miniz_c" "$temp_dir/miniz.c"
        echo "$temp_dir"
    else
        error "Invalid release path: $release_path"
    fi
}

# Patch miniz into stb_unpack.h
patch_stb_unpack() {
    local miniz_dir="$1"
    local miniz_h="$miniz_dir/miniz.h"
    local miniz_c="$miniz_dir/miniz.c"
    
    if [ ! -f "$miniz_h" ] || [ ! -f "$miniz_c" ]; then
        error "miniz.h or miniz.c not found in $miniz_dir"
    fi
    
    # Check for markers in stb_unpack.h
    if ! grep -q "STBUP_MINIZ_HEADER_START" "$STB_UNPACK_H"; then
        error "STBUP_MINIZ_HEADER_START marker not found in stb_unpack.h"
    fi
    if ! grep -q "STBUP_MINIZ_HEADER_END" "$STB_UNPACK_H"; then
        error "STBUP_MINIZ_HEADER_END marker not found in stb_unpack.h"
    fi
    if ! grep -q "STBUP_MINIZ_IMPL_START" "$STB_UNPACK_H"; then
        error "STBUP_MINIZ_IMPL_START marker not found in stb_unpack.h"
    fi
    if ! grep -q "STBUP_MINIZ_IMPL_END" "$STB_UNPACK_H"; then
        error "STBUP_MINIZ_IMPL_END marker not found in stb_unpack.h"
    fi
    
    # Create temporary file for new stb_unpack.h
    local temp_file=$(mktemp)
    
    # Process stb_unpack.h line by line
    local in_header=false
    local in_impl=false
    local header_written=false
    local impl_written=false
    
    while IFS= read -r line; do
        if [[ "$line" == *"STBUP_MINIZ_HEADER_START"* ]]; then
            in_header=true
            echo "$line" >> "$temp_file"
            # Insert miniz.h content (with proper indentation and guards)
            echo "/* Embedded miniz.h - DO NOT EDIT MANUALLY - Use scripts/fuse_miniz.sh to update */" >> "$temp_file"
            echo "#ifndef MINIZ_H_INCLUDED" >> "$temp_file"
            echo "#define MINIZ_H_INCLUDED" >> "$temp_file"
            echo "" >> "$temp_file"
            # Read miniz.h and add it
            while IFS= read -r miniz_line; do
                # Skip the original include guard if present
                if [[ "$miniz_line" == "#ifndef MINIZ_H_INCLUDED" ]] || \
                   [[ "$miniz_line" == "#define MINIZ_H_INCLUDED" ]] || \
                   [[ "$miniz_line" == "#endif"*"MINIZ_H_INCLUDED"* ]]; then
                    continue
                fi
                echo "$miniz_line" >> "$temp_file"
            done < "$miniz_h"
            echo "" >> "$temp_file"
            echo "#endif /* MINIZ_H_INCLUDED */" >> "$temp_file"
            header_written=true
            continue
        elif [[ "$line" == *"STBUP_MINIZ_HEADER_END"* ]]; then
            in_header=false
            echo "$line" >> "$temp_file"
            continue
        elif [[ "$line" == *"STBUP_MINIZ_IMPL_START"* ]]; then
            in_impl=true
            echo "$line" >> "$temp_file"
            # Insert miniz.c content (with MINIZ_IMPLEMENTATION define)
            echo "/* Embedded miniz.c - DO NOT EDIT MANUALLY - Use scripts/fuse_miniz.sh to update */" >> "$temp_file"
            echo "#ifndef MINIZ_IMPLEMENTATION" >> "$temp_file"
            echo "#define MINIZ_IMPLEMENTATION" >> "$temp_file"
            echo "#endif" >> "$temp_file"
            echo "" >> "$temp_file"
            # Read miniz.c and add it
            while IFS= read -r miniz_line; do
                # Skip the MINIZ_IMPLEMENTATION define if present
                if [[ "$miniz_line" == "#define MINIZ_IMPLEMENTATION" ]] || \
                   [[ "$miniz_line" == "#ifdef MINIZ_IMPLEMENTATION" ]] || \
                   [[ "$miniz_line" == "#endif"*"MINIZ_IMPLEMENTATION"* ]]; then
                    continue
                fi
                # Skip #include "miniz.h" since it's already embedded above
                if [[ "$miniz_line" == '#include "miniz.h"' ]] || \
                   [[ "$miniz_line" == "#include \"miniz.h\"" ]] || \
                   [[ "$miniz_line" == "#include <miniz.h>" ]]; then
                    continue
                fi
                echo "$miniz_line" >> "$temp_file"
            done < "$miniz_c"
            impl_written=true
            continue
        elif [[ "$line" == *"STBUP_MINIZ_IMPL_END"* ]]; then
            in_impl=false
            echo "$line" >> "$temp_file"
            continue
        elif [ "$in_header" = true ] || [ "$in_impl" = true ]; then
            # Skip lines between markers (they've been replaced)
            continue
        else
            # Normal line, copy as-is
            echo "$line" >> "$temp_file"
        fi
    done < "$STB_UNPACK_H"
    
    if [ "$header_written" != true ] || [ "$impl_written" != true ]; then
        rm -f "$temp_file"
        error "Failed to write miniz content (header_written=$header_written, impl_written=$impl_written)"
    fi
    
    # Replace original file
    mv "$temp_file" "$STB_UNPACK_H"
    info "Successfully patched miniz into stb_unpack.h"
}

# Main
main() {
    local release_path="$1"
    
    info "Fusing miniz into stb_unpack.h..."
    
    # Extract miniz files
    local miniz_dir=$(extract_miniz "$release_path")
    
    # Cleanup function
    cleanup() {
        rm -rf "$miniz_dir"
    }
    trap cleanup EXIT
    
    # Patch stb_unpack.h
    patch_stb_unpack "$miniz_dir"
    
    info "Done! Remember to test the updated stb_unpack.h"
}

main "$@"


# Makefile for stb_unpack.h
# Primary build system for building and testing

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra
TEST_DIR = test

# Detect compiler
ifeq ($(shell which gcc 2>/dev/null),)
  ifeq ($(shell which clang 2>/dev/null),)
    $(error No C compiler found. Please install gcc or clang)
  else
    CC = clang
  endif
endif

.PHONY: all build test test-extract test-create test-compat test-targz test-targz-compat test-all example clean help

# Default target
all: build

# Build test programs
build: $(TEST_DIR)/build/test $(TEST_DIR)/build/test_create $(TEST_DIR)/build/test_targz $(TEST_DIR)/build/test_zip

$(TEST_DIR)/build/test: $(TEST_DIR)/src/test.c stb_unpack.h miniz.c
	@echo "Building extraction test..."
	@mkdir -p $(TEST_DIR)/build
	@$(CC) $(TEST_DIR)/src/test.c miniz.c -o $(TEST_DIR)/build/test $(CFLAGS) -I. -DMINIZ_IMPLEMENTATION

$(TEST_DIR)/build/test_create: $(TEST_DIR)/src/test_create.c stb_unpack.h miniz.c
	@echo "Building creation test..."
	@mkdir -p $(TEST_DIR)/build
	@$(CC) $(TEST_DIR)/src/test_create.c miniz.c -o $(TEST_DIR)/build/test_create $(CFLAGS) -I. -DMINIZ_IMPLEMENTATION

$(TEST_DIR)/build/test_targz: $(TEST_DIR)/src/test_targz.c stb_unpack.h miniz.c
	@echo "Building .tar.gz test..."
	@mkdir -p $(TEST_DIR)/build
	@$(CC) $(TEST_DIR)/src/test_targz.c miniz.c -o $(TEST_DIR)/build/test_targz $(CFLAGS) -I. -DMINIZ_IMPLEMENTATION

$(TEST_DIR)/build/test_zip: $(TEST_DIR)/src/test_zip.c stb_unpack.h miniz.c
	@echo "Building .zip test..."
	@mkdir -p $(TEST_DIR)/build
	@$(CC) $(TEST_DIR)/src/test_zip.c miniz.c -o $(TEST_DIR)/build/test_zip $(CFLAGS) -I. -DMINIZ_IMPLEMENTATION

# Build example program
example/extract_src: example/extract_src.c stb_unpack.h miniz.c
	@echo "Building extract_src example..."
	@mkdir -p example
	@$(CC) example/extract_src.c miniz.c -o example/extract_src $(CFLAGS) -I. -DMINIZ_IMPLEMENTATION

# Run tests (builds first if needed)
test: build
	@cd $(TEST_DIR) && ./run_all_tests.sh

test-extract: $(TEST_DIR)/test
	@cd $(TEST_DIR) && ./test_extract.sh

test-create: $(TEST_DIR)/test_create
	@cd $(TEST_DIR) && ./test_create.sh

test-compat: $(TEST_DIR)/test_create
	@cd $(TEST_DIR) && ./test_tar_compat.sh

test-targz: $(TEST_DIR)/test_targz
	@cd $(TEST_DIR) && ./test_targz.sh

test-targz-compat: $(TEST_DIR)/test_targz
	@cd $(TEST_DIR) && ./test_targz_compat.sh

# Alias for test
test-all: test

# Build example (alias for example/extract_src)
example: example/extract_src

# Clean build artifacts and test outputs
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(TEST_DIR)/build
	@rm -rf $(TEST_DIR)/output
	@rm -f example/extract_src
	@echo "Clean complete."

# Help
help:
	@echo "stb_unpack.h Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  make              - Build all test programs (default)"
	@echo "  make build        - Build all test programs"
	@echo "  make test          - Build and run all tests"
	@echo "  make test-extract  - Build and run extraction test"
	@echo "  make test-create   - Build and run creation test"
	@echo "  make test-compat   - Build and run compatibility test"
	@echo "  make test-targz     - Build and run .tar.gz test"
	@echo "  make test-targz-compat - Build and run .tar.gz compatibility test"
	@echo "  make test-all      - Alias for 'make test'"
	@echo "  make example      - Build extract_src example"
	@echo "  make clean         - Remove all build artifacts and test outputs"
	@echo "  make help          - Show this help"
	@echo ""
	@echo "Examples:"
	@echo "  make              # Just build"
	@echo "  make test         # Build and test"
	@echo "  make clean        # Clean up"


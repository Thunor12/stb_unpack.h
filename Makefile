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

.PHONY: all build test test-extract test-create test-compat test-all clean help

# Default target
all: build

# Build test programs
build: $(TEST_DIR)/test $(TEST_DIR)/test_create

$(TEST_DIR)/test: $(TEST_DIR)/test.c stb_unpack.h
	@echo "Building extraction test..."
	@$(CC) $(TEST_DIR)/test.c -o $(TEST_DIR)/test $(CFLAGS) -I.

$(TEST_DIR)/test_create: $(TEST_DIR)/test_create.c stb_unpack.h
	@echo "Building creation test..."
	@$(CC) $(TEST_DIR)/test_create.c -o $(TEST_DIR)/test_create $(CFLAGS) -I.

# Run tests (builds first if needed)
test: build
	@cd $(TEST_DIR) && ./run_all_tests.sh

test-extract: $(TEST_DIR)/test
	@cd $(TEST_DIR) && ./test_extract.sh

test-create: $(TEST_DIR)/test_create
	@cd $(TEST_DIR) && ./test_create.sh

test-compat: $(TEST_DIR)/test_create
	@cd $(TEST_DIR) && ./test_tar_compat.sh

# Alias for test
test-all: test

# Clean build artifacts and test outputs
clean:
	@echo "Cleaning build artifacts..."
	@rm -f $(TEST_DIR)/test $(TEST_DIR)/test_create $(TEST_DIR)/test.exe
	@rm -f $(TEST_DIR)/*.tar
	@rm -rf $(TEST_DIR)/out $(TEST_DIR)/tar_extracted
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
	@echo "  make test-all      - Alias for 'make test'"
	@echo "  make clean         - Remove all build artifacts and test outputs"
	@echo "  make help          - Show this help"
	@echo ""
	@echo "Examples:"
	@echo "  make              # Just build"
	@echo "  make test         # Build and test"
	@echo "  make clean        # Clean up"


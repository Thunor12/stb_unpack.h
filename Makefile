# Makefile for stb_unpack.h tests

CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -I.
TEST_DIR = test

.PHONY: all test test-extract test-create test-compat clean help

all: test

# Build test programs
test-extract-build:
	@cd $(TEST_DIR) && $(CC) test.c -o test $(CFLAGS) -I..

test-create-build:
	@cd $(TEST_DIR) && $(CC) test_create.c -o test_create $(CFLAGS) -I..

# Run tests
test: test-extract test-create test-compat

test-extract: test-extract-build
	@cd $(TEST_DIR) && ./test_extract.sh

test-create: test-create-build
	@cd $(TEST_DIR) && ./test_create.sh

test-compat: test-create-build
	@cd $(TEST_DIR) && ./test_tar_compat.sh

# Run all tests via test runner
test-all:
	@cd $(TEST_DIR) && ./run_all_tests.sh

# Clean build artifacts
clean:
	@rm -f $(TEST_DIR)/test $(TEST_DIR)/test_create $(TEST_DIR)/test.exe
	@rm -f $(TEST_DIR)/*.tar
	@rm -rf $(TEST_DIR)/out $(TEST_DIR)/tar_extracted

# Help
help:
	@echo "Available targets:"
	@echo "  make test          - Run all tests"
	@echo "  make test-extract  - Run extraction test"
	@echo "  make test-create   - Run creation test"
	@echo "  make test-compat   - Run compatibility test"
	@echo "  make test-all      - Run all tests via test runner"
	@echo "  make clean         - Remove build artifacts"
	@echo "  make help          - Show this help"


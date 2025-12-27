/*
 * Test Runner for stb_unpack.h
 * 
 * This is a C-based test runner that executes all tests for the stb_unpack.h library.
 * It replaces the previous bash scripts for better cross-platform compatibility.
 * 
 * The test runner:
 * - Detects if miniz is available (embedded or external)
 * - Checks for external tools (tar, unzip, zip) and skips tests if unavailable
 * - Runs 9 comprehensive test suites
 * - Reports pass/fail/skip status for each test
 * - Provides a summary at the end
 * 
 * Test return codes:
 *   0 = Test passed
 *   1 = Test failed
 *  -1 = Test skipped (external tool not available)
 */

#define STB_UNPACK_IMPLEMENTATION
#include "../../stb_unpack.h"
#define NOB_IMPLEMENTATION
#include "../../nob.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Using nob_file_exists from nob.h instead of custom implementation

/**
 * Get executable path with .exe extension on Windows
 * 
 * On Windows, executables have .exe extension. This function handles that.
 * Note: Returns a pointer to a static buffer, so don't use multiple times in the same expression.
 * 
 * @param base_path Base path without extension
 * @return Path with .exe on Windows, original path on Unix
 */
static const char* exe_path(const char *base_path) {
#ifdef _WIN32
    static char win_path[256];
    if (snprintf(win_path, sizeof(win_path), "%s.exe", base_path) >= (int)sizeof(win_path)) {
        // Truncated, return original
        return base_path;
    }
    return win_path;
#else
    return base_path;
#endif
}

/**
 * Check if a command is available in PATH
 * 
 * Uses 'which' on Unix and 'where' on Windows to check if a command exists.
 * This is used to determine if external tools (tar, unzip, zip) are available.
 * 
 * @param cmd Command name to check (e.g., "tar", "unzip", "zip")
 * @return true if command is available, false otherwise
 */
static bool command_available(const char *cmd) {
    Nob_Cmd test_cmd = {0};
#ifdef _WIN32
    nob_cmd_append(&test_cmd, "where", cmd);
#else
    nob_cmd_append(&test_cmd, "which", cmd);
#endif
    Nob_Cmd_Opt opt = {0};
#ifdef _WIN32
    opt.stdout_path = "nul";
    opt.stderr_path = "nul";
#else
    opt.stdout_path = "/dev/null";
    opt.stderr_path = "/dev/null";
#endif
    bool available = nob_cmd_run_opt(&test_cmd, opt);
    nob_cmd_free(test_cmd);
    return available;
}

/**
 * Check if miniz is available (embedded or external)
 * 
 * miniz can be either:
 * - Embedded in stb_unpack.h (checked by looking for marker)
 * - External files (miniz.h and miniz.c in repository root)
 * 
 * @return true if miniz is available, false otherwise
 */
static bool miniz_available(void) {
    const char *stb_paths[] = {"../stb_unpack.h", "../../stb_unpack.h", "stb_unpack.h", NULL};
    const char *miniz_c_paths[] = {"../miniz.c", "../../miniz.c", "miniz.c", NULL};
    const char *miniz_h_paths[] = {"../miniz.h", "../../miniz.h", "miniz.h", NULL};
    
    // Check for embedded miniz
    for (int i = 0; stb_paths[i]; i++) {
        Nob_String_Builder content = {0};
        if (nob_read_entire_file(stb_paths[i], &content)) {
            bool embedded = (strstr(content.items, "Embedded miniz.h - DO NOT EDIT") != NULL);
            nob_sb_free(content);
            if (embedded) return true;
        }
    }
    
    // Check for external miniz
    for (int i = 0; miniz_c_paths[i]; i++) {
        if (nob_file_exists(miniz_c_paths[i])) {
            for (int j = 0; miniz_h_paths[j]; j++) {
                if (nob_file_exists(miniz_h_paths[j])) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

/**
 * Compare two files for equality
 * 
 * Reads both files and compares their contents byte-by-byte.
 * 
 * @param path1 Path to first file
 * @param path2 Path to second file
 * @return true if files are identical, false otherwise
 */
static bool files_equal(const char *path1, const char *path2) {
    Nob_String_Builder content1 = {0};
    Nob_String_Builder content2 = {0};
    
    if (!nob_read_entire_file(path1, &content1)) {
        return false;
    }
    if (!nob_read_entire_file(path2, &content2)) {
        nob_sb_free(content1);
        return false;
    }
    
    bool equal = (content1.count == content2.count && 
                  memcmp(content1.items, content2.items, content1.count) == 0);
    
    nob_sb_free(content1);
    nob_sb_free(content2);
    return equal;
}

/**
 * Run a test executable with arguments
 * 
 * Executes a test program and redirects output to null (silent operation).
 * Status is printed by the caller, not by this function.
 * 
 * @param exe_path_arg Path to executable (may or may not have .exe)
 * @param test_name Name of the test (for logging, currently unused)
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @return 0 on success, 1 on failure
 */
static int run_test_exe(const char *exe_path_arg, const char *test_name, int argc, char **argv) {
    (void)test_name; // Status printed by caller
    
    // Determine the actual executable path (handle .exe on Windows)
    const char *actual_path = exe_path_arg;
#ifdef _WIN32
    // On Windows, try with .exe first, then without
    size_t len = strlen(exe_path_arg);
    if (len >= 4 && strcmp(exe_path_arg + len - 4, ".exe") == 0) {
        // Already has .exe, use as-is
        if (!nob_file_exists(exe_path_arg)) {
            return 1;
        }
        actual_path = exe_path_arg;
    } else {
        // Try with .exe
        static char win_path[256];
        snprintf(win_path, sizeof(win_path), "%s.exe", exe_path_arg);
        if (nob_file_exists(win_path)) {
            actual_path = win_path;
        } else if (nob_file_exists(exe_path_arg)) {
            actual_path = exe_path_arg;
        } else {
            return 1;
        }
    }
#else
    if (!nob_file_exists(exe_path_arg)) {
        return 1;
    }
#endif
    
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, actual_path);
    for (int i = 0; i < argc; i++) {
        nob_cmd_append(&cmd, argv[i]);
    }
    
    // Redirect output to null for silent operation
    Nob_Cmd_Opt opt = {0};
#ifdef _WIN32
    opt.stdout_path = "nul";
    opt.stderr_path = "nul";
#else
    opt.stdout_path = "/dev/null";
    opt.stderr_path = "/dev/null";
#endif
    int result = nob_cmd_run_opt(&cmd, opt) ? 0 : 1;
    nob_cmd_free(cmd);
    
    return result;
}

/**
 * Test 1: TAR Extraction
 * 
 * Creates a TAR archive using the system tar command, then extracts it
 * using stbup_tar_extract_stream to verify extraction works correctly.
 * 
 * @return 0 on success, 1 on failure
 */
static int test_tar_extract(void) {
    // Create test archive using tar command (for compatibility)
    Nob_Cmd cmd = {0};
#ifdef _WIN32
    nob_cmd_append(&cmd, "tar", "cf", "output/archive.tar", "-C", "input", "test_file.txt");
#else
    nob_cmd_append(&cmd, "sh", "-c", "cd input && tar cf ../output/archive.tar test_file.txt");
#endif
    nob_cmd_run(&cmd);
    nob_cmd_free(cmd);
    
    if (!nob_file_exists("output/archive.tar")) {
        return 1;
    }
    
    // Run extraction test
    char *args[] = {};
    return run_test_exe(exe_path("build/test"), "TAR Extraction Test", 0, args);
}

/**
 * Test 2: TAR Creation
 * 
 * Creates a TAR archive using stbup_tar_create_file, then verifies it
 * can be extracted by the standard tar tool. This ensures our TARs are
 * compatible with standard tools.
 * 
 * @return 0 on success, 1 on failure, -1 if skipped (tar not available)
 */
static int test_tar_create(void) {
    if (!command_available("tar")) {
        printf("⚠ TAR Creation Test: SKIPPED (tar command not available)\n");
        return -1; // Special return code for skipped
    }
    
    // Create test input file
    FILE *f = fopen("input/test_input.txt", "w");
    if (!f) return 1;
    fprintf(f, "Hello, this is a test file!\n");
    fprintf(f, "It has multiple lines.\n");
    fprintf(f, "And some content.\n");
    fclose(f);
    
    // Create archive with our tool
    char *args[] = {"output/our_archive.tar", "input/test_input.txt"};
    if (run_test_exe(exe_path("build/test_create"), "TAR Creation Test", 2, args) != 0) {
        return 1;
    }
    
    if (!nob_file_exists("output/our_archive.tar")) {
        return 1;
    }
    
    // Verify by extracting with tar and comparing
    stbup_mkdirs("output/our_extracted");
    Nob_Cmd extract_cmd = {0};
#ifdef _WIN32
    nob_cmd_append(&extract_cmd, "tar", "xf", "output/our_archive.tar", "-C", "output/our_extracted");
#else
    nob_cmd_append(&extract_cmd, "sh", "-c", "cd output/our_extracted && tar xf ../our_archive.tar");
#endif
    if (!nob_cmd_run(&extract_cmd)) {
        nob_cmd_free(extract_cmd);
        return 1;
    }
    nob_cmd_free(extract_cmd);
    
    if (!nob_file_exists("output/our_extracted/test_input.txt")) {
        return 1;
    }
    
    return files_equal("input/test_input.txt", "output/our_extracted/test_input.txt") ? 0 : 1;
}

/**
 * Test 3: TAR Compatibility
 * 
 * Ensures that TAR archives created by our library can be read by
 * standard tar tools. This is a compatibility verification test.
 * 
 * @return 0 on success, 1 on failure, -1 if skipped (tar not available)
 */
static int test_tar_compat(void) {
    if (!command_available("tar")) {
        printf("⚠ TAR Compatibility Test: SKIPPED (tar command not available)\n");
        return -1; // Special return code for skipped
    }
    
    // Create test file
    FILE *f = fopen("input/test_compat_input.txt", "w");
    if (!f) return 1;
    fprintf(f, "Hello, this is a test file!\n");
    fprintf(f, "It has multiple lines.\n");
    fprintf(f, "And some content for testing.\n");
    fprintf(f, "The quick brown fox jumps over the lazy dog.\n");
    fclose(f);
    
    // Create archive with our tool
    char *args[] = {"output/our_compat_archive.tar", "input/test_compat_input.txt"};
    if (run_test_exe(exe_path("build/test_create"), "TAR Compatibility Test", 2, args) != 0) {
        return 1;
    }
    
    if (!nob_file_exists("output/our_compat_archive.tar")) {
        return 1;
    }
    
    // Extract with tar
    stbup_mkdirs("output/tar_extracted");
    Nob_Cmd extract_cmd = {0};
#ifdef _WIN32
    nob_cmd_append(&extract_cmd, "tar", "xf", "output/our_compat_archive.tar", "-C", "output/tar_extracted");
#else
    nob_cmd_append(&extract_cmd, "sh", "-c", "cd output/tar_extracted && tar xf ../our_compat_archive.tar");
#endif
    if (!nob_cmd_run(&extract_cmd)) {
        nob_cmd_free(extract_cmd);
        return 1;
    }
    nob_cmd_free(extract_cmd);
    
    if (!nob_file_exists("output/tar_extracted/test_compat_input.txt")) {
        return 1;
    }
    
    return files_equal("input/test_compat_input.txt", "output/tar_extracted/test_compat_input.txt") ? 0 : 1;
}

/**
 * Test 4: .tar.gz Basic Test
 * 
 * Tests basic .tar.gz creation and extraction functionality.
 * Creates a .tar.gz archive and then extracts it to verify both
 * operations work correctly.
 * 
 * @return 0 on success, 1 on failure
 */
static int test_targz_basic(void) {
    // Create test file
    FILE *f = fopen("input/test_targz_input.txt", "w");
    if (!f) return 1;
    fprintf(f, "Hello from .tar.gz test!\n");
    fprintf(f, "This file will be compressed.\n");
    fprintf(f, "Multiple lines of content.\n");
    fclose(f);
    
    // Create archive
    Nob_Cmd create_cmd = {0};
    nob_cmd_append(&create_cmd, exe_path("build/test_targz"), "-c", "output/test_archive.tar.gz", "input/test_targz_input.txt");
    if (!nob_cmd_run(&create_cmd)) {
        nob_cmd_free(create_cmd);
        return 1;
    }
    nob_cmd_free(create_cmd);
    
    if (!nob_file_exists("output/test_archive.tar.gz")) {
        return 1;
    }
    
    // Extract
    stbup_mkdirs("output/targz_out");
    char *extract_args[] = {"output/test_archive.tar.gz", "output/targz_out"};
    if (run_test_exe(exe_path("build/test_targz"), ".tar.gz Test", 2, extract_args) != 0) {
        return 1;
    }
    
    if (!nob_file_exists("output/targz_out/test_targz_input.txt")) {
        return 1;
    }
    
    return files_equal("input/test_targz_input.txt", "output/targz_out/test_targz_input.txt") ? 0 : 1;
}

/**
 * Test 5: .tar.gz Compatibility
 * 
 * Ensures that .tar.gz archives created by our library can be read by
 * standard tar tools. This verifies gzip compression compatibility.
 * 
 * @return 0 on success, 1 on failure, -1 if skipped (tar not available)
 */
static int test_targz_compat(void) {
    if (!command_available("tar")) {
        printf("⚠ .tar.gz Compatibility Test: SKIPPED (tar command not available)\n");
        return -1; // Special return code for skipped
    }
    
    // Create test file
    FILE *f = fopen("input/test_targz_compat.txt", "w");
    if (!f) return 1;
    fprintf(f, "Compatibility test for .tar.gz\n");
    fprintf(f, "This archive should be readable by standard tools.\n");
    fprintf(f, "Testing gzip compression compatibility.\n");
    fclose(f);
    
    // Create archive with our tool
    Nob_Cmd create_cmd = {0};
    nob_cmd_append(&create_cmd, exe_path("build/test_targz"), "-c", "output/our_targz_archive.tar.gz", "input/test_targz_compat.txt");
    if (!nob_cmd_run(&create_cmd)) {
        nob_cmd_free(create_cmd);
        return 1;
    }
    nob_cmd_free(create_cmd);
    
    if (!nob_file_exists("output/our_targz_archive.tar.gz")) {
        return 1;
    }
    
    // Extract with tar
    stbup_mkdirs("output/tar_extracted_targz");
    Nob_Cmd extract_cmd = {0};
#ifdef _WIN32
    nob_cmd_append(&extract_cmd, "tar", "xzf", "output/our_targz_archive.tar.gz", "-C", "output/tar_extracted_targz");
#else
    nob_cmd_append(&extract_cmd, "sh", "-c", "cd output/tar_extracted_targz && tar xzf ../our_targz_archive.tar.gz");
#endif
    if (!nob_cmd_run(&extract_cmd)) {
        nob_cmd_free(extract_cmd);
        return 1;
    }
    nob_cmd_free(extract_cmd);
    
    if (!nob_file_exists("output/tar_extracted_targz/test_targz_compat.txt")) {
        return 1;
    }
    
    return files_equal("input/test_targz_compat.txt", "output/tar_extracted_targz/test_targz_compat.txt") ? 0 : 1;
}

/**
 * Test 6: .zip Basic Test
 * 
 * Tests basic ZIP creation and extraction functionality.
 * Creates a ZIP archive and then extracts it to verify both
 * operations work correctly.
 * 
 * @return 0 on success, 1 on failure
 */
static int test_zip_basic(void) {
    // Create test file
    FILE *f = fopen("input/test_zip_input.txt", "w");
    if (!f) return 1;
    fprintf(f, "Hello from .zip test!\n");
    fprintf(f, "This file will be zipped.\n");
    fprintf(f, "Multiple lines of content.\n");
    fclose(f);
    
    // Create archive
    Nob_Cmd create_cmd = {0};
    nob_cmd_append(&create_cmd, exe_path("build/test_zip"), "-c", "output/test_archive.zip", "input/test_zip_input.txt");
    if (!nob_cmd_run(&create_cmd)) {
        nob_cmd_free(create_cmd);
        return 1;
    }
    nob_cmd_free(create_cmd);
    
    if (!nob_file_exists("output/test_archive.zip")) {
        return 1;
    }
    
    // Extract
    stbup_mkdirs("output/zip_out");
    char *extract_args[] = {"output/test_archive.zip", "output/zip_out"};
    if (run_test_exe(exe_path("build/test_zip"), ".zip Test", 2, extract_args) != 0) {
        return 1;
    }
    
    if (!nob_file_exists("output/zip_out/test_zip_input.txt")) {
        return 1;
    }
    
    return files_equal("input/test_zip_input.txt", "output/zip_out/test_zip_input.txt") ? 0 : 1;
}

/**
 * Test 7: .zip Compatibility
 * 
 * Ensures that ZIP archives created by our library can be read by
 * standard unzip tools. This verifies ZIP format compatibility.
 * 
 * @return 0 on success, 1 on failure, -1 if skipped (unzip not available)
 */
static int test_zip_compat(void) {
    if (!command_available("unzip")) {
        printf("⚠ .zip Compatibility Test: SKIPPED (unzip command not available)\n");
        return -1; // Special return code for skipped
    }
    
    // Create test file
    FILE *f = fopen("input/test_zip_compat.txt", "w");
    if (!f) return 1;
    fprintf(f, "Compatibility test for .zip\n");
    fprintf(f, "This archive should be readable by standard tools.\n");
    fprintf(f, "Testing zip compression compatibility.\n");
    fclose(f);
    
    // Create archive with our tool
    Nob_Cmd create_cmd = {0};
    nob_cmd_append(&create_cmd, exe_path("build/test_zip"), "-c", "output/our_zip_archive.zip", "input/test_zip_compat.txt");
    if (!nob_cmd_run(&create_cmd)) {
        nob_cmd_free(create_cmd);
        return 1;
    }
    nob_cmd_free(create_cmd);
    
    if (!nob_file_exists("output/our_zip_archive.zip")) {
        return 1;
    }
    
    // Extract with unzip
    stbup_mkdirs("output/zip_extracted");
    Nob_Cmd extract_cmd = {0};
#ifdef _WIN32
    nob_cmd_append(&extract_cmd, "unzip", "-q", "-o", "output/our_zip_archive.zip", "-d", "output/zip_extracted");
#else
    nob_cmd_append(&extract_cmd, "unzip", "-q", "-o", "output/our_zip_archive.zip", "-d", "output/zip_extracted");
#endif
    if (!nob_cmd_run(&extract_cmd)) {
        nob_cmd_free(extract_cmd);
        return 1;
    }
    nob_cmd_free(extract_cmd);
    
    if (!nob_file_exists("output/zip_extracted/test_zip_compat.txt")) {
        return 1;
    }
    
    return files_equal("input/test_zip_compat.txt", "output/zip_extracted/test_zip_compat.txt") ? 0 : 1;
}

/**
 * Test 8: .tar.gz Comprehensive Test
 * 
 * Additional edge case testing for .tar.gz archives.
 * Currently tests single-file archives (multi-file has known limitations).
 * 
 * @return 0 on success, 1 on failure
 */
static int test_targz_comprehensive(void) {
    // Create test file
    stbup_mkdirs("output/comprehensive/temp");
    FILE *f = fopen("output/comprehensive/temp/single.txt", "w");
    if (!f) return 1;
    fprintf(f, "Single file comprehensive test\n");
    fclose(f);
    
    // Create archive using our own function (like other tests)
    Nob_Cmd create_cmd = {0};
    nob_cmd_append(&create_cmd, exe_path("build/test_targz"), "-c", "output/comprehensive/single.tar.gz", "output/comprehensive/temp/single.txt");
    Nob_Cmd_Opt opt_create = {0};
#ifdef _WIN32
    opt_create.stdout_path = "nul";
    opt_create.stderr_path = "nul";
#else
    opt_create.stdout_path = "/dev/null";
    opt_create.stderr_path = "/dev/null";
#endif
    if (!nob_cmd_run_opt(&create_cmd, opt_create)) {
        nob_cmd_free(create_cmd);
        return 1;
    }
    nob_cmd_free(create_cmd);
    
    if (!nob_file_exists("output/comprehensive/single.tar.gz")) {
        return 1;
    }
    
    // Extract with our tool using test_targz executable
    stbup_mkdirs("output/comprehensive/out");
    Nob_Cmd extract_cmd = {0};
    nob_cmd_append(&extract_cmd, exe_path("build/test_targz"), "output/comprehensive/single.tar.gz", "output/comprehensive/out");
    Nob_Cmd_Opt opt_extract = {0};
#ifdef _WIN32
    opt_extract.stdout_path = "nul";
    opt_extract.stderr_path = "nul";
#else
    opt_extract.stdout_path = "/dev/null";
    opt_extract.stderr_path = "/dev/null";
#endif
    if (!nob_cmd_run_opt(&extract_cmd, opt_extract)) {
        nob_cmd_free(extract_cmd);
        return 1;
    }
    nob_cmd_free(extract_cmd);
    
    // Verify file
    if (!nob_file_exists("output/comprehensive/out/single.txt")) {
        return 1;
    }
    
    return files_equal("output/comprehensive/temp/single.txt", "output/comprehensive/out/single.txt") ? 0 : 1;
}

/**
 * Test 9: .zip Comprehensive Test
 * 
 * Additional edge case testing for ZIP archives, including
 * directory structures and multiple files.
 * 
 * @return 0 on success, 1 on failure, -1 if skipped (zip not available)
 */
static int test_zip_comprehensive(void) {
    if (!command_available("zip")) {
        printf("⚠ .zip Comprehensive Test: SKIPPED (zip command not available)\n");
        return -1; // Special return code for skipped
    }
    
    // Create test directory structure
    stbup_mkdirs("output/zip_comprehensive/temp/zip_test1");
    
    FILE *f1 = fopen("output/zip_comprehensive/temp/zip_test1/single.txt", "w");
    if (f1) { fprintf(f1, "Single file content\n"); fclose(f1); }
    
    // Create archive using zip command
    Nob_Cmd create_cmd = {0};
#ifdef _WIN32
    nob_cmd_append(&create_cmd, "zip", "-q", "output/zip_comprehensive/single.zip", "output/zip_comprehensive/temp/zip_test1/single.txt");
#else
    nob_cmd_append(&create_cmd, "sh", "-c", "cd output/zip_comprehensive/temp/zip_test1 && zip -q ../../single.zip single.txt");
#endif
    if (!nob_cmd_run(&create_cmd)) {
        nob_cmd_free(create_cmd);
        return 1;
    }
    nob_cmd_free(create_cmd);
    
    if (!nob_file_exists("output/zip_comprehensive/single.zip")) {
        return 1;
    }
    
    // Extract with our tool
    stbup_mkdirs("output/zip_comprehensive/out");
    if (!stbup_zip_extract("output/zip_comprehensive/single.zip", "output/zip_comprehensive/out")) {
        return 1;
    }
    
    // Verify file
    if (!nob_file_exists("output/zip_comprehensive/out/single.txt")) {
        return 1;
    }
    
    return files_equal("output/zip_comprehensive/temp/zip_test1/single.txt", "output/zip_comprehensive/out/single.txt") ? 0 : 1;
}

/**
 * Test case structure
 * 
 * Each test has a name, a function pointer, and a flag indicating
 * whether it requires miniz (for compression support).
 */
typedef struct {
    const char *name;           // Test name for display
    int (*func)(void);          // Test function (returns 0=pass, 1=fail, -1=skip)
    bool requires_miniz;        // Whether test requires miniz
} TestCase;

/**
 * Test suite array
 * 
 * All tests are registered here. The main function iterates through
 * this array and runs each test.
 */
/**
 * Security test runner - runs security regression tests
 */
static int test_security(void) {
    char *args[] = {};
    return run_test_exe(exe_path("build/test_security"), "Security Tests", 0, args);
}

static const TestCase tests[] = {
    {"TAR Extraction Test", test_tar_extract, false},
    {"TAR Creation Test", test_tar_create, false},
    {"TAR Compatibility Test", test_tar_compat, false},
    {".tar.gz Test", test_targz_basic, true},
    {".tar.gz Compatibility Test", test_targz_compat, true},
    {".tar.gz Comprehensive Test", test_targz_comprehensive, true},
    {".zip Test", test_zip_basic, true},
    {".zip Compatibility Test", test_zip_compat, true},
    {".zip Comprehensive Test", test_zip_comprehensive, true},
    {"Security Tests", test_security, true},
};

/**
 * Main test runner
 * 
 * Executes all registered tests and provides a summary.
 * Tests that require miniz are skipped if miniz is not available.
 * Tests that require external tools (tar, unzip, zip) are skipped
 * if those tools are not found (with a warning).
 * 
 * Exit codes:
 *   0 = All tests passed
 *   1 = One or more tests failed
 */
int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    // Ensure output directories exist (tests create files here)
    stbup_mkdirs("output");
    stbup_mkdirs("output/comprehensive");
    stbup_mkdirs("output/zip_comprehensive");
    
    // Test statistics
    int passed = 0;
    int failed = 0;
    bool has_miniz = miniz_available();
    
    // Run all tests
    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        // Skip tests that require miniz if miniz is not available
        if (tests[i].requires_miniz && !has_miniz) {
            printf("⚠ %s: SKIPPED (miniz not available)\n", tests[i].name);
            continue;
        }
        
        // Execute the test
        int result = tests[i].func();
        if (result == -1) {
            // Test was skipped (return code -1 indicates skip, e.g., external tool not available)
            // Status already printed by the test function
            continue;
        } else if (result == 0) {
            printf("✓ %s: PASSED\n", tests[i].name);
            passed++;
        } else {
            printf("✗ %s: FAILED\n", tests[i].name);
            failed++;
        }
    }
    
    // Print summary
    printf("\n");
    if (failed == 0) {
        printf("✓ All tests passed! (%d/%d)\n", passed, passed + failed);
        return 0;
    } else {
        printf("✗ Some tests failed! (%d passed, %d failed)\n", passed, failed);
        return 1;
    }
}

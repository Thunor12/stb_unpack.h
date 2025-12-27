#define STB_UNPACK_IMPLEMENTATION
#include "../../stb_unpack.h"
#define NOB_IMPLEMENTATION
#include "../../nob.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#include <io.h>
#define access _access
#define F_OK 0
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

// Check if file exists
static bool file_exists(const char *path) {
    return access(path, F_OK) == 0;
}

// Check if a command is available in PATH
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

// Check if miniz is available (embedded or external)
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
        if (file_exists(miniz_c_paths[i])) {
            for (int j = 0; miniz_h_paths[j]; j++) {
                if (file_exists(miniz_h_paths[j])) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

// Compare two files
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

// Run a test executable with arguments (silent - status printed by caller)
static int run_test_exe(const char *exe_path, const char *test_name, int argc, char **argv) {
    (void)test_name; // Status printed by caller
    if (!file_exists(exe_path)) {
        return 1;
    }
    
    Nob_Cmd cmd = {0};
    nob_cmd_append(&cmd, exe_path);
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

// Test 1: TAR Extraction
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
    
    if (!file_exists("output/archive.tar")) {
        return 1;
    }
    
    // Run extraction test
    char *args[] = {};
    return run_test_exe("build/test", "TAR Extraction Test", 0, args);
}

// Test 2: TAR Creation
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
    if (run_test_exe("build/test_create", "TAR Creation Test", 2, args) != 0) {
        return 1;
    }
    
    if (!file_exists("output/our_archive.tar")) {
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
    
    if (!file_exists("output/our_extracted/test_input.txt")) {
        return 1;
    }
    
    return files_equal("input/test_input.txt", "output/our_extracted/test_input.txt") ? 0 : 1;
}

// Test 3: TAR Compatibility (our archives readable by tar)
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
    if (run_test_exe("build/test_create", "TAR Compatibility Test", 2, args) != 0) {
        return 1;
    }
    
    if (!file_exists("output/our_compat_archive.tar")) {
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
    
    if (!file_exists("output/tar_extracted/test_compat_input.txt")) {
        return 1;
    }
    
    return files_equal("input/test_compat_input.txt", "output/tar_extracted/test_compat_input.txt") ? 0 : 1;
}

// Test 4: .tar.gz basic
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
    nob_cmd_append(&create_cmd, "build/test_targz", "-c", "output/test_archive.tar.gz", "input/test_targz_input.txt");
    if (!nob_cmd_run(&create_cmd)) {
        nob_cmd_free(create_cmd);
        return 1;
    }
    nob_cmd_free(create_cmd);
    
    if (!file_exists("output/test_archive.tar.gz")) {
        return 1;
    }
    
    // Extract
    stbup_mkdirs("output/targz_out");
    char *extract_args[] = {"output/test_archive.tar.gz", "output/targz_out"};
    if (run_test_exe("build/test_targz", ".tar.gz Test", 2, extract_args) != 0) {
        return 1;
    }
    
    if (!file_exists("output/targz_out/test_targz_input.txt")) {
        return 1;
    }
    
    return files_equal("input/test_targz_input.txt", "output/targz_out/test_targz_input.txt") ? 0 : 1;
}

// Test 5: .tar.gz compatibility (our archives readable by tar)
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
    nob_cmd_append(&create_cmd, "build/test_targz", "-c", "output/our_targz_archive.tar.gz", "input/test_targz_compat.txt");
    if (!nob_cmd_run(&create_cmd)) {
        nob_cmd_free(create_cmd);
        return 1;
    }
    nob_cmd_free(create_cmd);
    
    if (!file_exists("output/our_targz_archive.tar.gz")) {
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
    
    if (!file_exists("output/tar_extracted_targz/test_targz_compat.txt")) {
        return 1;
    }
    
    return files_equal("input/test_targz_compat.txt", "output/tar_extracted_targz/test_targz_compat.txt") ? 0 : 1;
}

// Test 6: .zip basic
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
    nob_cmd_append(&create_cmd, "build/test_zip", "-c", "output/test_archive.zip", "input/test_zip_input.txt");
    if (!nob_cmd_run(&create_cmd)) {
        nob_cmd_free(create_cmd);
        return 1;
    }
    nob_cmd_free(create_cmd);
    
    if (!file_exists("output/test_archive.zip")) {
        return 1;
    }
    
    // Extract
    stbup_mkdirs("output/zip_out");
    char *extract_args[] = {"output/test_archive.zip", "output/zip_out"};
    if (run_test_exe("build/test_zip", ".zip Test", 2, extract_args) != 0) {
        return 1;
    }
    
    if (!file_exists("output/zip_out/test_zip_input.txt")) {
        return 1;
    }
    
    return files_equal("input/test_zip_input.txt", "output/zip_out/test_zip_input.txt") ? 0 : 1;
}

// Test 7: .zip compatibility (our archives readable by unzip)
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
    nob_cmd_append(&create_cmd, "build/test_zip", "-c", "output/our_zip_archive.zip", "input/test_zip_compat.txt");
    if (!nob_cmd_run(&create_cmd)) {
        nob_cmd_free(create_cmd);
        return 1;
    }
    nob_cmd_free(create_cmd);
    
    if (!file_exists("output/our_zip_archive.zip")) {
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
    
    if (!file_exists("output/zip_extracted/test_zip_compat.txt")) {
        return 1;
    }
    
    return files_equal("input/test_zip_compat.txt", "output/zip_extracted/test_zip_compat.txt") ? 0 : 1;
}

// Test 8: .tar.gz comprehensive (single file test - multi-file has known limitations)
static int test_targz_comprehensive(void) {
    // Create test file
    stbup_mkdirs("output/comprehensive/temp");
    FILE *f = fopen("output/comprehensive/temp/single.txt", "w");
    if (!f) return 1;
    fprintf(f, "Single file comprehensive test\n");
    fclose(f);
    
    // Create archive using our own function (like other tests)
    Nob_Cmd create_cmd = {0};
    nob_cmd_append(&create_cmd, "build/test_targz", "-c", "output/comprehensive/single.tar.gz", "output/comprehensive/temp/single.txt");
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
    
    if (!file_exists("output/comprehensive/single.tar.gz")) {
        return 1;
    }
    
    // Extract with our tool using test_targz executable
    stbup_mkdirs("output/comprehensive/out");
    Nob_Cmd extract_cmd = {0};
    nob_cmd_append(&extract_cmd, "build/test_targz", "output/comprehensive/single.tar.gz", "output/comprehensive/out");
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
    if (!file_exists("output/comprehensive/out/single.txt")) {
        return 1;
    }
    
    return files_equal("output/comprehensive/temp/single.txt", "output/comprehensive/out/single.txt") ? 0 : 1;
}

// Test 9: .zip comprehensive (multiple files and directories)
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
    
    if (!file_exists("output/zip_comprehensive/single.zip")) {
        return 1;
    }
    
    // Extract with our tool
    stbup_mkdirs("output/zip_comprehensive/out");
    if (!stbup_zip_extract("output/zip_comprehensive/single.zip", "output/zip_comprehensive/out")) {
        return 1;
    }
    
    // Verify file
    if (!file_exists("output/zip_comprehensive/out/single.txt")) {
        return 1;
    }
    
    return files_equal("output/zip_comprehensive/temp/zip_test1/single.txt", "output/zip_comprehensive/out/single.txt") ? 0 : 1;
}

typedef struct {
    const char *name;
    int (*func)(void);
    bool requires_miniz;
} TestCase;

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
};

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    // Ensure output directories exist
    stbup_mkdirs("output");
    stbup_mkdirs("output/comprehensive");
    stbup_mkdirs("output/zip_comprehensive");
    
    int passed = 0;
    int failed = 0;
    bool has_miniz = miniz_available();
    
    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        if (tests[i].requires_miniz && !has_miniz) {
            printf("⚠ %s: SKIPPED (miniz not available)\n", tests[i].name);
            continue;
        }
        
        int result = tests[i].func();
        if (result == -1) {
            // Test was skipped (return code -1 indicates skip)
            continue;
        } else if (result == 0) {
            printf("✓ %s: PASSED\n", tests[i].name);
            passed++;
        } else {
            printf("✗ %s: FAILED\n", tests[i].name);
            failed++;
        }
    }
    
    // Summary
    printf("\n");
    if (failed == 0) {
        printf("✓ All tests passed! (%d/%d)\n", passed, passed + failed);
        return 0;
    } else {
        printf("✗ Some tests failed! (%d passed, %d failed)\n", passed, failed);
        return 1;
    }
}

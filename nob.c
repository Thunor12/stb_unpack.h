/*
 * stb_unpack.h Build System
 * 
 * This is a single-file build system using nob.c (similar to make).
 * It handles:
 * - Building the nob build system itself
 * - Compiling test programs
 * - Compiling example programs
 * - Running the test suite
 * - Cleaning build artifacts
 * 
 * Usage:
 *   ./nob              # Build and run tests (default)
 *   ./nob build        # Build test programs only
 *   ./nob test         # Build and run all tests
 *   ./nob example      # Build example program
 *   ./nob clean        # Clean build artifacts
 *   ./nob help         # Show help message
 */

#define NOB_IMPLEMENTATION
#include "nob.h"

// Compiler configuration
#ifdef _WIN32
#define CC "gcc.exe"  // Windows uses gcc.exe
#else
#define CC "gcc"      // Unix-like systems use gcc
#endif

// Directory paths
#define BUILD_DIR "test/build/"      // Where compiled test executables go
#define TEST_SRC_DIR "test/src/"     // Where test source files are
#define EXAMPLE_DIR "example/"       // Where example programs are

// Compiler flags
#define CFLAGS "-std=c99", "-Wall", "-Wextra", "-I.", "-D_POSIX_C_SOURCE=200809L"

/**
 * Check if miniz is embedded in stb_unpack.h
 * 
 * When miniz is embedded, we don't need to compile miniz.c separately.
 * This function checks for the embedded marker in stb_unpack.h.
 * 
 * @return 1 if miniz is embedded, 0 otherwise
 */
static int is_miniz_embedded(void)
{
    Nob_String_Builder content = { 0 };
    if (!nob_read_entire_file("stb_unpack.h", &content))
    {
        return 0;
    }
    int embedded = (strstr(content.items, "Embedded miniz.h - DO NOT EDIT") != NULL);
    nob_sb_free(content);
    return embedded;
}

/**
 * Compile a test executable
 * 
 * Compiles a test program from source, handling:
 * - Windows vs Unix executable naming (.exe extension)
 * - Conditional miniz.c linking (only if not embedded)
 * - Incremental builds (skips if up-to-date)
 * 
 * @param exe_name Name of the executable (without extension)
 * @param src_file Path to the source file
 * @return 1 on success, 0 on failure
 */
static int compile_test_exe(const char *exe_name, const char *src_file)
{
    Nob_Cmd cmd = { 0 };
    Nob_String_Builder out_path = { 0 };
#ifdef _WIN32
    nob_sb_appendf(&out_path, "%s%s.exe", BUILD_DIR, exe_name);
#else
    nob_sb_appendf(&out_path, "%s%s", BUILD_DIR, exe_name);
#endif

    Nob_File_Paths deps = { 0 };
    nob_da_append(&deps, src_file);
    nob_da_append(&deps, "stb_unpack.h");

    // Check if we need to rebuild
    if (!nob_needs_rebuild(out_path.items, deps.items, deps.count))
    {
        nob_log(NOB_INFO, "%s up to date", out_path.items);
        nob_sb_free(out_path);
        nob_da_free(deps);
        return 1;
    }

    nob_log(NOB_INFO, "Building %s...", exe_name);

    nob_cmd_append(&cmd, CC);
    nob_cmd_append(&cmd, CFLAGS);
    nob_cmd_append(&cmd, "-o", out_path.items);
    nob_cmd_append(&cmd, src_file);

    // Add miniz.c if not embedded
    if (!is_miniz_embedded())
    {
        nob_cmd_append(&cmd, "miniz.c");
        nob_cmd_append(&cmd, "-DMINIZ_IMPLEMENTATION");
    }

    int ok = nob_cmd_run(&cmd);

    nob_cmd_free(cmd);
    nob_sb_free(out_path);
    nob_da_free(deps);

    return ok;
}

/**
 * Compile an example executable
 * 
 * Similar to compile_test_exe, but for example programs.
 * Examples are built in the example/ directory.
 * 
 * @param exe_name Name of the executable (without extension)
 * @param src_file Path to the source file
 * @return 1 on success, 0 on failure
 */
static int compile_example_exe(const char *exe_name, const char *src_file)
{
    Nob_Cmd cmd = { 0 };
    Nob_String_Builder out_path = { 0 };
#ifdef _WIN32
    nob_sb_appendf(&out_path, "%s%s.exe", EXAMPLE_DIR, exe_name);
#else
    nob_sb_appendf(&out_path, "%s%s", EXAMPLE_DIR, exe_name);
#endif

    Nob_File_Paths deps = { 0 };
    nob_da_append(&deps, src_file);
    nob_da_append(&deps, "stb_unpack.h");

    // Check if we need to rebuild
    if (!nob_needs_rebuild(out_path.items, deps.items, deps.count))
    {
        nob_log(NOB_INFO, "%s up to date", out_path.items);
        nob_sb_free(out_path);
        nob_da_free(deps);
        return 1;
    }

    nob_log(NOB_INFO, "Building %s...", exe_name);

    nob_cmd_append(&cmd, CC);
    nob_cmd_append(&cmd, CFLAGS);
    nob_cmd_append(&cmd, "-o", out_path.items);
    nob_cmd_append(&cmd, src_file);

    // Add miniz.c if not embedded
    if (!is_miniz_embedded())
    {
        nob_cmd_append(&cmd, "miniz.c");
        nob_cmd_append(&cmd, "-DMINIZ_IMPLEMENTATION");
    }

    int ok = nob_cmd_run(&cmd);

    nob_cmd_free(cmd);
    nob_sb_free(out_path);
    nob_da_free(deps);

    return ok;
}

int main(int argc, char **argv)
{
    // Auto-rebuild nob if nob.c or nob.h changed
    NOB_GO_REBUILD_URSELF(argc, argv);

    // Ensure required directories exist
    if (!nob_mkdir_if_not_exists(BUILD_DIR))
    {
        return 1;
    }
    if (!nob_mkdir_if_not_exists(EXAMPLE_DIR))
    {
        return 1;
    }

    // Parse command-line arguments
    nob_shift_args(&argc, &argv); // Skip program name

    int do_test = 0;      // Run tests flag
    int do_example = 0;   // Build example flag
    int do_clean = 0;     // Clean flag

    // Parse command-line arguments
    for (int i = 0; i < argc; i++)
    {
        const char *arg = argv[i];

        if (strcmp(arg, "clean") == 0)
        {
            do_clean = 1;
        }
        else if (strcmp(arg, "test") == 0 || strcmp(arg, "tests") == 0)
        {
            do_test = 1;
        }
        else if (strcmp(arg, "example") == 0 || strcmp(arg, "examples") == 0)
        {
            do_example = 1;
        }
        else if (strcmp(arg, "build") == 0)
        {
            // "build" command: just compile, don't run tests
            // This is handled by the build section below
        }
        else if (strcmp(arg, "help") == 0 || strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
        {
            nob_log(NOB_INFO, "stb_unpack.h Build System");
            nob_log(NOB_INFO, " ");
            nob_log(NOB_INFO, "Usage: %s [command]", argv[0]);
            nob_log(NOB_INFO, " ");
            nob_log(NOB_INFO, "Commands:");
            nob_log(NOB_INFO, "  (no args)  - Build all test programs and run tests");
            nob_log(NOB_INFO, "  build      - Build all test programs");
            nob_log(NOB_INFO, "  test       - Build and run all tests");
            nob_log(NOB_INFO, "  example    - Build example program");
            nob_log(NOB_INFO, "  clean      - Remove all build artifacts and test outputs");
            nob_log(NOB_INFO, "  help       - Show this help message");
            return 0;
        }
        else
        {
            nob_log(NOB_ERROR, "Unknown argument: %s", arg);
            nob_log(NOB_INFO, "Run '%s help' for usage information", argv[0]);
            return 1;
        }
    }

    if (do_clean)
    {
        nob_log(NOB_INFO, "Cleaning...");
        Nob_Cmd cmd = { 0 };
#ifdef _WIN32
        nob_cmd_append(&cmd, "rmdir", "/s", "/q", BUILD_DIR);
        nob_cmd_append(&cmd, "rmdir", "/s", "/q", "test/output");
        nob_cmd_append(&cmd, "del", "/f", "/q", EXAMPLE_DIR "extract_src.exe");
#else
        nob_cmd_append(&cmd, "rm", "-rf", BUILD_DIR);
        nob_cmd_run(&cmd);
        nob_cmd_free(cmd);
        cmd = (Nob_Cmd){ 0 };
        nob_cmd_append(&cmd, "rm", "-rf", "test/output");
        nob_cmd_run(&cmd);
        nob_cmd_free(cmd);
        cmd = (Nob_Cmd){ 0 };
        nob_cmd_append(&cmd, "rm", "-f", EXAMPLE_DIR "extract_src");
        nob_cmd_run(&cmd);
#endif
        nob_cmd_free(cmd);
        nob_log(NOB_INFO, "Clean complete.");
        return 0;
    }

    // Build all test programs
    if (!compile_test_exe("test", TEST_SRC_DIR "test.c"))
        return 1;
    if (!compile_test_exe("test_create", TEST_SRC_DIR "test_create.c"))
        return 1;
    if (!compile_test_exe("test_targz", TEST_SRC_DIR "test_targz.c"))
        return 1;
    if (!compile_test_exe("test_zip", TEST_SRC_DIR "test_zip.c"))
        return 1;
    if (!compile_test_exe("test_runner", TEST_SRC_DIR "test_runner.c"))
        return 1;

    // Build example program (if requested or if no args provided)
    if (do_example || argc == 0)
    {
        if (!compile_example_exe("extract_src", EXAMPLE_DIR "extract_src.c"))
            return 1;
    }

    // Run tests (if requested or if no args provided - default behavior)
    if (do_test || argc == 0)
    {
        // Ensure test output directory exists (tests create files here)
        if (!nob_mkdir_if_not_exists("test/output"))
        {
            nob_log(NOB_ERROR, "Failed to create test/output directory");
            return 1;
        }

        nob_log(NOB_INFO, "Running tests...");
        
        // Change to test directory before running tests
        // This is necessary because test_runner expects to run from test/ directory
        char *old_cwd = getcwd(NULL, 0);
        if (chdir("test") != 0) {
            nob_log(NOB_ERROR, "Failed to change to test directory");
            if (old_cwd) free(old_cwd);
            return 1;
        }
        
        // Run the test runner executable
        Nob_Cmd test_cmd = { 0 };
#ifdef _WIN32
        nob_cmd_append(&test_cmd, "build/test_runner.exe");
#else
        nob_cmd_append(&test_cmd, "build/test_runner");
#endif
        int test_result = nob_cmd_run(&test_cmd);
        nob_cmd_free(test_cmd);
        
        // Restore original working directory
        if (old_cwd) {
            chdir(old_cwd);
            free(old_cwd);
        }

        // Return failure if tests failed
        if (!test_result)
        {
            return 1;
        }
    }

    return 0;
}

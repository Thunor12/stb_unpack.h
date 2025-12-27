#define NOB_IMPLEMENTATION
#include "nob.h"

#ifdef _WIN32
#define CC "gcc.exe"
#else
#define CC "gcc"
#endif

#define BUILD_DIR "test/build/"
#define TEST_SRC_DIR "test/src/"
#define EXAMPLE_DIR "example/"

#define CFLAGS "-std=c99", "-Wall", "-Wextra", "-I.", "-D_POSIX_C_SOURCE=200809L"

// Check if miniz is embedded in stb_unpack.h
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

// Compile a test executable
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

// Compile example executable
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
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!nob_mkdir_if_not_exists(BUILD_DIR))
    {
        return 1;
    }
    if (!nob_mkdir_if_not_exists(EXAMPLE_DIR))
    {
        return 1;
    }

    nob_shift_args(&argc, &argv); // Ignore program name

    int do_test = 0;
    int do_example = 0;
    int do_clean = 0;

    // Parse arguments
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
            // Just build, don't run tests
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

    // Build example
    if (do_example || argc == 0)
    {
        if (!compile_example_exe("extract_src", EXAMPLE_DIR "extract_src.c"))
            return 1;
    }

    // Run tests if requested
    if (do_test || argc == 0)
    {
        // Ensure test output directory exists (tests need it)
        if (!nob_mkdir_if_not_exists("test/output"))
        {
            nob_log(NOB_ERROR, "Failed to create test/output directory");
            return 1;
        }

        nob_log(NOB_INFO, "Running tests...");
        Nob_String_Builder test_runner_path = {0};
        nob_sb_appendf(&test_runner_path, "%stest_runner", BUILD_DIR);
        
        // Change to test directory before running tests
        char *old_cwd = getcwd(NULL, 0);
        if (chdir("test") != 0) {
            nob_log(NOB_ERROR, "Failed to change to test directory");
            nob_sb_free(test_runner_path);
            if (old_cwd) free(old_cwd);
            return 1;
        }
        
        Nob_Cmd test_cmd = { 0 };
#ifdef _WIN32
        nob_cmd_append(&test_cmd, "build/test_runner.exe");
#else
        nob_cmd_append(&test_cmd, "build/test_runner");
#endif
        int test_result = nob_cmd_run(&test_cmd);
        nob_cmd_free(test_cmd);
        
        // Change back
        if (old_cwd) {
            chdir(old_cwd);
            free(old_cwd);
        }
        
        nob_sb_free(test_runner_path);

        if (!test_result)
        {
            return 1;
        }
    }

    return 0;
}

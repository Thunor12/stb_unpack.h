#define STB_UNPACK_IMPLEMENTATION
#include "../../stb_unpack.h"

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
#endif

// Check if file exists
static bool file_exists(const char *path) {
    return access(path, F_OK) == 0;
}

static void write_tar_header(stbup_tar_header *header, const char *name, uint64_t size) {
    memset(header, 0, sizeof(*header));
    strncpy(header->name, name, sizeof(header->name) - 1);
    stbup_u64_to_octal(header->mode, sizeof(header->mode), 0100644);
    stbup_u64_to_octal(header->uid, sizeof(header->uid), 0);
    stbup_u64_to_octal(header->gid, sizeof(header->gid), 0);
    stbup_u64_to_octal(header->size, sizeof(header->size), size);
    stbup_u64_to_octal(header->mtime, sizeof(header->mtime), 0);
    header->typeflag = '0';
    memcpy(header->magic, "ustar", 5);
    header->magic[5] = ' ';
    header->version[0] = ' ';
    header->version[1] = 0;

    unsigned int checksum = stbup_tar_checksum(header);
    char chksum_str[8];
    snprintf(chksum_str, sizeof(chksum_str), "%06o", checksum);
    memcpy(header->chksum, chksum_str, 6);
    header->chksum[6] = 0;
    header->chksum[7] = ' ';
}

static int create_zip_with_entry(const char *entry_name, const char *contents, size_t size, const char *zip_path) {
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));
    if (!mz_zip_writer_init_heap(&zip, 0, 0)) {
        return 0;
    }

    if (!mz_zip_writer_add_mem(&zip, entry_name, contents, size, MZ_DEFAULT_COMPRESSION)) {
        mz_zip_writer_end(&zip);
        return 0;
    }

    void *zip_data = NULL;
    size_t zip_size = 0;
    if (!mz_zip_writer_finalize_heap_archive(&zip, &zip_data, &zip_size)) {
        mz_zip_writer_end(&zip);
        return 0;
    }

    mz_zip_writer_end(&zip);

    FILE *f = fopen(zip_path, "wb");
    if (!f) {
        mz_free(zip_data);
        return 0;
    }
    int ok = (fwrite(zip_data, 1, zip_size, f) == zip_size);
    fclose(f);
    mz_free(zip_data);
    return ok;
}

/**
 * Security Test 1: Path Traversal in TAR
 * 
 * Tests that TAR archives with malicious paths (../, absolute paths) are rejected.
 */
static int test_tar_path_traversal(void) {
    unsigned char archive[1024] = {0};
    write_tar_header((stbup_tar_header *)archive, "../malicious.txt", 9);
    memcpy(archive + 512, "malicious", 9);

    stbup_mkdirs("output/security_test/tar_path");
    int result = stbup_tar_extract_stream(archive, sizeof(archive), "output/security_test/tar_path");
    bool wrote_inside = file_exists("output/security_test/tar_path/malicious.txt");

    if (result == 0 && !wrote_inside) {
        return 0;
    }
    return 1;
}

/**
 * Security Test 2: Path Traversal in ZIP
 * 
 * Tests that ZIP archives with malicious paths are rejected.
 * Note: This test verifies the behavior indirectly by checking that
 * files with malicious paths don't get extracted outside the output directory.
 */
static int test_zip_path_traversal(void) {
    stbup_mkdirs("output/security_test");
    const char *zip_path = "output/security_test/malicious.zip";
    const char *zip_out = "output/security_test/zip_out";
    stbup_mkdirs(zip_out);

    if (!create_zip_with_entry("../zip_evil.txt", "evil", 4, zip_path)) {
        return 1;
    }

    int result = stbup_zip_extract(zip_path, zip_out);
    remove(zip_path);

    if (result == 0 && !file_exists("output/security_test/zip_out/zip_evil.txt")) {
        return 0;
    }
    return 1;
}

/**
 * Security Test 3: Truncated TAR Entry
 * 
 * Tests that TAR extraction handles truncated entries safely (doesn't read past buffer).
 */
static int test_tar_truncated_entry(void) {
    unsigned char archive[1024] = {0};
    /* Claim a very large file but only provide one block */
    write_tar_header((stbup_tar_header *)archive, "truncated.txt", 1024 * 1024);
    memset(archive + 512, 'A', 512);

    int result = stbup_tar_extract_stream(archive, sizeof(archive), "output/security_test/truncated");
    return (result == 0) ? 0 : 1;
}

/**
 * Security Test 4: Corrupted GZIP Footer
 * 
 * Tests that gzip extraction validates the footer CRC32 and size.
 */
static int test_gzip_corrupted_footer(void) {
    // Create a valid gzip file first
    const char *test_data = "Hello, this is test data for gzip corruption test!";
    size_t test_data_size = strlen(test_data);
    
    void *compressed = NULL;
    size_t compressed_size = 0;
    
    if (!stbup_gzip_compress(test_data, test_data_size, &compressed, &compressed_size)) {
        return 1; // Failed to create test gzip
    }
    
    // Corrupt the CRC32 in the footer
    unsigned char *p = (unsigned char *)compressed;
    p[compressed_size - 8] ^= 0xFF; // Flip bits in CRC32
    
    // Try to decompress - should fail due to CRC32 mismatch
    void *decompressed = NULL;
    size_t decompressed_size = 0;
    int result = stbup_gzip_decompress(compressed, compressed_size, &decompressed, &decompressed_size);
    
    free(compressed);
    if (decompressed) free(decompressed);
    
    // Should fail (return 0) due to corrupted footer
    if (result == 0) {
        return 0; // Pass - corruption detected
    }
    
    return 1; // Fail - corruption not detected
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    printf("Security Tests\n");
    printf("==============\n\n");
    
    int passed = 0;
    int failed = 0;
    
    // Test 1: TAR path traversal
    printf("Testing TAR path traversal protection...\n");
    if (test_tar_path_traversal() == 0) {
        printf("✓ TAR Path Traversal Test: PASSED\n");
        passed++;
    } else {
        printf("✗ TAR Path Traversal Test: FAILED\n");
        failed++;
    }
    
    // Test 2: ZIP path traversal
    printf("Testing ZIP path traversal protection...\n");
    if (test_zip_path_traversal() == 0) {
        printf("✓ ZIP Path Traversal Test: PASSED\n");
        passed++;
    } else {
        printf("✗ ZIP Path Traversal Test: FAILED\n");
        failed++;
    }
    
    // Test 3: Truncated TAR entry
    printf("Testing truncated TAR entry handling...\n");
    if (test_tar_truncated_entry() == 0) {
        printf("✓ Truncated TAR Entry Test: PASSED\n");
        passed++;
    } else {
        printf("✗ Truncated TAR Entry Test: FAILED\n");
        failed++;
    }
    
    // Test 4: Corrupted gzip footer
    printf("Testing corrupted gzip footer detection...\n");
    if (test_gzip_corrupted_footer() == 0) {
        printf("✓ Corrupted GZIP Footer Test: PASSED\n");
        passed++;
    } else {
        printf("✗ Corrupted GZIP Footer Test: FAILED\n");
        failed++;
    }
    
    printf("\n");
    if (failed == 0) {
        printf("✓ All security tests passed! (%d/%d)\n", passed, passed + failed);
        return 0;
    } else {
        printf("✗ Some security tests failed! (%d passed, %d failed)\n", passed, failed);
        return 1;
    }
}


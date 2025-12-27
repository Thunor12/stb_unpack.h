#define STB_UNPACK_IMPLEMENTATION
#include "../../stb_unpack.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <archive.tar.gz> <out_dir>\n", argv[0]);
    fprintf(stderr, "   or: %s -c <archive.tar.gz> <file>\n", argv[0]);
    return 1;
  }

  if (argc == 4 && strcmp(argv[1], "-c") == 0) {
    /* Create mode */
    const char *archive_path = argv[2];
    const char *file_path = argv[3];

    printf("Creating .tar.gz archive: %s\n", archive_path);
    printf("Adding file: %s\n", file_path);

    if (!stbup_targz_create_file(archive_path, file_path)) {
      fprintf(stderr, "Error: Failed to create .tar.gz archive\n");
      return 1;
    }

    printf("Successfully created .tar.gz archive: %s\n", archive_path);
    return 0;
  } else {
    /* Extract mode */
    const char *archive_path = argv[1];
    const char *out_dir = argv[2];

    printf("Extracting .tar.gz archive: %s\n", archive_path);
    printf("To directory: %s\n", out_dir);

    if (!stbup_targz_extract(archive_path, out_dir)) {
      fprintf(stderr, "Error: Failed to extract .tar.gz archive\n");
      return 1;
    }

    printf("Successfully extracted .tar.gz archive to '%s' directory\n", out_dir);
    return 0;
  }
}


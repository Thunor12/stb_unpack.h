#define STB_UNPACK_IMPLEMENTATION
#include "stb_unpack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <archive.tar> <file1> [file2] ...\n", argv[0]);
    return 1;
  }

  const char *archive_path = argv[1];
  const char *file_path = argv[2];

  printf("Creating TAR archive: %s\n", archive_path);
  printf("Adding file: %s\n", file_path);

  if (!stbup_tar_create_file(archive_path, file_path)) {
    fprintf(stderr, "Error: Failed to create TAR archive\n");
    return 1;
  }

  printf("Successfully created TAR archive: %s\n", archive_path);
  return 0;
}


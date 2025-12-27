#define STB_UNPACK_IMPLEMENTATION
#include "../stb_unpack.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  const char *archive_path = "raylib-5.5_linux_amd64.tar.gz";
  const char *out_dir = "src";
  
  if (argc > 1) {
    archive_path = argv[1];
  }
  if (argc > 2) {
    out_dir = argv[2];
  }
  
  printf("Extracting %s to %s...\n", archive_path, out_dir);
  
  if (!stbup_targz_extract(archive_path, out_dir)) {
    fprintf(stderr, "Error: Failed to extract archive\n");
    return 1;
  }
  
  printf("Successfully extracted archive to '%s' directory\n", out_dir);
  return 0;
}


#define STB_UNPACK_IMPLEMENTATION
#include "../stb_unpack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <archive.tar.gz>\n", argv[0]);
    return 1;
  }

  const char *archive_path = argv[1];
  
  void *compressed_data = NULL;
  size_t compressed_size = 0;
  if (!stbup_read_file(archive_path, &compressed_data, &compressed_size)) {
    fprintf(stderr, "Failed to read file\n");
    return 1;
  }
  
  printf("Compressed size: %zu bytes\n", compressed_size);
  printf("First 20 bytes: ");
  for (int i = 0; i < 20 && i < (int)compressed_size; i++) {
    printf("%02x ", ((unsigned char *)compressed_data)[i]);
  }
  printf("\n");
  
  void *decompressed = NULL;
  size_t decompressed_size = 0;
  
  if (stbup_gzip_decompress(compressed_data, compressed_size, &decompressed, &decompressed_size)) {
    printf("Decompressed successfully! Size: %zu bytes\n", decompressed_size);
    printf("First 20 bytes of decompressed: ");
    for (int i = 0; i < 20 && i < (int)decompressed_size; i++) {
      printf("%02x ", ((unsigned char *)decompressed)[i]);
    }
    printf("\n");
    free(decompressed);
  } else {
    printf("Decompression failed!\n");
  }
  
  free(compressed_data);
  return 0;
}


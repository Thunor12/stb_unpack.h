#define STB_UNPACK_IMPLEMENTATION
#include "../stb_unpack.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file>\n", argv[0]);
    return 1;
  }

  const char *file_path = argv[1];
  
  void *file_data = NULL;
  size_t file_size = 0;
  if (!stbup_read_file(file_path, &file_data, &file_size)) {
    fprintf(stderr, "Failed to read file\n");
    return 1;
  }
  
  printf("File size: %zu bytes\n", file_size);
  printf("First 30 bytes: ");
  for (int i = 0; i < 30 && i < (int)file_size; i++) {
    printf("%02x ", ((unsigned char *)file_data)[i]);
  }
  printf("\n");
  
  /* Create TAR in memory */
  size_t tar_data_size = 512 + ((file_size + 511) & ~511) + 1024;
  void *tar_data = malloc(tar_data_size);
  memset(tar_data, 0, tar_data_size);
  
  stbup_tar_header *h = (stbup_tar_header *)tar_data;
  memcpy(h->name, "test.txt", 8);
  stbup_u64_to_octal(h->size, sizeof(h->size), file_size);
  h->typeflag = '0';
  memcpy(h->magic, "ustar", 5);
  h->magic[5] = ' ';
  h->version[0] = ' ';
  h->version[1] = 0;
  
  unsigned int checksum = stbup_tar_checksum(h);
  char chksum_str[8];
  snprintf(chksum_str, sizeof(chksum_str), "%06o", checksum);
  memcpy(h->chksum, chksum_str, 6);
  h->chksum[6] = 0;
  h->chksum[7] = ' ';
  
  memcpy((unsigned char *)tar_data + 512, file_data, file_size);
  
  printf("\nTAR data first 30 bytes: ");
  for (int i = 0; i < 30 && i < (int)tar_data_size; i++) {
    printf("%02x ", ((unsigned char *)tar_data)[i]);
  }
  printf("\n");
  
  free(file_data);
  free(tar_data);
  return 0;
}


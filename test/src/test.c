#define STB_UNPACK_IMPLEMENTATION
#include "../../stb_unpack.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    FILE *f = fopen("output/archive.tar", "rb");
    if (!f)
    {
        fprintf(stderr, "Error: Could not open output/archive.tar\n");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    size_t sz = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (sz == 0)
    {
        fprintf(stderr, "Error: output/archive.tar is empty\n");
        fclose(f);
        return 1;
    }

    void *buf = malloc(sz);
    if (!buf)
    {
        fprintf(stderr, "Error: Could not allocate memory\n");
        fclose(f);
        return 1;
    }

    size_t read = fread(buf, 1, sz, f);
    fclose(f);

    if (read != sz)
    {
        fprintf(stderr, "Error: Could not read entire file\n");
        free(buf);
        return 1;
    }

    printf("Archive size: %zu bytes\n", sz);

    if (!stbup_tar_extract_stream(buf, sz, "output/out"))
    {
        fprintf(stderr, "Error: Failed to extract archive\n");
        free(buf);
        return 1;
    }

    printf("Successfully extracted archive to 'output/out' directory\n");
    free(buf);
    return 0;
}

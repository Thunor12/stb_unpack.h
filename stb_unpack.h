/*
------------------------------------------------------------------------------
   stb_unpack.h - stb-style archive extraction helper (WIP)

   Public domain / MIT-0

   Phase 1:
   - Arena allocator
   - Portable filesystem helpers
   - Streaming TAR parser

   Phase 2:
   - gzip (miniz)
   - zip (miniz)

------------------------------------------------------------------------------
*/

#ifndef STB_UNPACK_H
#define STB_UNPACK_H

#include <stddef.h>
#include <stdint.h>

/* ============================================================
   Configuration
   ============================================================ */

#ifndef STBUP_ASSERT
#include <assert.h>
#define STBUP_ASSERT(x) assert(x)
#endif

#ifndef STBUP_ARENA_ALLOC
#include <stdlib.h>
#define STBUP_ARENA_ALLOC(sz) malloc(sz)
#define STBUP_ARENA_FREE(p) free(p)
#endif

#ifndef STBUP_PATH_MAX
#define STBUP_PATH_MAX 1024
#endif

/* ============================================================
   Arena allocator
   ============================================================ */

typedef struct stbup_arena {
  unsigned char *base;
  size_t size;
  size_t used;
  int owns_memory;
} stbup_arena;

static stbup_arena stbup_make_arena(void *buffer, size_t size) {
  stbup_arena a;
  a.base = (unsigned char *)buffer;
  a.size = size;
  a.used = 0;
  a.owns_memory = 0;
  return a;
}

static stbup_arena stbup_make_heap_arena(size_t size) {
  stbup_arena a;
  a.base = (unsigned char *)STBUP_ARENA_ALLOC(size);
  a.size = size;
  a.used = 0;
  a.owns_memory = 1;
  return a;
}

static void stbup_free_arena(stbup_arena *a) {
  if (a->owns_memory && a->base)
    STBUP_ARENA_FREE(a->base);
  a->base = NULL;
  a->size = a->used = 0;
}

static void *stbup_arena_alloc(stbup_arena *a, size_t sz) {
  sz = (sz + 7) & ~7; /* align */
  if (a->used + sz > a->size)
    return NULL;
  void *p = a->base + a->used;
  a->used += sz;
  return p;
}

static void stbup_arena_reset(stbup_arena *a) { a->used = 0; }

/* ============================================================
   Platform filesystem layer
   ============================================================ */

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <direct.h>
#include <windows.h>

static int stbup_mkdir(const char *path) {
  if (_mkdir(path) == 0)
    return 1;
  return GetLastError() == ERROR_ALREADY_EXISTS;
}

#else
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

static int stbup_mkdir(const char *path) {
  if (mkdir(path, 0755) == 0)
    return 1;
  return access(path, F_OK) == 0;
}
#endif

#ifdef _WIN32
#include <sys/stat.h>
#endif

#include <stdio.h>
#include <string.h>

/* mkdir -p */
static int stbup_mkdirs(const char *path) {
  char tmp[STBUP_PATH_MAX];
  size_t len = strlen(path);
  if (len >= sizeof(tmp))
    return 0;

  memcpy(tmp, path, len + 1);

  for (char *p = tmp + 1; *p; p++) {
    if (*p == '/' || *p == '\\') {
      char c = *p;
      *p = 0;
      if (!stbup_mkdir(tmp))
        return 0;
      *p = c;
    }
  }
  return stbup_mkdir(tmp);
}

/* get directory part of path */
static void stbup_dirname(char *path) {
  char *last_slash = NULL;
  char *last_backslash = NULL;
  for (char *p = path; *p; p++) {
    if (*p == '/') last_slash = p;
    if (*p == '\\') last_backslash = p;
  }
  char *last_sep = (last_slash > last_backslash) ? last_slash : last_backslash;
  if (last_sep)
    *last_sep = 0;
  else
    path[0] = 0;
}

static int stbup_write_file(const char *path, const void *data, size_t size) {
  FILE *f = fopen(path, "wb");
  if (!f)
    return 0;
  fwrite(data, 1, size, f);
  fclose(f);
  return 1;
}

/* ============================================================
   TAR format
   ============================================================ */

typedef struct {
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char chksum[8];
  char typeflag;
  char linkname[100];
  char magic[6];
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
} stbup_tar_header;

/* octal ASCII → integer */
static uint64_t stbup_octal_to_u64(const char *s, size_t n) {
  uint64_t v = 0;
  while (n-- && *s) {
    if (*s >= '0' && *s <= '7')
      v = (v << 3) + (*s - '0');
    else if (*s != ' ' && *s != '\0')
      break; /* stop at non-octal, non-padding chars */
    s++;
  }
  return v;
}

/* integer → octal ASCII (null-terminated with leading zeros, TAR format) */
static void stbup_u64_to_octal(char *dst, size_t dst_size, uint64_t v) {
  /* TAR format: null-terminated octal with leading zeros, no trailing spaces */
  char tmp[32];
  int i = sizeof(tmp) - 1;
  tmp[i] = 0;
  
  if (v == 0) {
    tmp[--i] = '0';
  } else {
    while (v > 0 && i > 0) {
      tmp[--i] = '0' + (v & 7);
      v >>= 3;
    }
  }
  
  /* Pad with leading zeros to fill the field, then null-terminate */
  int len = sizeof(tmp) - i - 1;
  int pad = (int)dst_size - 1 - len; /* -1 for null terminator */
  if (pad < 0) pad = 0;
  
  memset(dst, '0', pad);
  memcpy(dst + pad, tmp + i, len);
  dst[dst_size - 1] = 0;
}

/* calculate TAR header checksum */
static unsigned int stbup_tar_checksum(const stbup_tar_header *h) {
  unsigned int sum = 0;
  const unsigned char *p = (const unsigned char *)h;
  size_t struct_size = sizeof(stbup_tar_header);
  /* Calculate over 512 bytes: struct (500 bytes) + 12 bytes padding (zeros) */
  for (int i = 0; i < 512; i++) {
    if (i >= 148 && i < 156) { /* checksum field itself (bytes 148-155) - treat as all spaces */
      sum += ' ';
    } else if (i < (int)struct_size) {
      sum += p[i]; /* Read from struct */
    } else {
      /* Padding bytes (after struct, bytes 500-511) - zeros */
      sum += 0;
    }
  }
  return sum;
}

/* ============================================================
   TAR streaming extractor
   ============================================================ */

typedef struct {
  FILE *out;
  char out_dir[STBUP_PATH_MAX];
} stbup_tar_ctx;

static int stbup_tar_extract_stream(const void *tar_data, size_t tar_size,
                                    const char *out_dir) {
  const unsigned char *p = (const unsigned char *)tar_data;
  const unsigned char *end = p + tar_size;

  while (p + 512 <= end) {
    const stbup_tar_header *h = (const stbup_tar_header *)p;

    /* end of archive = two zero blocks */
    int empty = 1;
    for (int i = 0; i < 512; i++) {
      if (p[i] != 0) {
        empty = 0;
        break;
      }
    }
    if (empty)
      break;

    uint64_t size = stbup_octal_to_u64(h->size, sizeof(h->size));

    /* trim trailing spaces/null from name and prefix */
    char name[101];
    char prefix[156];
    size_t name_len = 0;
    size_t prefix_len = 0;
    
    for (int i = 0; i < 100 && h->name[i] && h->name[i] != ' '; i++)
      name[name_len++] = h->name[i];
    name[name_len] = 0;
    
    for (int i = 0; i < 155 && h->prefix[i] && h->prefix[i] != ' '; i++)
      prefix[prefix_len++] = h->prefix[i];
    prefix[prefix_len] = 0;

    if (name_len == 0)
      continue; /* skip entries with no name */

    char fullpath[STBUP_PATH_MAX];
    if (prefix_len > 0) {
      snprintf(fullpath, sizeof(fullpath), "%s/%s/%s", out_dir, prefix, name);
    } else {
      snprintf(fullpath, sizeof(fullpath), "%s/%s", out_dir, name);
    }

    if (h->typeflag == '5') {
      /* directory */
      stbup_mkdirs(fullpath);
    } else if (h->typeflag == '0' || h->typeflag == '\0') {
      /* regular file */
      char dirpath[STBUP_PATH_MAX];
      memcpy(dirpath, fullpath, sizeof(dirpath));
      stbup_dirname(dirpath);
      if (dirpath[0] && !stbup_mkdirs(dirpath))
        continue; /* skip if we can't create parent dir */
      stbup_write_file(fullpath, p + 512, (size_t)size);
    }

    /* advance to next header */
    size_t skip = 512 + ((size + 511) & ~511);
    p += skip;
  }

  return 1;
}

/* ============================================================
   TAR creator
   ============================================================ */

static int stbup_read_file(const char *path, void **data, size_t *size) {
  FILE *f = fopen(path, "rb");
  if (!f)
    return 0;
  
  fseek(f, 0, SEEK_END);
  long sz = ftell(f);
  fseek(f, 0, SEEK_SET);
  
  if (sz < 0) {
    fclose(f);
    return 0;
  }
  
  void *buf = malloc((size_t)sz);
  if (!buf) {
    fclose(f);
    return 0;
  }
  
  if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
    free(buf);
    fclose(f);
    return 0;
  }
  
  fclose(f);
  *data = buf;
  *size = (size_t)sz;
  return 1;
}

/* Create a TAR archive from a single file (equivalent to tar cf) */
static int stbup_tar_create_file(const char *archive_path, const char *file_path) {
  FILE *out = fopen(archive_path, "wb");
  if (!out)
    return 0;
  
  void *file_data = NULL;
  size_t file_size = 0;
  if (!stbup_read_file(file_path, &file_data, &file_size)) {
    fclose(out);
    return 0;
  }
  
  /* Extract filename from path */
  const char *filename = file_path;
  const char *last_slash = strrchr(file_path, '/');
  const char *last_backslash = strrchr(file_path, '\\');
  if (last_slash || last_backslash) {
    const char *last_sep = (last_slash > last_backslash) ? last_slash : last_backslash;
    filename = last_sep + 1;
  }
  
  size_t name_len = strlen(filename);
  if (name_len > 100) {
    /* Use prefix for long names (simplified: just truncate for now) */
    name_len = 100;
  }
  
  /* Create TAR header */
  stbup_tar_header h;
  memset(&h, 0, sizeof(h));
  
  memcpy(h.name, filename, name_len);
  
  /* Get file stats for mode, uid, gid, mtime */
  uint64_t mode = 0644;
  uint64_t uid = 0;
  uint64_t gid = 0;
  uint64_t mtime = 0;
  
#ifdef _WIN32
  /* On Windows, use default values */
  struct _stat st;
  if (_stat(file_path, &st) == 0) {
    mode = (st.st_mode & 0777) | 0100000; /* regular file with permissions */
    uid = 0;
    gid = 0;
    mtime = (uint64_t)st.st_mtime;
  } else {
    mode = 0100644; /* default: regular file, rw-r--r-- */
  }
#else
  struct stat st;
  if (stat(file_path, &st) == 0) {
    mode = st.st_mode & 0777; /* just permissions, file type is in typeflag */
    uid = st.st_uid;
    gid = st.st_gid;
    mtime = (uint64_t)st.st_mtime;
  } else {
    mode = 0644; /* default: rw-r--r-- */
  }
#endif
  
  stbup_u64_to_octal(h.mode, sizeof(h.mode), mode);
  stbup_u64_to_octal(h.uid, sizeof(h.uid), uid);
  stbup_u64_to_octal(h.gid, sizeof(h.gid), gid);
  stbup_u64_to_octal(h.size, sizeof(h.size), file_size);
  stbup_u64_to_octal(h.mtime, sizeof(h.mtime), mtime);
  h.typeflag = '0'; /* regular file */
  memcpy(h.magic, "ustar", 5); /* "ustar" (5 bytes) */
  h.magic[5] = ' '; /* space (6th byte) */
  h.version[0] = ' '; /* space (1st byte) */
  h.version[1] = 0; /* null (2nd byte) */
  
  /* Get username and groupname */
#ifndef _WIN32
  struct passwd *pw = getpwuid(uid);
  if (pw) {
    size_t uname_len = strlen(pw->pw_name);
    if (uname_len > 31) uname_len = 31;
    memcpy(h.uname, pw->pw_name, uname_len);
  }
  struct group *gr = getgrgid(gid);
  if (gr) {
    size_t gname_len = strlen(gr->gr_name);
    if (gname_len > 31) gname_len = 31;
    memcpy(h.gname, gr->gr_name, gname_len);
  }
#endif
  
  /* Calculate and write checksum (must be done after other fields are set) */
  unsigned int checksum = stbup_tar_checksum(&h);
  /* Checksum field: 6 octal digits + null + space (8 bytes total) */
  char chksum_str[8];
  snprintf(chksum_str, sizeof(chksum_str), "%06o", checksum); /* 6 digits */
  memcpy(h.chksum, chksum_str, 6);
  h.chksum[6] = 0; /* null */
  h.chksum[7] = ' '; /* space */
  
  /* Write header (must be exactly 512 bytes) */
  /* TAR header struct is 500 bytes, need 12 bytes padding to make 512 */
  if (fwrite(&h, 1, sizeof(h), out) != sizeof(h)) {
    free(file_data);
    fclose(out);
    return 0;
  }
  /* Write 12 bytes of zero padding to make header exactly 512 bytes */
  char padding[12] = {0};
  if (fwrite(padding, 1, 12, out) != 12) {
    free(file_data);
    fclose(out);
    return 0;
  }
  
  /* Write file data */
  if (fwrite(file_data, 1, file_size, out) != file_size) {
    free(file_data);
    fclose(out);
    return 0;
  }
  
  /* Pad to 512-byte boundary */
  size_t pad = (512 - (file_size % 512)) % 512;
  if (pad > 0) {
    char zeros[512] = {0};
    if (fwrite(zeros, 1, pad, out) != pad) {
      free(file_data);
      fclose(out);
      return 0;
    }
  }
  
  /* Write two zero blocks (end of archive) */
  char zeros[1024] = {0};
  if (fwrite(zeros, 1, 1024, out) != 1024) {
    free(file_data);
    fclose(out);
    return 0;
  }
  
  free(file_data);
  fclose(out);
  return 1;
}

#endif /* STB_UNPACK_H */

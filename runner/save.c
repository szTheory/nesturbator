#define _POSIX_C_SOURCE 200809L
#include "save.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

int nesturbator_run_save_path(char *buf, size_t cap, const char *dir, const char *rom)
{
    const char *name = rom;
    size_t stem;
    size_t dir_len = strlen(dir);
    for (const char *p = rom; *p != '\0'; p++) {
        if (*p == '/' || *p == '\\') {
            name = p + 1;
        }
    }
    stem = strlen(name);
    for (size_t k = stem; k > 1u; k--) {
        if (name[k - 1u] == '.') {
            stem = k - 1u;
            break;
        }
    }
    /* dir + '/' + stem + ".sav" + NUL */
    if (dir_len + 1u + stem + 4u + 1u > cap) {
        return 0;
    }
    memcpy(buf, dir, dir_len);
    buf[dir_len] = '/';
    memcpy(buf + dir_len + 1u, name, stem);
    memcpy(buf + dir_len + 1u + stem, ".sav", 5u);
    return 1;
}

enum nesturbator_run_save_status nesturbator_run_save_load(const char *path, uint8_t *span,
                                                           size_t size)
{
    FILE *f = fopen(path, "rb");
    long length;
    if (f == NULL) {
        if (errno == ENOENT) {
            return NESTURBATOR_RUN_SAVE_FRESH;
        }
        fprintf(stderr, "nesturbator-run: cannot read %s\n", path);
        return NESTURBATOR_RUN_SAVE_ERROR;
    }
    if (fseek(f, 0, SEEK_END) != 0 || (length = ftell(f)) < 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        fprintf(stderr, "nesturbator-run: cannot read %s\n", path);
        return NESTURBATOR_RUN_SAVE_ERROR;
    }
    if ((unsigned long)length != (unsigned long)size) {
        fclose(f);
        fprintf(stderr,
                "nesturbator-run: %s is %lu bytes; the cartridge's battery RAM is %lu bytes\n",
                path, (unsigned long)length, (unsigned long)size);
        return NESTURBATOR_RUN_SAVE_MISMATCH;
    }
    if (fread(span, 1, size, f) != size) {
        fclose(f);
        fprintf(stderr, "nesturbator-run: cannot read %s\n", path);
        return NESTURBATOR_RUN_SAVE_ERROR;
    }
    fclose(f);
    return NESTURBATOR_RUN_SAVE_LOADED;
}

int nesturbator_run_save_flush(const char *path, const uint8_t *span, size_t size, uint8_t *shadow)
{
    char tmp[4096];
    FILE *f;
    int ok;
    if (snprintf(tmp, sizeof tmp, "%s.tmp", path) >= (int)sizeof tmp) {
        fprintf(stderr, "nesturbator-run: cannot write %s.tmp\n", path);
        return 1;
    }
    f = fopen(tmp, "wb");
    if (f == NULL) {
        fprintf(stderr, "nesturbator-run: cannot write %s\n", tmp);
        return 1;
    }
    ok = fwrite(span, 1, size, f) == size && fflush(f) == 0;
#ifdef _WIN32
    ok = ok && _commit(_fileno(f)) == 0;
#else
    ok = ok && fsync(fileno(f)) == 0;
#endif
    if (fclose(f) != 0) {
        ok = 0;
    }
    if (!ok) {
        fprintf(stderr, "nesturbator-run: cannot write %s\n", tmp);
        return 1;
    }
#ifdef _WIN32
    ok = MoveFileExA(tmp, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    ok = rename(tmp, path) == 0;
#endif
    if (!ok) {
        fprintf(stderr, "nesturbator-run: cannot replace %s; the new save is in %s\n", path, tmp);
        return 1;
    }
    memcpy(shadow, span, size);
    return 0;
}

#define _POSIX_C_SOURCE 200809L
#ifdef __APPLE__
#define _DARWIN_C_SOURCE /* exposes F_FULLFSYNC */
#endif
#include "save.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#ifndef S_ISDIR
#define S_ISDIR(m) (((m)&_S_IFMT) == _S_IFDIR)
#endif
#define stat _stat
#define lstat _stat
#else
#include <fcntl.h>
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

/* One stderr line for a failed save; `winerr` is a GetLastError() code, or 0 to use errno. */
static void report_failure(const char *path, int err, unsigned long winerr)
{
    if (winerr != 0u) {
        fprintf(stderr, "nesturbator-run: cannot save %s (Windows error %lu); the previous save is kept\n",
                path, winerr);
    } else {
        fprintf(stderr, "nesturbator-run: cannot save %s (%s); the previous save is kept\n", path,
                strerror(err));
    }
}

static int sync_stream(FILE *f)
{
#ifdef _WIN32
    return _commit(_fileno(f));
#elif defined(F_FULLFSYNC)
    /* Apple documents that fsync() does not flush the drive's write cache; F_FULLFSYNC does
       (fcntl(2), fsync(2) on macOS). Fall back to fsync() where the filesystem refuses it. */
    if (fcntl(fileno(f), F_FULLFSYNC) == 0) {
        return 0;
    }
    return fsync(fileno(f));
#else
    return fsync(fileno(f));
#endif
}

int nesturbator_run_file_replace(const char *path, const uint8_t *data, size_t size)
{
    char tmp[4096];
    FILE *f;
    struct stat st;
    int err = 0;
    unsigned long winerr = 0u;
    int ok;
    if (snprintf(tmp, sizeof tmp, "%s.tmp", path) >= (int)sizeof tmp) {
        report_failure(path, ENAMETOOLONG, 0u);
        return 1;
    }
    /* lstat: a symlink here (even a dangling one) is judged as a link, never followed. */
    if (lstat(tmp, &st) == 0) {
        /* A directory here is not ours: refuse it (POSIX remove() would delete an empty one). */
        if (S_ISDIR(st.st_mode)) {
            report_failure(path, EISDIR, 0u);
            return 1;
        }
        /* A regular file or a symlink is a leftover; remove() unlinks a link, not its target. */
        (void)remove(tmp);
    }
    f = fopen(tmp, "wbx");
    if (f == NULL) {
        /* Name the temporary file: it is what blocks the save. */
        report_failure(tmp, errno, 0u);
        return 1;
    }
    ok = fwrite(data, 1, size, f) == size;
    if (!ok) {
        err = errno;
    }
    if (ok && fflush(f) != 0) {
        ok = 0;
        err = errno;
    }
    if (ok && sync_stream(f) != 0) {
        ok = 0;
        err = errno;
    }
    if (fclose(f) != 0 && ok) {
        ok = 0;
        err = errno;
    }
    if (ok) {
#ifdef _WIN32
        if (MoveFileExA(tmp, path, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == 0) {
            ok = 0;
            winerr = (unsigned long)GetLastError();
        }
#else
        if (rename(tmp, path) != 0) {
            ok = 0;
            err = errno;
        }
#endif
    }
    if (!ok) {
        (void)remove(tmp);
        report_failure(path, err, winerr);
        return 1;
    }
    return 0;
}

int nesturbator_run_save_flush(const char *path, const uint8_t *span, size_t size, uint8_t *shadow)
{
    if (nesturbator_run_file_replace(path, span, size) != 0) {
        return 1;
    }
    memcpy(shadow, span, size);
    return 0;
}

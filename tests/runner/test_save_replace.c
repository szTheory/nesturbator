/* The runner's atomic file replace (TUNE-07, D-06): every failure leaves the
   old file intact and no .tmp behind, and a directory at .tmp is never
   removed, and a symlink at .tmp is unlinked without writing through it.
   argv[1] is a scratch base directory; each case uses its own subdirectory. */
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#define STAT _stat
#define IS_DIR(m) (((m)&_S_IFMT) == _S_IFDIR)
#else
#include <unistd.h>
#define MKDIR(p) mkdir((p), 0755)
#define STAT stat
#define IS_DIR(m) S_ISDIR(m)
#endif

#include "save.h"
#include "../check.h"

#define DATA_SIZE 8192u

static char base[1024];

static void make_dir(const char *p)
{
    (void)MKDIR(p);
}

static void write_file(const char *p, const uint8_t *d, size_t n)
{
    FILE *f = fopen(p, "wb");
    if (f != NULL) {
        (void)fwrite(d, 1, n, f);
        fclose(f);
    }
}

static int read_equals(const char *p, const uint8_t *d, size_t n)
{
    static uint8_t buf[DATA_SIZE + 16u];
    FILE *f = fopen(p, "rb");
    size_t got;
    if (f == NULL) {
        return 0;
    }
    got = fread(buf, 1, sizeof buf, f);
    fclose(f);
    return got == n && memcmp(buf, d, n) == 0;
}

static int path_exists(const char *p)
{
    struct STAT st;
    return STAT(p, &st) == 0;
}

static int is_dir(const char *p)
{
    struct STAT st;
    return STAT(p, &st) == 0 && IS_DIR(st.st_mode);
}

static void fill(uint8_t *d, uint8_t seed)
{
    for (size_t i = 0; i < DATA_SIZE; i++) {
        d[i] = (uint8_t)(seed + i * 7u + (i >> 8));
    }
}

/* Makes <base>/<name> and puts "<case>/<leaf>" into out. */
static void case_dir(char *dir, size_t cap, const char *name)
{
    snprintf(dir, cap, "%s/%s", base, name);
    make_dir(dir);
}

int main(int argc, char **argv)
{
    static uint8_t old_data[DATA_SIZE];
    static uint8_t new_data[DATA_SIZE];
    char dir[1100];
    char target[1200];
    char tmp[1210];
    char other[1300];
    if (argc < 2) {
        fprintf(stderr, "usage: test_save_replace SCRATCH_DIR\n");
        return 2;
    }
    snprintf(base, sizeof base, "%s", argv[1]);
    make_dir(base);
    fill(old_data, 1u);
    fill(new_data, 99u);

    /* (a) happy path replaces the bytes and leaves no .tmp. */
    case_dir(dir, sizeof dir, "happy");
    snprintf(target, sizeof target, "%s/a.sav", dir);
    snprintf(tmp, sizeof tmp, "%s.tmp", target);
    write_file(target, old_data, DATA_SIZE);
    CHECK(nesturbator_run_file_replace(target, new_data, DATA_SIZE) == 0);
    CHECK(read_equals(target, new_data, DATA_SIZE));
    CHECK(!path_exists(tmp));

    /* (b) a directory at the target path: refused, directory remains, no .tmp. */
    case_dir(dir, sizeof dir, "dir_target");
    snprintf(target, sizeof target, "%s/a.sav", dir);
    snprintf(tmp, sizeof tmp, "%s.tmp", target);
    make_dir(target);
    CHECK(nesturbator_run_file_replace(target, new_data, DATA_SIZE) == 1);
    CHECK(is_dir(target));
    CHECK(!path_exists(tmp));

    /* (c) a missing parent directory: nothing is created. */
    case_dir(dir, sizeof dir, "no_parent");
    snprintf(target, sizeof target, "%s/missing/a.sav", dir);
    snprintf(tmp, sizeof tmp, "%s.tmp", target);
    CHECK(nesturbator_run_file_replace(target, new_data, DATA_SIZE) == 1);
    CHECK(!path_exists(target));
    CHECK(!path_exists(tmp));

    /* (d) a non-empty directory at .tmp: refused, old bytes, directory and content untouched. */
    case_dir(dir, sizeof dir, "dir_tmp");
    snprintf(target, sizeof target, "%s/a.sav", dir);
    snprintf(tmp, sizeof tmp, "%s.tmp", target);
    snprintf(other, sizeof other, "%s/inner", tmp);
    write_file(target, old_data, DATA_SIZE);
    make_dir(tmp);
    write_file(other, old_data, 16u);
    CHECK(nesturbator_run_file_replace(target, new_data, DATA_SIZE) == 1);
    CHECK(read_equals(target, old_data, DATA_SIZE));
    CHECK(is_dir(tmp));
    CHECK(read_equals(other, old_data, 16u));

    /* (d2) empty directory at .tmp: POSIX remove() would delete it, so the helper must not. */
    case_dir(dir, sizeof dir, "dir_tmp_empty");
    snprintf(target, sizeof target, "%s/a.sav", dir);
    snprintf(tmp, sizeof tmp, "%s.tmp", target);
    write_file(target, old_data, DATA_SIZE);
    make_dir(tmp);
    CHECK(nesturbator_run_file_replace(target, new_data, DATA_SIZE) == 1);
    CHECK(is_dir(tmp));
    CHECK(read_equals(target, old_data, DATA_SIZE));

    /* (e) a stale .tmp file from a killed write is replaced; the new bytes win. */
    case_dir(dir, sizeof dir, "stale");
    snprintf(target, sizeof target, "%s/a.sav", dir);
    snprintf(tmp, sizeof tmp, "%s.tmp", target);
    write_file(target, old_data, DATA_SIZE);
    write_file(tmp, old_data, 33u);
    CHECK(nesturbator_run_file_replace(target, new_data, DATA_SIZE) == 0);
    CHECK(read_equals(target, new_data, DATA_SIZE));
    CHECK(!path_exists(tmp));

    /* (f) a regular file used as a parent path component (ENOTDIR, also for root). */
    case_dir(dir, sizeof dir, "notdir");
    snprintf(other, sizeof other, "%s/afile", dir);
    snprintf(target, sizeof target, "%s/afile/x.sav", dir);
    snprintf(tmp, sizeof tmp, "%s.tmp", target);
    write_file(other, old_data, 20u);
    CHECK(nesturbator_run_file_replace(target, new_data, DATA_SIZE) == 1);
    CHECK(read_equals(other, old_data, 20u));
    CHECK(!path_exists(tmp));

#ifndef _WIN32
    /* (g) chmod 0555 on the directory: the write fails, or (root) the probe shows it is allowed. */
    case_dir(dir, sizeof dir, "chmod");
    snprintf(target, sizeof target, "%s/a.sav", dir);
    snprintf(tmp, sizeof tmp, "%s.tmp", target);
    snprintf(other, sizeof other, "%s/probe", dir);
    write_file(target, old_data, DATA_SIZE);
    CHECK(chmod(dir, 0555) == 0);
    {
        FILE *probe = fopen(other, "wb");
        if (probe != NULL) {
            fclose(probe);
            remove(other);
            printf("note: chmod 0555 does not block this user; the chmod sub-case passes\n");
        } else {
            int rc = nesturbator_run_file_replace(target, new_data, DATA_SIZE);
            CHECK(rc == 1);
            CHECK(read_equals(target, old_data, DATA_SIZE));
            CHECK(!path_exists(tmp));
        }
    }
    CHECK(chmod(dir, 0755) == 0);

    /* (h) a symlink at .tmp pointing at a victim file: the victim is never written through. */
    case_dir(dir, sizeof dir, "symlink");
    snprintf(target, sizeof target, "%s/a.sav", dir);
    snprintf(tmp, sizeof tmp, "%s.tmp", target);
    snprintf(other, sizeof other, "%s/victim", dir);
    write_file(target, old_data, DATA_SIZE);
    write_file(other, old_data, 64u);
    CHECK(symlink(other, tmp) == 0);
    CHECK(nesturbator_run_file_replace(target, new_data, DATA_SIZE) == 0);
    CHECK(read_equals(other, old_data, 64u));
    CHECK(read_equals(target, new_data, DATA_SIZE));
    CHECK(!path_exists(tmp));

    /* (i) a dangling symlink at .tmp: cleared like any stale leftover, the save succeeds. */
    case_dir(dir, sizeof dir, "symlink_dangling");
    snprintf(target, sizeof target, "%s/a.sav", dir);
    snprintf(tmp, sizeof tmp, "%s.tmp", target);
    snprintf(other, sizeof other, "%s/nowhere", dir);
    write_file(target, old_data, DATA_SIZE);
    CHECK(symlink(other, tmp) == 0);
    CHECK(nesturbator_run_file_replace(target, new_data, DATA_SIZE) == 0);
    CHECK(read_equals(target, new_data, DATA_SIZE));
    CHECK(!path_exists(tmp));
    CHECK(!path_exists(other));
#endif
    CHECK_DONE();
}

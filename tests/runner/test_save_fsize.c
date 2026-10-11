/* The runner's atomic replace under a file size limit (TUNE-07, D-06): the
   write is cut short, the helper returns 1, the old file is intact and no .tmp
   is left. This is its own process because RLIMIT_FSIZE applies to every file
   the process writes, including the test's own setup files. */
#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "save.h"
#include "../check.h"

#define DATA_SIZE 8192u

int main(int argc, char **argv)
{
    static uint8_t old_data[DATA_SIZE];
    static uint8_t new_data[DATA_SIZE];
    static uint8_t got[DATA_SIZE + 16u];
    char target[1200];
    char tmp[1210];
    struct rlimit saved;
    struct rlimit low;
    struct stat st;
    FILE *f;
    int rc;
    size_t n = 0;
    if (argc < 2) {
        fprintf(stderr, "usage: test_save_fsize SCRATCH_DIR\n");
        return 2;
    }
    (void)mkdir(argv[1], 0755);
    snprintf(target, sizeof target, "%s/a.sav", argv[1]);
    snprintf(tmp, sizeof tmp, "%s.tmp", target);
    for (size_t i = 0; i < DATA_SIZE; i++) {
        old_data[i] = (uint8_t)(i * 3u + 1u);
        new_data[i] = (uint8_t)(i * 5u + 77u);
    }
    f = fopen(target, "wb");
    CHECK(f != NULL);
    if (f != NULL) {
        CHECK(fwrite(old_data, 1, DATA_SIZE, f) == DATA_SIZE);
        fclose(f);
    }

    signal(SIGXFSZ, SIG_IGN);
    CHECK(getrlimit(RLIMIT_FSIZE, &saved) == 0);
    low = saved;
    low.rlim_cur = 16;
    CHECK(setrlimit(RLIMIT_FSIZE, &low) == 0);
    rc = nesturbator_run_file_replace(target, new_data, DATA_SIZE);
    CHECK(setrlimit(RLIMIT_FSIZE, &saved) == 0);

    CHECK(rc == 1);
    f = fopen(target, "rb");
    CHECK(f != NULL);
    if (f != NULL) {
        n = fread(got, 1, sizeof got, f);
        fclose(f);
    }
    CHECK(n == DATA_SIZE && memcmp(got, old_data, DATA_SIZE) == 0);
    CHECK(stat(tmp, &st) != 0);
    CHECK_DONE();
}

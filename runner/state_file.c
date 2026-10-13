#include "state_file.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#ifndef S_ISDIR
#define S_ISDIR(m) (((m) & _S_IFMT) == _S_IFDIR)
#endif
#define stat _stat
#endif

int nesturbator_run_state_read(const char *path, uint8_t *buf, size_t cap, size_t *got)
{
    struct stat st;
    FILE *f;
    size_t n;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        fprintf(stderr, "nesturbator-run: cannot read state %s (is a directory)\n", path);
        return 1;
    }
    /* A stop signal can interrupt the open of a FIFO or a slow file; the load is never aborted
       by it (RUN-02, D-05), so an interrupted open or read is retried. */
    do {
        f = fopen(path, "rb");
    } while (f == NULL && errno == EINTR);
    if (f == NULL) {
        fprintf(stderr, "nesturbator-run: cannot read state %s (%s)\n", path, strerror(errno));
        return 1;
    }
    n = 0;
    for (;;) {
        n += fread(buf + n, 1, cap - n, f);
        if (ferror(f) != 0 && errno == EINTR) {
            clearerr(f);
            continue;
        }
        break;
    }
    if (ferror(f) != 0) {
        fprintf(stderr, "nesturbator-run: cannot read state %s (read error)\n", path);
        fclose(f);
        return 1;
    }
    fclose(f);
    *got = n;
    return 0;
}

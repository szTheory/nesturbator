/* The runner's save-state file reading for --load-state (RUN-01, D-07). */
#ifndef NESTURBATOR_RUN_STATE_FILE_H
#define NESTURBATOR_RUN_STATE_FILE_H

#include <stddef.h>
#include <stdint.h>

/* Reads at most `cap` bytes of the file at `path` into `buf` and stores the
   count in `*got`. A caller that passes the state size plus one sees a longer
   file as one byte too many, without allocating from the file's length. A
   directory, a missing file or a read error prints
   "nesturbator-run: cannot read state <path> (<reason>)" and returns 1;
   otherwise returns 0. The file is opened read-only and never changed. An open or read
   interrupted by a signal is retried, so a stop request never aborts a load. */
int nesturbator_run_state_read(const char *path, uint8_t *buf, size_t cap, size_t *got);

#endif /* NESTURBATOR_RUN_STATE_FILE_H */

/* The runner's battery-save file handling for --save-dir (D-18, D-19). */
#ifndef NESTURBATOR_RUN_SAVE_H
#define NESTURBATOR_RUN_SAVE_H

#include <stddef.h>
#include <stdint.h>

enum nesturbator_run_save_status {
    NESTURBATOR_RUN_SAVE_LOADED,   /* the file held exactly `size` bytes, now in the span */
    NESTURBATOR_RUN_SAVE_FRESH,    /* there is no file; the span is untouched */
    NESTURBATOR_RUN_SAVE_MISMATCH, /* the file's length is not `size`; a message was printed */
    NESTURBATOR_RUN_SAVE_ERROR     /* the file could not be read; a message was printed */
};

/* Writes "<dir>/<stem>.sav" to buf. The stem is the last component of `rom`
   split at '/' or '\' on every platform, minus its last extension unless that
   dot is the first character. Returns 1, or 0 when `cap` is too small. */
int nesturbator_run_save_path(char *buf, size_t cap, const char *dir, const char *rom);

/* Reads `path` into span. The file is opened read-only and never changed. */
enum nesturbator_run_save_status nesturbator_run_save_load(const char *path, uint8_t *span,
                                                           size_t size);

/* Replaces `path` with the span's bytes through "<path>.tmp": write, flush,
   sync, checked close, rename. On success the span is copied to `shadow` and
   0 is returned. On failure a message is printed and 1 is returned. */
int nesturbator_run_save_flush(const char *path, const uint8_t *span, size_t size, uint8_t *shadow);

#endif /* NESTURBATOR_RUN_SAVE_H */

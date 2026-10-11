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

/* Replaces `path` with `size` bytes of `data` through "<path>.tmp": a stale
   regular file or symlink at the temporary name is removed (a link is
   unlinked, its target is never touched), then exclusive create, write,
   flush, sync (F_FULLFSYNC on Apple platforms, where fsync alone does not
   flush the drive cache), checked close, rename (MoveFileExA with write-through on
   Windows). Returns 0, or 1 after printing one line:
   "nesturbator-run: cannot save <path> (<reason>); the previous save is kept".
   On failure the old file is untouched and the .tmp this call created is
   removed; a directory at the temporary name is refused and left alone. Two
   runners sharing one save directory are not supported. The result is the old
   file or the new file, never torn. There is no directory sync, so a power cut just after the rename
   may still show the previous save. Used by the battery flush and, later, by
   state writes. */
int nesturbator_run_file_replace(const char *path, const uint8_t *data, size_t size);

/* The helper plus the shadow copy: on success the span is copied to `shadow`
   and 0 is returned; on failure the shadow is unchanged and 1 is returned. */
int nesturbator_run_save_flush(const char *path, const uint8_t *span, size_t size, uint8_t *shadow);

#endif /* NESTURBATOR_RUN_SAVE_H */

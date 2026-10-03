/* The runner's binary PPM (P6) writer for --dump-frame (D-16). */
#ifndef NESTURBATOR_RUN_PPM_H
#define NESTURBATOR_RUN_PPM_H

#include <stdint.h>

/* Writes a 256x240 frame of XRGB8888 pixels (0x00RRGGBB, `pitch` elements per
   row) to `path` as "P6\n256 240\n255\n" followed by R, G, B bytes, row by row
   from the top. Returns 0 on success, -1 if the file cannot be opened,
   written or closed. */
int nesturbator_run_write_ppm(const char *path, const uint32_t *xrgb, uint32_t pitch);

#endif /* NESTURBATOR_RUN_PPM_H */

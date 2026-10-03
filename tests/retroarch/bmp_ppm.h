/* Image readers and the exact comparison behind compare_frame (D-17).
   Test code only: never linked into the core.

   Return codes everywhere: 0 ok, 1 mismatch or wrong size, 2 the file
   cannot be read or parsed. */
#ifndef NESTURBATOR_TEST_BMP_PPM_H
#define NESTURBATOR_TEST_BMP_PPM_H

#include <stddef.h>
#include <stdint.h>

#define NESTURBATOR_TEST_W 256u
#define NESTURBATOR_TEST_H 240u
#define NESTURBATOR_TEST_RGB_SIZE (NESTURBATOR_TEST_W * NESTURBATOR_TEST_H * 3u)

/* Reads a binary PPM that is exactly "P6", whitespace, "256", whitespace,
   "240", whitespace, "255", one whitespace byte and 256*240*3 bytes of RGB,
   with nothing after them. Anything else returns 2. */
int nesturbator_test_read_ppm(const char *path, uint8_t rgb[NESTURBATOR_TEST_RGB_SIZE]);

/* Reads a BMP with a 40, 108 or 124-byte header: 24 bpp uncompressed, or
   32 bpp uncompressed or BI_BITFIELDS, either row order. Sets *w and *h, then
   writes the pixels top row first as R,G,B into rgb when w*h*3 fits in
   rgb_cap, and returns 0. When it does not fit, returns 1 with *w and *h
   set. A malformed or truncated file returns 2. */
int nesturbator_test_read_bmp(const char *path, uint32_t *w, uint32_t *h, uint8_t *rgb,
                              size_t rgb_cap);

/* Compares two 256x240 RGB images. Returns 0 when every byte is equal, else
   1 with the first differing pixel, in row order, in *x and *y. */
int nesturbator_test_compare(const uint8_t *a, const uint8_t *b, uint32_t *x, uint32_t *y);

/* compare_frame's whole check: reads the runner's P6 and the screenshot BMP,
   requires 256x240 and exact equality, and writes a one-line reason into
   msg (empty on success). Returns 0, 1 or 2 as above. */
int nesturbator_test_compare_files(const char *ppm_path, const char *bmp_path, char *msg,
                                   size_t msg_cap);

#endif /* NESTURBATOR_TEST_BMP_PPM_H */

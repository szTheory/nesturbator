/* Bounds-checked P6 and BMP readers and the exact frame comparison (D-17).
   Each file is read whole into a buffer of its exact size, at most 16 MiB,
   and every field read is checked against that size first. */
#include "bmp_ppm.h"

#include <stdio.h>
#include <stdlib.h>

#define FILE_LIMIT (16ul * 1024ul * 1024ul)
/* Larger than any screenshot this test meets, small enough that
   stride * height cannot overflow 64 bits. */
#define DIM_LIMIT 16384u

static uint8_t *read_file(const char *path, size_t *size)
{
    FILE *f = fopen(path, "rb");
    if (f == NULL)
        return NULL;
    uint8_t *buf = NULL;
    long end = -1;
    if (fseek(f, 0, SEEK_END) == 0)
        end = ftell(f);
    if (end >= 0 && (unsigned long)end <= FILE_LIMIT && fseek(f, 0, SEEK_SET) == 0) {
        size_t n = (size_t)end;
        buf = malloc(n > 0 ? n : 1);
        if (buf != NULL && fread(buf, 1, n, f) != n) {
            free(buf);
            buf = NULL;
        }
        *size = n;
    }
    fclose(f);
    return buf;
}

/* Little-endian fields, byte by byte; 0 when the field lies past the end. */
static int get16(const uint8_t *b, size_t n, size_t off, uint32_t *v)
{
    if (off > n || n - off < 2)
        return 0;
    *v = (uint32_t)b[off] | (uint32_t)b[off + 1] << 8;
    return 1;
}

static int get32(const uint8_t *b, size_t n, size_t off, uint32_t *v)
{
    if (off > n || n - off < 4)
        return 0;
    *v = (uint32_t)b[off] | (uint32_t)b[off + 1] << 8 | (uint32_t)b[off + 2] << 16 |
         (uint32_t)b[off + 3] << 24;
    return 1;
}

static int is_space(uint8_t c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

/* Skips at least one whitespace byte, then reads a decimal number of at most
   five digits. */
static int ppm_number(const uint8_t *b, size_t n, size_t *pos, uint32_t *v)
{
    size_t p = *pos;
    if (p >= n || !is_space(b[p]))
        return 0;
    while (p < n && is_space(b[p]))
        p++;
    uint32_t value = 0;
    size_t digits = 0;
    while (p < n && b[p] >= '0' && b[p] <= '9' && digits < 6) {
        value = value * 10u + (uint32_t)(b[p] - '0');
        p++;
        digits++;
    }
    if (digits == 0 || digits > 5)
        return 0;
    *v = value;
    *pos = p;
    return 1;
}

int nesturbator_test_read_ppm(const char *path, uint8_t rgb[NESTURBATOR_TEST_RGB_SIZE])
{
    size_t n = 0;
    uint8_t *b = read_file(path, &n);
    if (b == NULL)
        return 2;
    int rc = 2;
    size_t pos = 2;
    uint32_t w = 0, h = 0, maxval = 0;
    if (n >= 2 && b[0] == 'P' && b[1] == '6' && ppm_number(b, n, &pos, &w) &&
        ppm_number(b, n, &pos, &h) && ppm_number(b, n, &pos, &maxval) && w == NESTURBATOR_TEST_W &&
        h == NESTURBATOR_TEST_H && maxval == 255u && pos < n && is_space(b[pos]) &&
        n - (pos + 1) == NESTURBATOR_TEST_RGB_SIZE) {
        for (size_t i = 0; i < NESTURBATOR_TEST_RGB_SIZE; i++)
            rgb[i] = b[pos + 1 + i];
        rc = 0;
    }
    free(b);
    return rc;
}

/* A BI_BITFIELDS mask must select one whole byte; returns its shift. */
static int mask_shift(uint32_t mask, uint32_t *shift)
{
    for (uint32_t s = 0; s < 32u; s += 8u) {
        if (mask == 0xFFu << s) {
            *shift = s;
            return 1;
        }
    }
    return 0;
}

static int parse_bmp(const uint8_t *b, size_t n, uint32_t *w, uint32_t *h, uint8_t *rgb,
                     size_t rgb_cap)
{
    uint32_t offset, hsize, width, height, planes, bpp, comp;
    if (n < 2 || b[0] != 'B' || b[1] != 'M')
        return 2;
    if (!get32(b, n, 10, &offset) || !get32(b, n, 14, &hsize))
        return 2;
    /* BITMAPINFOHEADER (40 bytes), BITMAPV4HEADER (108), BITMAPV5HEADER
       (124). sips writes 40 for RGB and 124 for RGBA input (01-RESEARCH,
       measured). */
    if (hsize != 40u && hsize != 108u && hsize != 124u)
        return 2;
    if (!get32(b, n, 18, &width) || !get32(b, n, 22, &height) || !get16(b, n, 26, &planes) ||
        !get16(b, n, 28, &bpp) || !get32(b, n, 30, &comp))
        return 2;
    if (14u + hsize > n || offset < 14u + hsize || planes != 1u)
        return 2;
    /* Width is a positive int32. A negative height means the rows are stored
       top row first; a positive one, bottom row first. */
    if (width == 0u || width > DIM_LIMIT)
        return 2;
    int top_down = (height & 0x80000000u) != 0u;
    uint32_t rows = top_down ? 0u - height : height;
    if (rows == 0u || rows > DIM_LIMIT)
        return 2;

    uint32_t shift_r = 16u, shift_g = 8u, shift_b = 0u;
    if (bpp == 32u && comp == 3u) {
        /* BI_BITFIELDS: the red, green and blue masks follow the 40-byte
           fields at file offset 54, inside a V4 or V5 header or after a
           40-byte one. */
        uint32_t mr, mg, mb;
        if (!get32(b, n, 54, &mr) || !get32(b, n, 58, &mg) || !get32(b, n, 62, &mb))
            return 2;
        if (!mask_shift(mr, &shift_r) || !mask_shift(mg, &shift_g) || !mask_shift(mb, &shift_b))
            return 2;
    } else if (!((bpp == 24u || bpp == 32u) && comp == 0u)) {
        return 2;
    }

    /* Each row is padded to a multiple of four bytes. */
    uint64_t stride = (((uint64_t)width * bpp + 31u) / 32u) * 4u;
    if ((uint64_t)offset + stride * rows > (uint64_t)n)
        return 2;

    *w = width;
    *h = rows;
    if ((uint64_t)width * rows * 3u > (uint64_t)rgb_cap)
        return 1;
    size_t bytes = bpp / 8u;
    for (uint32_t y = 0; y < rows; y++) {
        uint32_t src_row = top_down ? y : rows - 1u - y;
        const uint8_t *src = b + offset + (size_t)stride * src_row;
        uint8_t *dst = rgb + (size_t)y * width * 3u;
        for (uint32_t x = 0; x < width; x++) {
            const uint8_t *p = src + (size_t)x * bytes;
            if (bpp == 24u || comp == 0u) {
                /* Stored as B, G, R (then an unused byte at 32 bpp). */
                dst[x * 3u + 0u] = p[2];
                dst[x * 3u + 1u] = p[1];
                dst[x * 3u + 2u] = p[0];
            } else {
                uint32_t v = (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
                             (uint32_t)p[3] << 24;
                dst[x * 3u + 0u] = (uint8_t)(v >> shift_r);
                dst[x * 3u + 1u] = (uint8_t)(v >> shift_g);
                dst[x * 3u + 2u] = (uint8_t)(v >> shift_b);
            }
        }
    }
    return 0;
}

int nesturbator_test_read_bmp(const char *path, uint32_t *w, uint32_t *h, uint8_t *rgb,
                              size_t rgb_cap)
{
    size_t n = 0;
    uint8_t *b = read_file(path, &n);
    if (b == NULL)
        return 2;
    int rc = parse_bmp(b, n, w, h, rgb, rgb_cap);
    free(b);
    return rc;
}

int nesturbator_test_compare(const uint8_t *a, const uint8_t *b, uint32_t *x, uint32_t *y)
{
    for (uint32_t row = 0; row < NESTURBATOR_TEST_H; row++) {
        for (uint32_t col = 0; col < NESTURBATOR_TEST_W; col++) {
            size_t i = ((size_t)row * NESTURBATOR_TEST_W + col) * 3u;
            if (a[i] != b[i] || a[i + 1] != b[i + 1] || a[i + 2] != b[i + 2]) {
                *x = col;
                *y = row;
                return 1;
            }
        }
    }
    return 0;
}

static unsigned rgb_hex(const uint8_t *p)
{
    return (unsigned)p[0] << 16 | (unsigned)p[1] << 8 | (unsigned)p[2];
}

int nesturbator_test_compare_files(const char *ppm_path, const char *bmp_path, char *msg,
                                   size_t msg_cap)
{
    uint8_t *expected = malloc(NESTURBATOR_TEST_RGB_SIZE);
    uint8_t *got = malloc(NESTURBATOR_TEST_RGB_SIZE);
    int rc = 2;
    uint32_t w = 0, h = 0, x = 0, y = 0;
    msg[0] = '\0';
    if (expected == NULL || got == NULL) {
        snprintf(msg, msg_cap, "out of memory");
    } else if (nesturbator_test_read_ppm(ppm_path, expected) != 0) {
        snprintf(msg, msg_cap, "cannot read %s as a 256x240 P6 image", ppm_path);
    } else if (nesturbator_test_read_bmp(bmp_path, &w, &h, got, NESTURBATOR_TEST_RGB_SIZE) == 2) {
        snprintf(msg, msg_cap, "cannot read %s as a BMP image", bmp_path);
    } else if (w != NESTURBATOR_TEST_W || h != NESTURBATOR_TEST_H) {
        snprintf(msg, msg_cap, "size %u x %u, expected 256 x 240", (unsigned)w, (unsigned)h);
        rc = 1;
    } else if (nesturbator_test_compare(expected, got, &x, &y) != 0) {
        size_t i = ((size_t)y * NESTURBATOR_TEST_W + x) * 3u;
        snprintf(msg, msg_cap, "first difference at %u,%u: expected %06X got %06X", (unsigned)x,
                 (unsigned)y, rgb_hex(expected + i), rgb_hex(got + i));
        rc = 1;
    } else {
        rc = 0;
    }
    free(expected);
    free(got);
    return rc;
}

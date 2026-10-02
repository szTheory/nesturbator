/* compare_frame's readers and comparison on images built here (D-17).
   Every image comes from one pattern, pixel (x,y) = (x, y, x ^ y) with each
   channel taken mod 256, written as a P6 and as each BMP layout sips
   produces or might produce. argv[1] is a directory to write them into; the
   equal pair frame.ppm and shot.bmp is left there for retroarch.compare.cli. */
#include "../check.h"
#include "bmp_ppm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *dir;

static uint8_t pattern(uint32_t x, uint32_t y, unsigned ch)
{
    uint32_t v = ch == 0u ? x : ch == 1u ? y : x ^ y;
    return (uint8_t)(v & 0xFFu);
}

static const char *path_of(const char *name)
{
    static char path[1024];
    snprintf(path, sizeof path, "%s/%s", dir, name);
    return path;
}

static const char *write_file(const char *name, const uint8_t *data, size_t n)
{
    const char *path = path_of(name);
    FILE *f = fopen(path, "wb");
    CHECK(f != NULL);
    if (f != NULL) {
        CHECK(fwrite(data, 1, n, f) == n);
        CHECK(fclose(f) == 0);
    }
    return path;
}

static void put16(uint8_t *b, size_t off, uint32_t v)
{
    b[off] = (uint8_t)v;
    b[off + 1] = (uint8_t)(v >> 8);
}

static void put32(uint8_t *b, size_t off, uint32_t v)
{
    put16(b, off, v & 0xFFFFu);
    put16(b, off + 2, v >> 16);
}

static const char *write_ppm(const char *name)
{
    static const char header[] = "P6\n256 240\n255\n";
    size_t hn = sizeof header - 1;
    size_t n = hn + NESTURBATOR_TEST_RGB_SIZE;
    uint8_t *b = malloc(n);
    CHECK(b != NULL);
    if (b == NULL)
        return path_of(name);
    memcpy(b, header, hn);
    for (uint32_t y = 0; y < NESTURBATOR_TEST_H; y++)
        for (uint32_t x = 0; x < NESTURBATOR_TEST_W; x++)
            for (unsigned c = 0; c < 3u; c++)
                b[hn + (y * NESTURBATOR_TEST_W + x) * 3u + c] = pattern(x, y, c);
    const char *path = write_file(name, b, n);
    free(b);
    return path;
}

/* A BMP layout: header size, bits per pixel, compression, row order and,
   for BI_BITFIELDS, the bit position of red, green, blue and alpha. */
struct layout {
    uint32_t hsize, bpp, comp;
    int top_down;
    uint32_t sr, sg, sb, sa;
};

static const struct layout RGB40_TOP = {40, 24, 0, 1, 0, 0, 0, 0};
static const struct layout RGB40_BOTTOM = {40, 24, 0, 0, 0, 0, 0, 0};
static const struct layout V4_BOTTOM = {108, 32, 3, 0, 16, 8, 0, 24};
static const struct layout V5_TOP = {124, 32, 3, 1, 16, 8, 0, 24};
static const struct layout V4_RGBA_TOP = {108, 32, 3, 1, 0, 8, 16, 24};
static const struct layout X40_BOTTOM = {40, 32, 0, 0, 0, 0, 0, 0};

/* Builds a w x h BMP of the pattern in a malloc'd buffer; *n is its size. */
static uint8_t *make_bmp(const struct layout *l, uint32_t w, uint32_t h, size_t *n)
{
    uint32_t offset = 14u + l->hsize + (l->hsize == 40u && l->comp == 3u ? 12u : 0u);
    uint32_t stride = ((w * l->bpp + 31u) / 32u) * 4u;
    *n = (size_t)offset + (size_t)stride * h;
    uint8_t *b = calloc(*n, 1);
    CHECK(b != NULL);
    if (b == NULL)
        return NULL;
    b[0] = 'B';
    b[1] = 'M';
    put32(b, 2, (uint32_t)*n);
    put32(b, 10, offset);
    put32(b, 14, l->hsize);
    put32(b, 18, w);
    put32(b, 22, l->top_down ? 0u - h : h);
    put16(b, 26, 1);
    put16(b, 28, l->bpp);
    put32(b, 30, l->comp);
    put32(b, 34, stride * h);
    put32(b, 38, 2835);
    put32(b, 42, 2835);
    if (l->comp == 3u) {
        put32(b, 54, 0xFFu << l->sr);
        put32(b, 58, 0xFFu << l->sg);
        put32(b, 62, 0xFFu << l->sb);
        put32(b, 66, 0xFFu << l->sa);
    }
    for (uint32_t y = 0; y < h; y++) {
        uint32_t row = l->top_down ? y : h - 1u - y;
        uint8_t *dst = b + offset + (size_t)stride * row;
        for (uint32_t x = 0; x < w; x++) {
            uint32_t r = pattern(x, y, 0), g = pattern(x, y, 1), bl = pattern(x, y, 2);
            if (l->bpp == 24u) {
                dst[x * 3u + 0u] = (uint8_t)bl;
                dst[x * 3u + 1u] = (uint8_t)g;
                dst[x * 3u + 2u] = (uint8_t)r;
            } else if (l->comp == 0u) {
                dst[x * 4u + 0u] = (uint8_t)bl;
                dst[x * 4u + 1u] = (uint8_t)g;
                dst[x * 4u + 2u] = (uint8_t)r;
            } else {
                put32(dst, x * 4u, r << l->sr | g << l->sg | bl << l->sb | 0xFFu << l->sa);
            }
        }
    }
    return b;
}

static const char *write_bmp(const char *name, const struct layout *l, uint32_t w, uint32_t h)
{
    size_t n = 0;
    uint8_t *b = make_bmp(l, w, h, &n);
    const char *path = b != NULL ? write_file(name, b, n) : path_of(name);
    free(b);
    return path;
}

static char ppm_path[1024];
static uint8_t expected[NESTURBATOR_TEST_RGB_SIZE];
static uint8_t got[NESTURBATOR_TEST_RGB_SIZE];

/* The layout reads back as the pattern, and compare_files accepts it. */
static void check_equal(const char *name, const struct layout *l)
{
    const char *path = write_bmp(name, l, NESTURBATOR_TEST_W, NESTURBATOR_TEST_H);
    uint32_t w = 0, h = 0, x = 99, y = 99;
    memset(got, 0, sizeof got);
    CHECK_EQ_U64(nesturbator_test_read_bmp(path, &w, &h, got, sizeof got), 0);
    CHECK_EQ_U64(w, 256);
    CHECK_EQ_U64(h, 240);
    CHECK_EQ_U64(nesturbator_test_compare(expected, got, &x, &y), 0);
    char msg[512];
    CHECK_EQ_U64(nesturbator_test_compare_files(ppm_path, path, msg, sizeof msg), 0);
    CHECK(msg[0] == '\0');
}

/* compare_files returns rc with exactly this message. */
static void check_files(const char *bmp_path, int rc, const char *want)
{
    char msg[512];
    CHECK_EQ_U64(nesturbator_test_compare_files(ppm_path, bmp_path, msg, sizeof msg), rc);
    if (strcmp(msg, want) != 0) {
        fprintf(stderr, "message \"%s\", expected \"%s\"\n", msg, want);
        CHECK(0);
    }
}

/* A BMP of the 40-byte, 24 bpp, top-down layout with byte off set to v
   (as a 32-bit field when wide, else one byte) and the file cut to keep
   bytes, must be rejected with 2. */
static void check_bad_bmp(const char *name, size_t off, uint32_t v, int wide, size_t keep)
{
    size_t n = 0;
    uint8_t *b = make_bmp(&RGB40_TOP, NESTURBATOR_TEST_W, NESTURBATOR_TEST_H, &n);
    if (b == NULL)
        return;
    if (wide)
        put32(b, off, v);
    else if (off < n)
        b[off] = (uint8_t)v;
    const char *path = write_file(name, b, keep < n ? keep : n);
    free(b);
    uint32_t w = 0, h = 0;
    CHECK_EQ_U64(nesturbator_test_read_bmp(path, &w, &h, got, sizeof got), 2);
}

static void check_bad_ppm(const char *name, const char *header, size_t data_bytes)
{
    size_t hn = strlen(header);
    uint8_t *b = calloc(hn + data_bytes + 1, 1);
    CHECK(b != NULL);
    if (b == NULL)
        return;
    memcpy(b, header, hn);
    const char *path = write_file(name, b, hn + data_bytes);
    free(b);
    CHECK_EQ_U64(nesturbator_test_read_ppm(path, got), 2);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: test_compare_frame DIR\n");
        return 2;
    }
    dir = argv[1];
    snprintf(ppm_path, sizeof ppm_path, "%s", write_ppm("frame.ppm"));
    CHECK_EQ_U64(nesturbator_test_read_ppm(ppm_path, expected), 0);
    CHECK_EQ_U64(expected[(17u * 256u + 255u) * 3u + 2u], 255u ^ 17u);

    /* sips writes the first for RGB input and the V5 for RGBA input
       (01-RESEARCH); the others are the remaining valid combinations. */
    check_equal("shot.bmp", &RGB40_TOP);
    check_equal("rgb40_bottom.bmp", &RGB40_BOTTOM);
    check_equal("v4_bottom.bmp", &V4_BOTTOM);
    check_equal("v5_top.bmp", &V5_TOP);
    check_equal("v4_rgba_top.bmp", &V4_RGBA_TOP);
    check_equal("x40_bottom.bmp", &X40_BOTTOM);

    /* Boundary: one column or one row off is a size mismatch, never a pass.
       The reader reports the size even when the pixels do not fit. */
    uint32_t w = 0, h = 0;
    const char *wide = write_bmp("wide.bmp", &V5_TOP, 257, 240);
    CHECK_EQ_U64(nesturbator_test_read_bmp(wide, &w, &h, got, sizeof got), 1);
    CHECK_EQ_U64(w, 257);
    CHECK_EQ_U64(h, 240);
    check_files(wide, 1, "size 257 x 240, expected 256 x 240");
    check_files(write_bmp("short.bmp", &RGB40_BOTTOM, 256, 239), 1,
                "size 256 x 239, expected 256 x 240");

    /* Precision: one channel of one pixel off by one is found, at its
       place, with no tolerance. Pixel (17,200) is 11 C8 D9. */
    size_t n = 0;
    uint8_t *b = make_bmp(&V5_TOP, NESTURBATOR_TEST_W, NESTURBATOR_TEST_H, &n);
    if (b != NULL) {
        b[138u + (200u * 256u + 17u) * 4u + 1u]++; /* green, stored B,G,R,A */
        const char *off_by_one = write_file("off_by_one.bmp", b, n);
        free(b);
        uint32_t x = 0, y = 0;
        CHECK_EQ_U64(nesturbator_test_read_bmp(off_by_one, &w, &h, got, sizeof got), 0);
        CHECK_EQ_U64(nesturbator_test_compare(expected, got, &x, &y), 1);
        CHECK_EQ_U64(x, 17);
        CHECK_EQ_U64(y, 200);
        check_files(off_by_one, 1, "first difference at 17,200: expected 11C8D9 got 11C9D9");
    }

    /* Malformed or truncated files exit 2 without reading past the end;
       the asan preset runs this test. A 40-byte header puts the pixels at
       offset 54. */
    size_t full = 54u + NESTURBATOR_TEST_RGB_SIZE;
    check_bad_bmp("trunc_last.bmp", 0, 'B', 0, full - 1u);
    check_bad_bmp("trunc_header.bmp", 0, 'B', 0, 30);
    check_bad_bmp("trunc_magic.bmp", 0, 'B', 0, 1);
    check_bad_bmp("empty.bmp", 0, 'B', 0, 0);
    check_bad_bmp("offset_plus4.bmp", 10, 58, 1, full);
    check_bad_bmp("offset_huge.bmp", 10, 0xFFFFFFF0u, 1, full);
    check_bad_bmp("offset_inside.bmp", 10, 20, 1, full);
    check_bad_bmp("magic.bmp", 1, 'X', 0, full);
    check_bad_bmp("hsize.bmp", 14, 64, 1, full);
    check_bad_bmp("width0.bmp", 18, 0, 1, full);
    check_bad_bmp("height0.bmp", 22, 0, 1, full);
    check_bad_bmp("height_huge.bmp", 22, 0x80000000u, 1, full);
    check_bad_bmp("planes.bmp", 26, 2, 0, full);
    check_bad_bmp("bpp16.bmp", 28, 16, 0, full);
    check_bad_bmp("rle.bmp", 30, 1, 1, full);
    check_bad_bmp("bitfields24.bmp", 30, 3, 1, full);
    /* A BI_BITFIELDS mask must select a whole byte. */
    b = make_bmp(&V5_TOP, NESTURBATOR_TEST_W, NESTURBATOR_TEST_H, &n);
    if (b != NULL) {
        put32(b, 54, 0x00FFFF00u);
        const char *mask = write_file("mask.bmp", b, n);
        free(b);
        CHECK_EQ_U64(nesturbator_test_read_bmp(mask, &w, &h, got, sizeof got), 2);
    }
    char msg[512];
    CHECK_EQ_U64(nesturbator_test_compare_files(ppm_path, path_of("no-such.bmp"), msg, sizeof msg),
                 2);

    /* The P6 reader takes exactly the runner's header and pixel count. */
    check_bad_ppm("w255.ppm", "P6\n255 240\n255\n", NESTURBATOR_TEST_RGB_SIZE);
    check_bad_ppm("h241.ppm", "P6\n256 241\n255\n", NESTURBATOR_TEST_RGB_SIZE);
    check_bad_ppm("max65535.ppm", "P6\n256 240\n65535\n", NESTURBATOR_TEST_RGB_SIZE);
    check_bad_ppm("p3.ppm", "P3\n256 240\n255\n", NESTURBATOR_TEST_RGB_SIZE);
    check_bad_ppm("nospace.ppm", "P6\n256 240\n255", NESTURBATOR_TEST_RGB_SIZE);
    check_bad_ppm("short.ppm", "P6\n256 240\n255\n", NESTURBATOR_TEST_RGB_SIZE - 1u);
    check_bad_ppm("long.ppm", "P6\n256 240\n255\n", NESTURBATOR_TEST_RGB_SIZE + 1u);
    check_bad_ppm("header_only.ppm", "P6\n256 240\n", 0);
    char short_ppm[1024];
    snprintf(short_ppm, sizeof short_ppm, "%s", path_of("short.ppm"));
    CHECK_EQ_U64(nesturbator_test_compare_files(short_ppm, path_of("shot.bmp"), msg, sizeof msg),
                 2);

    CHECK_DONE();
}

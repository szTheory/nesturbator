/* Tests for the Holy Mapperel result-screen reader in hm_decode.h.
   With no argument: the decoder cases on painted frames.
   With --glyphs ROM: the font table equals tiles $30-$39 and $01-$06 of the
   committed Holy Mapperel M3 ROM's CHR. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../check.h"
#include "hm_decode.h"

static uint8_t frame[HM_RGB_SIZE];

static void fill(uint8_t r, uint8_t g, uint8_t b)
{
    size_t i;
    for (i = 0; i < HM_RGB_SIZE; i += 3) {
        frame[i] = r;
        frame[i + 1] = g;
        frame[i + 2] = b;
    }
}

/* Paints glyph index g at cell (x, y) with colour (r, g, b) on lit pixels. */
static void paint(int x, int y, int glyph, uint8_t r, uint8_t g, uint8_t b)
{
    int row;
    int col;
    for (row = 0; row < 8; row++) {
        for (col = 0; col < 8; col++) {
            if ((hm_glyph[glyph][row] & (0x80u >> col)) != 0) {
                uint8_t *p = frame + ((size_t)(y + row) * HM_WIDTH + (size_t)(x + col)) * 3u;
                p[0] = r;
                p[1] = g;
                p[2] = b;
            }
        }
    }
}

static void screen(int d0, int d1, int d2, int d3)
{
    int d[4];
    int k;
    d[0] = d0;
    d[1] = d1;
    d[2] = d2;
    d[3] = d3;
    fill(0x10, 0x20, 0x30);
    paint(HM_ANCHOR_D_X, HM_ROW_Y, 13, 0xf0, 0xe0, 0xd0);
    paint(HM_ANCHOR_E_X, HM_ROW_Y, 14, 0xf0, 0xe0, 0xd0);
    for (k = 0; k < 4; k++) {
        paint(HM_DIGIT_X + 8 * k, HM_ROW_Y, d[k], 0xf0, 0xe0, 0xd0);
    }
}

static void test_cases(void)
{
    uint16_t code = 0xffff;

    screen(0, 0, 0, 0);
    CHECK_EQ_U64(hm_decode(frame, &code), 0);
    CHECK_EQ_HEX(code, 0x0000);

    screen(12, 0, 13, 14);
    CHECK_EQ_U64(hm_decode(frame, &code), 1);
    CHECK_EQ_HEX(code, 0xc0de);

    screen(1, 2, 10, 15);
    CHECK_EQ_U64(hm_decode(frame, &code), 1);
    CHECK_EQ_HEX(code, 0x12af);

    /* Every glyph reads back as itself in the last digit cell. */
    {
        int g;
        for (g = 0; g < 16; g++) {
            screen(0, 0, 0, g);
            CHECK_EQ_U64(hm_decode(frame, &code), g == 0 ? 0 : 1);
            CHECK_EQ_HEX(code, g);
        }
    }

    /* A blank frame has no result screen. */
    fill(0, 0, 0);
    CHECK_EQ_U64(hm_decode(frame, &code), 2);

    /* Digits 0000 without the anchor cells. */
    screen(0, 0, 0, 0);
    paint(HM_ANCHOR_D_X, HM_ROW_Y, 13, 0x10, 0x20, 0x30);
    CHECK_EQ_U64(hm_decode(frame, &code), 2);
    screen(0, 0, 0, 0);
    paint(HM_ANCHOR_E_X, HM_ROW_Y, 14, 0x10, 0x20, 0x30);
    CHECK_EQ_U64(hm_decode(frame, &code), 2);

    /* One extra lit pixel in a digit cell matches no glyph. */
    screen(0, 0, 0, 0);
    {
        uint8_t *p =
            frame + ((size_t)(HM_ROW_Y + 7) * HM_WIDTH + (size_t)(HM_DIGIT_X + 8 * 2 + 7)) * 3u;
        p[0] = 0xf0;
    }
    {
        int digit[4];
        CHECK_EQ_U64(hm_decode_digits(frame, &code, digit), 2);
        CHECK_EQ_U64(digit[0], 0);
        CHECK_EQ_U64(digit[2] < 0, 1);
    }

    /* Colours do not matter: swapped colours still read 0000. */
    screen(0, 0, 0, 0);
    {
        size_t i;
        for (i = 0; i < HM_RGB_SIZE; i += 3) {
            if (frame[i] == 0x10) {
                frame[i] = 0xf0;
                frame[i + 1] = 0xe0;
                frame[i + 2] = 0xd0;
            } else {
                frame[i] = 0x10;
                frame[i + 1] = 0x20;
                frame[i + 2] = 0x30;
            }
        }
    }
    CHECK_EQ_U64(hm_decode(frame, &code), 0);
}

static void test_ppm(void)
{
    static uint8_t file[HM_HEADER_SIZE + HM_RGB_SIZE];
    const uint8_t *rgb = NULL;

    memcpy(file, HM_HEADER, HM_HEADER_SIZE);
    CHECK_EQ_U64(sizeof file, 184335);
    CHECK_EQ_U64(hm_read_ppm(file, sizeof file, &rgb), 1);
    CHECK(rgb == file + HM_HEADER_SIZE);
    CHECK_EQ_U64(hm_read_ppm(file, sizeof file - 1, &rgb), 0);
    CHECK_EQ_U64(hm_read_ppm(file, 10, &rgb), 0);

    file[1] = '3';
    CHECK_EQ_U64(hm_read_ppm(file, sizeof file, &rgb), 0);
    file[1] = '6';

    memcpy(file, "P6\n256 239\n255\n", HM_HEADER_SIZE);
    CHECK_EQ_U64(hm_read_ppm(file, sizeof file, &rgb), 0);
}

/* Tiles $30-$39 are the glyphs 0-9 and $01-$06 are A-F. The ROM's CHR starts
   after the 16-byte header and 32 KiB of PRG; each tile is 16 bytes, two bit
   planes of 8 rows, and the font uses pixel value 1 only. */
static int test_glyphs(const char *path)
{
    static uint8_t rom[16 + 32768 + 32768];
    FILE *f = fopen(path, "rb");
    size_t n;
    int g;
    int row;

    if (f == NULL) {
        fprintf(stderr, "cannot open %s\n", path);
        return 1;
    }
    n = fread(rom, 1, sizeof rom, f);
    fclose(f);
    CHECK_EQ_U64(n, sizeof rom);
    for (g = 0; g < 16; g++) {
        int tile = g < 10 ? 0x30 + g : 0x01 + (g - 10);
        const uint8_t *t = rom + 16 + 32768 + tile * 16;
        for (row = 0; row < 8; row++) {
            CHECK_EQ_HEX(t[row] | t[row + 8], hm_glyph[g][row]);
        }
    }
    CHECK_DONE();
}

int main(int argc, char **argv)
{
    if (argc == 3 && strcmp(argv[1], "--glyphs") == 0) {
        return test_glyphs(argv[2]);
    }
    if (argc != 1) {
        fprintf(stderr, "usage: holymapperel.test_decode [--glyphs ROM]\n");
        return 2;
    }
    test_cases();
    test_ppm();
    CHECK_DONE();
}

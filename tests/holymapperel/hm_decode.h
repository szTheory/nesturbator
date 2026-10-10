/* Test-only reader for the Holy Mapperel result screen in a 256x240 P6 image.
   The screen layout is from Holy Mapperel src/main.s at commit
   c022622274ca8b83d214dea97e4388a6b0e92d8a (zlib, (c) Damian Yerrick): the
   line "DETAILED TEST RESULT: " is on nametable row 8, so the anchor "DE" of
   the word DETAILED sits at pixels x 16 and 24, y 64, and the four result
   digits at x 192, 200, 208 and 216, y 64. The shipped runner knows nothing
   of this; the frame is the only input. */
#ifndef NESTURBATOR_HM_DECODE_H
#define NESTURBATOR_HM_DECODE_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define HM_WIDTH 256
#define HM_HEIGHT 240
#define HM_HEADER "P6\n256 240\n255\n"
#define HM_HEADER_SIZE 15u
#define HM_RGB_SIZE ((size_t)HM_WIDTH * HM_HEIGHT * 3u)

#define HM_ROW_Y 64
#define HM_ANCHOR_D_X 16
#define HM_ANCHOR_E_X 24
#define HM_DIGIT_X 192

/* Holy Mapperel font, zlib, (c) Damian Yerrick. Index 0-9 then A-F. Bit 7 is
   the leftmost pixel. Rows 5-7 are blank. These are the combined bit planes of
   tiles $30-$39 and $01-$06 of M3_P32K_C32K_H.nes. */
static const uint8_t hm_glyph[16][8] = {
    {0x3c, 0x66, 0x66, 0x66, 0x3c, 0x00, 0x00, 0x00}, /* 0 */
    {0x18, 0x38, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00}, /* 1 */
    {0x7c, 0x06, 0x3c, 0x60, 0x7e, 0x00, 0x00, 0x00}, /* 2 */
    {0x7c, 0x06, 0x3c, 0x06, 0x7c, 0x00, 0x00, 0x00}, /* 3 */
    {0x1e, 0x36, 0x66, 0x7f, 0x06, 0x00, 0x00, 0x00}, /* 4 */
    {0x7e, 0x60, 0x7c, 0x06, 0x7c, 0x00, 0x00, 0x00}, /* 5 */
    {0x3c, 0x60, 0x7c, 0x66, 0x3c, 0x00, 0x00, 0x00}, /* 6 */
    {0x7e, 0x06, 0x0c, 0x18, 0x18, 0x00, 0x00, 0x00}, /* 7 */
    {0x3c, 0x66, 0x3c, 0x66, 0x3c, 0x00, 0x00, 0x00}, /* 8 */
    {0x3c, 0x66, 0x3e, 0x06, 0x3c, 0x00, 0x00, 0x00}, /* 9 */
    {0x3c, 0x66, 0x7e, 0x66, 0x66, 0x00, 0x00, 0x00}, /* A */
    {0x7c, 0x66, 0x7c, 0x66, 0x7c, 0x00, 0x00, 0x00}, /* B */
    {0x3e, 0x60, 0x60, 0x60, 0x3e, 0x00, 0x00, 0x00}, /* C */
    {0x7c, 0x66, 0x66, 0x66, 0x7c, 0x00, 0x00, 0x00}, /* D */
    {0x7e, 0x60, 0x7c, 0x60, 0x7e, 0x00, 0x00, 0x00}, /* E */
    {0x7e, 0x60, 0x7c, 0x60, 0x60, 0x00, 0x00, 0x00}, /* F */
};

/* Returns 1 and the pixel pointer when buf is exactly "P6\n256 240\n255\n"
   followed by 256*240*3 bytes, else 0. */
static inline int hm_read_ppm(const uint8_t *buf, size_t size, const uint8_t **rgb)
{
    if (size != HM_HEADER_SIZE + HM_RGB_SIZE || memcmp(buf, HM_HEADER, HM_HEADER_SIZE) != 0) {
        return 0;
    }
    *rgb = buf + HM_HEADER_SIZE;
    return 1;
}

/* The 8x8 cell at (x, y): a pixel is lit when its colour differs from the
   cell's (0,0) pixel, so the palette does not matter. Returns the glyph index
   0-15 the cell matches, or -1. */
static inline int hm_cell(const uint8_t *rgb, int x, int y)
{
    const uint8_t *base = rgb + ((size_t)y * HM_WIDTH + (size_t)x) * 3u;
    uint8_t mask[8];
    int row;
    int col;
    int g;
    for (row = 0; row < 8; row++) {
        mask[row] = 0;
        for (col = 0; col < 8; col++) {
            const uint8_t *p = base + ((size_t)row * HM_WIDTH + (size_t)col) * 3u;
            if (memcmp(p, base, 3) != 0) {
                mask[row] = (uint8_t)(mask[row] | (0x80u >> col));
            }
        }
    }
    for (g = 0; g < 16; g++) {
        if (memcmp(mask, hm_glyph[g], 8) == 0) {
            return g;
        }
    }
    return -1;
}

/* Reads the result screen. Returns 2 when the anchor D, E is missing or a
   digit cell matches no glyph (digit[k] is then -1, else 0-15), 0 when the
   code is 0x0000 and 1 for any other code. code holds the four nibbles, first
   digit in the high nibble. */
static inline int hm_decode_digits(const uint8_t *rgb, uint16_t *code, int digit[4])
{
    int k;
    int bad = 0;
    *code = 0;
    if (hm_cell(rgb, HM_ANCHOR_D_X, HM_ROW_Y) != 13 ||
        hm_cell(rgb, HM_ANCHOR_E_X, HM_ROW_Y) != 14) {
        for (k = 0; k < 4; k++) {
            digit[k] = -1;
        }
        return 2;
    }
    for (k = 0; k < 4; k++) {
        digit[k] = hm_cell(rgb, HM_DIGIT_X + 8 * k, HM_ROW_Y);
        if (digit[k] < 0) {
            bad = 1;
        } else {
            *code = (uint16_t)((unsigned)(*code << 4) | (unsigned)digit[k]);
        }
    }
    if (bad) {
        return 2;
    }
    return *code == 0 ? 0 : 1;
}

static inline int hm_decode(const uint8_t *rgb, uint16_t *code)
{
    int digit[4];
    return hm_decode_digits(rgb, code, digit);
}

#endif /* NESTURBATOR_HM_DECODE_H */

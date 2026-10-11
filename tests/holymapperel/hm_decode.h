/* Test-only reader for the Holy Mapperel result screen in a 256x240 P6 image.
   The screen layout is from Holy Mapperel src/main.s at commit
   c022622274ca8b83d214dea97e4388a6b0e92d8a (zlib, (c) Damian Yerrick): the
   line "DETAILED TEST RESULT: " is on nametable row 8, so the anchor "DE" of
   the word DETAILED sits at pixels x 16 and 24, y 64, and the four result
   digits at x 192, 200, 208 and 216, y 64. The PRG RAM line is nametable row 6
   (y 48), whose text starts at column 2 (x 16): "<N>K PRG RAM OK" with
   " + BATTERY" when the save survived, or "PRG RAM MISSING". The shipped
   runner knows nothing of this; the frame is the only input. */
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
#define HM_PRG_RAM_Y 48
#define HM_TEXT_X 16
#define HM_TEXT_CELLS 28

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

/* Letters, space and '+' of the PRG RAM line, from tile (ASCII & $3F) of
   M3_P32K_C32K_H.nes, combined bit planes. The digits 1-9 and the letters C, D,
   F come from hm_glyph. The font draws O and 0 alike, so a 0 reads as O. */
struct hm_char {
    char c;
    uint8_t rows[8];
};

static const struct hm_char hm_text_glyph[] = {
    {' ', {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
    {'+', {0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x00, 0x00}},
    {'A', {0x3c, 0x66, 0x7e, 0x66, 0x66, 0x00, 0x00, 0x00}},
    {'B', {0x7c, 0x66, 0x7c, 0x66, 0x7c, 0x00, 0x00, 0x00}},
    {'E', {0x7e, 0x60, 0x7c, 0x60, 0x7e, 0x00, 0x00, 0x00}},
    {'G', {0x3e, 0x60, 0x6e, 0x66, 0x3e, 0x00, 0x00, 0x00}},
    {'I', {0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00}},
    {'K', {0x66, 0x6c, 0x78, 0x6c, 0x66, 0x00, 0x00, 0x00}},
    {'L', {0x60, 0x60, 0x60, 0x60, 0x7e, 0x00, 0x00, 0x00}},
    {'M', {0xc6, 0xee, 0xfe, 0xd6, 0xc6, 0x00, 0x00, 0x00}},
    {'N', {0x66, 0x76, 0x7e, 0x6e, 0x66, 0x00, 0x00, 0x00}},
    {'O', {0x3c, 0x66, 0x66, 0x66, 0x3c, 0x00, 0x00, 0x00}},
    {'P', {0x7c, 0x66, 0x7c, 0x60, 0x60, 0x00, 0x00, 0x00}},
    {'R', {0x7c, 0x66, 0x7c, 0x6c, 0x66, 0x00, 0x00, 0x00}},
    {'S', {0x3e, 0x60, 0x3c, 0x06, 0x7c, 0x00, 0x00, 0x00}},
    {'T', {0x7e, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00}},
    {'Y', {0x66, 0x3c, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00}},
};
#define HM_TEXT_GLYPHS (sizeof hm_text_glyph / sizeof hm_text_glyph[0])

/* The 8x8 cell at (x, y) as eight row masks, bit 7 leftmost. A pixel is lit
   when its colour differs from the cell's pixel (0,7), row 7 column 0, which
   is blank in every glyph used (the letter M lights pixel (0,0)), so the
   palette does not matter. */
static inline void hm_cell_mask(const uint8_t *rgb, int x, int y, uint8_t mask[8])
{
    const uint8_t *base = rgb + ((size_t)y * HM_WIDTH + (size_t)x) * 3u;
    const uint8_t *ref = base + (size_t)7 * HM_WIDTH * 3u;
    int row;
    int col;
    for (row = 0; row < 8; row++) {
        mask[row] = 0;
        for (col = 0; col < 8; col++) {
            const uint8_t *p = base + ((size_t)row * HM_WIDTH + (size_t)col) * 3u;
            if (memcmp(p, ref, 3) != 0) {
                mask[row] = (uint8_t)(mask[row] | (0x80u >> col));
            }
        }
    }
}

/* Returns the glyph index 0-15 the cell at (x, y) matches, or -1. */
static inline int hm_cell(const uint8_t *rgb, int x, int y)
{
    uint8_t mask[8];
    int g;
    hm_cell_mask(rgb, x, y, mask);
    for (g = 0; g < 16; g++) {
        if (memcmp(mask, hm_glyph[g], 8) == 0) {
            return g;
        }
    }
    return -1;
}

/* Returns the character of the cell at (x, y): a letter, space or '+' from
   hm_text_glyph, else a digit 1-9 or C, D, F from hm_glyph, else '?'. */
static inline char hm_text_cell(const uint8_t *rgb, int x, int y)
{
    uint8_t mask[8];
    size_t i;
    int g;
    hm_cell_mask(rgb, x, y, mask);
    for (i = 0; i < HM_TEXT_GLYPHS; i++) {
        if (memcmp(mask, hm_text_glyph[i].rows, 8) == 0) {
            return hm_text_glyph[i].c;
        }
    }
    for (g = 1; g < 16; g++) {
        if (memcmp(mask, hm_glyph[g], 8) == 0) {
            return "0123456789ABCDEF"[g];
        }
    }
    return '?';
}

/* Reads up to HM_TEXT_CELLS cells of text from x = HM_TEXT_X on pixel row y
   into out (NUL-terminated, at most cap - 1 characters), trailing spaces
   trimmed. An unknown cell is '?'. Returns 1 when no cell is '?', else 0. */
static inline int hm_read_text_row(const uint8_t *rgb, unsigned y, char *out, size_t cap)
{
    size_t n = 0;
    int k;
    int ok = 1;
    if (cap == 0) {
        return 0;
    }
    for (k = 0; k < HM_TEXT_CELLS && n + 1 < cap; k++) {
        char c = hm_text_cell(rgb, HM_TEXT_X + 8 * k, (int)y);
        if (c == '?') {
            ok = 0;
        }
        out[n++] = c;
    }
    while (n > 0 && out[n - 1] == ' ') {
        n--;
    }
    out[n] = '\0';
    return ok;
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

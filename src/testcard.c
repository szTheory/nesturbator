/* The built-in test card shown while no cartridge is loaded (D-01, D-02).
   A pure function of the coordinates: no table, no font, no state. It holds
   no version or text, so its hash never changes with a release (D-03). */
#include "internal.h"

uint16_t nesturbator__test_pixel(uint32_t x, uint32_t y)
{
    /* D-02 rule 1: white ($30) block in the top-left corner only, so a
       horizontal or vertical mirror of the image shows. */
    if (y < 8u && x < 8u) {
        return 0x30u;
    }
    /* D-02 rule 2: white ($30) one-pixel border, so cropping shows. */
    if (y == 0u || y == 239u || x == 0u || x == 255u) {
        return 0x30u;
    }
    /* D-02 rule 3: red ($16) top overscan band and blue ($12) bottom band,
       different colours so a vertical flip shows. */
    if (y < 8u) {
        return 0x16u;
    }
    if (y >= 232u) {
        return 0x12u;
    }
    /* D-02 rule 4: the chart, rows 8-231. Eight bands of 28 rows; band b is
       the emphasis value; inside a band, r (7 rows each) is the high nibble
       of the palette entry and the 16-pixel column c the low nibble. Each of
       the 512 native values appears in its own 16x7 cell. */
    uint32_t b = (y - 8u) / 28u;
    uint32_t r = ((y - 8u) % 28u) / 7u;
    uint32_t c = x / 16u;
    return (uint16_t)(((r << 4) | c) | (b << 6));
}

/* Synthetic background pixel cases with no copyrighted ROM content. */
#include <string.h>

#include "../check.h"
#include "internal.h"

static struct nesturbator nes;
static uint8_t chr[8192];
static uint16_t pixels[NESTURBATOR_WIDTH * NESTURBATOR_HEIGHT];

/* The pipeline fetches pixels from v, so each case starts on the pre-render line with rendering
   on and runs it through to the first two pixels of line 0 (343 dots). */
static void setup(uint8_t extra_mask)
{
    memset(&nes, 0, sizeof nes);
    memset(chr, 0, sizeof chr);
    memset(pixels, 0, sizeof pixels);
    /* The PPU reads pattern data through the board's page map. */
    for (unsigned i = 0u; i < 8u; ++i)
        nes.map.chr_r[i] = chr + i * 1024u;
    nes.ppu.mask = (uint8_t)(0x0au | extra_mask); /* background plus leftmost 8 pixels */
    nes.ppu.scanline = 261u;
    nes.ppu.video_output = pixels;
    nes.ppu.video_pitch = NESTURBATOR_WIDTH;
    nes.ppu.palette[0] = 0x0fu;
    nes.ppu.palette[4] = 0x10u;
    nes.ppu.palette[5] = 0x21u;
    nes.ppu.palette[1] = 0x06u;
    nes.ppu.nametable[0x3c0u] = 1u; /* top-left quadrant selects subpalette 1 */
    chr[0] = 0x08u;                 /* tile 0, row 0 has colour 1 at column 4 */
    nesturbator__ppu_register_write(&nes, 0x2005u, 4u);
    nesturbator__ppu_run_until(&nes, (341u + 2u) * 8u);
}

static void test_fine_scroll_and_attribute_select_native_pixel(void)
{
    setup(0u);
    CHECK_EQ_U64(pixels[0], 0x21u);
    CHECK_EQ_U64(pixels[1], 0x0fu);
}

static void test_grayscale_and_emphasis_stay_in_native_pixel(void)
{
    setup(0xa1u); /* grayscale plus red and blue emphasis */
    CHECK_EQ_U64(pixels[0], (0x21u & 0x30u) | (5u << 6));
}

int main(void)
{
    test_fine_scroll_and_attribute_select_native_pixel();
    test_grayscale_and_emphasis_stay_in_native_pixel();
    CHECK_DONE();
}

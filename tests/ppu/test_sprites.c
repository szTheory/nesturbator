/* Synthetic sprite pipeline cases; no copyrighted ROM content. */
#include <string.h>

#include "../check.h"
#include "internal.h"

static struct nesturbator nes;
static uint8_t chr[8192];
static uint8_t header[16];
static uint16_t pixels[NESTURBATOR_WIDTH * NESTURBATOR_HEIGHT];

static void setup(void)
{
    memset(&nes, 0, sizeof nes);
    memset(chr, 0, sizeof chr);
    memset(header, 0, sizeof header);
    memset(pixels, 0, sizeof pixels);
    nes.cart.bytes = header;
    nes.cart.chr = chr;
    nes.ppu.mask = 0x1eu; /* background, sprites and leftmost pixels */
    nes.ppu.video_output = pixels;
    nes.ppu.video_pitch = NESTURBATOR_WIDTH;
    nes.ppu.palette[0] = 0x0fu;
    nes.ppu.palette[0x11] = 0x2au;
    nes.ppu.oam[0] = 0u;
    nes.ppu.oam[1] = 1u;
    nes.ppu.oam[2] = 0u;
    nes.ppu.oam[3] = 0u;
    chr[0] = 0x80u;  /* background tile 0, row 0, column 0 */
    chr[16] = 0x80u; /* sprite tile 1, row 0, column 0 */
}

static void test_sprite_pixel_is_composed_and_hits_background(void)
{
    setup();
    nesturbator__ppu_run_until(&nes, 341u * 8u + 8u);
    CHECK_EQ_U64(pixels[0], 0x2au);
    CHECK_EQ_U64(nes.ppu.status & 0x40u, 0x40u);
}

static void test_ninth_in_range_sprite_sets_overflow(void)
{
    setup();
    for (uint32_t i = 0; i < 9u; i++)
        nes.ppu.oam[i * 4u] = 0u;
    nesturbator__ppu_run_until(&nes, 341u * 8u);
    CHECK_EQ_U64(nes.ppu.status & 0x20u, 0x20u);
}

static void test_left_clipping_and_x255_hit_boundary(void)
{
    setup();
    nes.ppu.mask &= (uint8_t)~0x04u;
    nesturbator__ppu_run_until(&nes, 341u * 8u + 8u);
    CHECK_EQ_U64(pixels[0], 0u);

    setup();
    nes.ppu.oam[3] = 255u;
    nesturbator__ppu_run_until(&nes, 341u * 8u + 256u * 8u);
    CHECK_EQ_U64(pixels[255], 0x2au);
    CHECK_EQ_U64(nes.ppu.status & 0x40u, 0u);
}

static void test_sprite_is_limited_to_its_eight_pixel_row(void)
{
    setup();
    nesturbator__ppu_run_until(&nes, 2u * 341u * 8u + 8u);
    CHECK_EQ_U64(pixels[NESTURBATOR_WIDTH], 0x0fu);
}

static void test_oam_dma_copies_page_and_wraps_destination(void)
{
    setup();
    for (uint16_t i = 0u; i < 256u; i++)
        nes.bus.ram[i] = (uint8_t)(i ^ 0xa5u);
    nes.ppu.oam_addr = 0xf0u;
    uint64_t before = nes.ticks;
    nesturbator__bus_write(&nes, 0x4014u, 0x00u);
    CHECK_EQ_U64(nes.ticks - before, 24u * 514u);
    CHECK_EQ_U64(nes.ppu.oam[0xf0u], 0xa5u);
    CHECK_EQ_U64(nes.ppu.oam[0xefu], (255u ^ 0xa5u));
    CHECK_EQ_U64(nes.ppu.oam_addr, 0xf0u);
}

int main(void)
{
    test_sprite_pixel_is_composed_and_hits_background();
    test_ninth_in_range_sprite_sets_overflow();
    test_left_clipping_and_x255_hit_boundary();
    test_sprite_is_limited_to_its_eight_pixel_row();
    test_oam_dma_copies_page_and_wraps_destination();
    CHECK_DONE();
}

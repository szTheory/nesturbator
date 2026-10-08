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
    memset(nes.ppu.oam, 0xff, sizeof nes.ppu.oam);
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
    nesturbator__ppu_run_until(&nes, 341u * 8u + 64u);
    CHECK_EQ_U64(pixels[0], 0x2au);
    CHECK_EQ_U64(nes.ppu.status & 0x40u, 0x40u);
}

static void test_ninth_in_range_sprite_sets_overflow(void)
{
    setup();
    for (uint32_t i = 0; i < 8u; i++)
        nes.ppu.oam[i * 4u] = 0u;
    nesturbator__ppu_run_until(&nes, 341u * 8u);
    CHECK_EQ_U64(nes.ppu.status & 0x20u, 0u);

    setup();
    for (uint32_t i = 0; i < 9u; i++)
        nes.ppu.oam[i * 4u] = 0u;
    nesturbator__ppu_run_until(&nes, 341u * 8u);
    CHECK_EQ_U64(nes.ppu.status & 0x20u, 0x20u);
}

static void test_left_clipping_and_x255_hit_boundary(void)
{
    setup();
    nes.ppu.mask &= 0xfbu;
    nesturbator__ppu_run_until(&nes, 341u * 8u + 8u);
    CHECK_EQ_U64(pixels[0], 0u);

    setup();
    nes.ppu.oam[3] = 8u;
    nesturbator__ppu_run_until(&nes, (341u + 9u) * 8u);
    CHECK_EQ_U64(pixels[8], 0x2au);

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

static void test_priority_and_horizontal_flip_select_expected_pixels(void)
{
    setup();
    nes.ppu.palette[1] = 0x21u;
    nes.ppu.oam[2] = 0x20u; /* behind opaque background */
    nesturbator__ppu_run_until(&nes, 341u * 8u + 8u);
    CHECK_EQ_U64(pixels[0], 0x21u);

    setup();
    nes.ppu.oam[2] = 0x40u;
    nesturbator__ppu_run_until(&nes, 341u * 8u + 64u);
    CHECK_EQ_U64(nes.ppu.sprite_attr[0], 0x40u);
    CHECK_EQ_U64(nes.ppu.sprite_lo[0], 0x01u);
    CHECK_EQ_U64(pixels[0], 0u);
    CHECK_EQ_U64(pixels[7], 0x2au);
}

static void test_8x16_selects_pattern_table_and_vertical_flip(void)
{
    setup();
    nes.ppu.control = 0x20u;
    nes.ppu.oam[1] = 3u;    /* table 1, even tile 2 */
    nes.ppu.oam[2] = 0x80u; /* row 0 selects source row 15 */
    chr[0x1000u + 32u + 16u + 7u] = 0x80u;
    nesturbator__ppu_run_until(&nes, 341u * 8u + 8u);
    CHECK_EQ_U64(pixels[0], 0x2au);
}

static void test_oam_dma_copies_page_and_wraps_destination(void)
{
    setup();
    for (uint16_t i = 0u; i < 256u; i++)
        nes.bus.ram[i] = (uint8_t)(i ^ 0xa5u);
    nes.ppu.oam_addr = 0xf0u;
    uint64_t before = nes.ticks;
    nesturbator__bus_write(&nes, 0x4014u, 0x00u);
    CHECK_EQ_U64(nes.ticks - before, 24u);
    CHECK_EQ_U64(nes.ppu.oam[0xf0u], 0xffu);
    (void)nesturbator__bus_read(&nes, 0x0000u);
    CHECK_EQ_U64(nes.ticks - before, 24u * 515u);
    CHECK_EQ_U64(nes.ppu.oam[0xf0u], 0xa5u);
    CHECK_EQ_U64(nes.ppu.oam[0xefu], (255u ^ 0xa5u));
    CHECK_EQ_U64(nes.ppu.oam_addr, 0xf0u);

    setup();
    for (uint16_t i = 0u; i < 256u; i++)
        nes.bus.ram[i] = (uint8_t)(i ^ 0xa5u);
    nes.ticks = 24u;
    nes.ppu.ppu_ticks = 24u;
    before = nes.ticks;
    nesturbator__bus_write(&nes, 0x4014u, 0x00u);
    CHECK_EQ_U64(nes.ticks - before, 24u);
    (void)nesturbator__bus_read(&nes, 0x0000u);
    CHECK_EQ_U64(nes.ticks - before, 24u * 516u);
    CHECK_EQ_U64(nes.ppu.oam[0], 0xa5u);
    CHECK_EQ_U64(nes.ppu.oam[255], (255u ^ 0xa5u));
    CHECK_EQ_U64(nes.ppu.ppu_ticks, nes.ticks);
}

static void test_dma_keeps_ppu_vblank_timing_while_stalling_cpu(void)
{
    setup();
    nes.ppu.scanline = 240u;
    nesturbator__bus_write(&nes, 0x4014u, 0x00u);
    CHECK_EQ_U64(nes.ppu.status & 0x80u, 0u);
    (void)nesturbator__bus_read(&nes, 0x0000u);
    CHECK_EQ_U64(nes.ppu.status & 0x80u, 0x80u);
    CHECK_EQ_U64(nes.ppu.ppu_ticks, nes.ticks);
}

int main(void)
{
    test_sprite_pixel_is_composed_and_hits_background();
    test_ninth_in_range_sprite_sets_overflow();
    test_left_clipping_and_x255_hit_boundary();
    test_sprite_is_limited_to_its_eight_pixel_row();
    test_priority_and_horizontal_flip_select_expected_pixels();
    test_8x16_selects_pattern_table_and_vertical_flip();
    test_oam_dma_copies_page_and_wraps_destination();
    test_dma_keeps_ppu_vblank_timing_while_stalling_cpu();
    CHECK_DONE();
}

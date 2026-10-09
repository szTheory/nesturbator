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
    nes.ppu.scanline = 261u;
    nes.cart.bytes = header;
    nes.cart.chr = chr;
    nes.ppu.mask = 0x1eu; /* background, sprites and leftmost pixels */
    nes.ppu.video_output = pixels;
    nes.ppu.video_pitch = NESTURBATOR_WIDTH;
    nes.ppu.palette[0] = 0x0fu;
    nes.ppu.palette[0x11] = 0x2au;
    nes.ppu.oam[0] = 0xffu;
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

static void test_prerender_wraps_sprite_rows_into_visible_scanline_zero(void)
{
    for (uint8_t odd = 0u; odd < 2u; odd++) {
        setup();
        chr[0] = 0u;
        nes.ppu.scanline = 261u;
        nes.ppu.odd_frame = odd;
        nes.ppu.oam[0] = 0xffu;
        nes.ppu.oam[3] = 16u;

        uint64_t first_row_dots = odd != 0u ? 340u + 18u : 341u + 18u;
        nesturbator__ppu_run_until(&nes, first_row_dots * 8u);
        CHECK_EQ_U64(pixels[16], 0x2au);
        CHECK_EQ_U64(pixels[17], 0x0fu);

        uint64_t second_row_dots = first_row_dots + 341u;
        nesturbator__ppu_run_until(&nes, second_row_dots * 8u);
        CHECK_EQ_U64(pixels[NESTURBATOR_WIDTH + 16u], 0x0fu);

        setup();
        chr[0] = 0u;
        nes.ppu.scanline = 261u;
        nes.ppu.odd_frame = odd;
        nes.ppu.oam[0] = 0u;
        nes.ppu.oam[3] = 16u;
        nesturbator__ppu_run_until(&nes, first_row_dots * 8u);
        CHECK_EQ_U64(pixels[16], 0x0fu);
        nesturbator__ppu_run_until(&nes, second_row_dots * 8u);
        CHECK_EQ_U64(pixels[NESTURBATOR_WIDTH + 16u], 0x2au);
    }
}

static void test_ninth_in_range_sprite_sets_overflow(void)
{
    setup();
    nes.ppu.scanline = 0u;
    for (uint32_t i = 0u; i < 64u; i++)
        nes.ppu.oam[i * 4u] = 8u;
    for (uint32_t i = 0; i < 8u; i++)
        nes.ppu.oam[i * 4u] = 0u;
    nesturbator__ppu_run_until(&nes, 257u * 8u);
    CHECK_EQ_U64(nes.ppu.status & 0x20u, 0u);

    setup();
    nes.ppu.scanline = 0u;
    for (uint32_t i = 0u; i < 64u; i++)
        nes.ppu.oam[i * 4u] = 8u;
    for (uint32_t i = 0; i < 9u; i++)
        nes.ppu.oam[i * 4u] = 0u;
    nesturbator__ppu_run_until(&nes, 257u * 8u);
    CHECK_EQ_U64(nes.ppu.status & 0x20u, 0x20u);
}

static void setup_diagonal_overflow(void)
{
    setup();
    nes.ppu.scanline = 0u;
    for (uint32_t i = 0u; i < 8u; i++)
        nes.ppu.oam[i * 4u] = 0u;
}

static void test_diagonal_overflow_compares_non_y_bytes(void)
{
    /* The ninth Y is read at dots 81/82. Each miss advances both the
       primary-OAM entry and byte index; byte index 3 wraps to 0. */
    const uint8_t compared_entries[] = {9u, 10u, 11u, 12u};
    const uint8_t compared_bytes[] = {1u, 2u, 3u, 0u};
    for (uint8_t case_index = 0u; case_index < 4u; case_index++) {
        setup_diagonal_overflow();
        uint32_t address = (uint32_t)compared_entries[case_index] * 4u +
                           compared_bytes[case_index];
        nes.ppu.oam[address] = 0u;
        uint64_t comparison_dot = (uint64_t)(84u + 2u * case_index);
        for (uint8_t step = 0u; step <= case_index + 1u; step++) {
            uint64_t read_dot = 81u + 2u * step;
            nesturbator__ppu_run_until(&nes, read_dot * 8u);
            CHECK_EQ_U64(nes.ppu.eval_n, 8u + step);
            CHECK_EQ_U64(nes.ppu.eval_m, step & 3u);
            CHECK_EQ_U64(nes.ppu.eval_latch,
                         step == case_index + 1u ? 0u : 0xffu);
            CHECK_EQ_U64(nes.ppu.status & 0x20u, 0u);
            if (read_dot + 1u < comparison_dot)
                nesturbator__ppu_run_until(&nes, (read_dot + 1u) * 8u);
        }
        nesturbator__ppu_run_until(&nes, comparison_dot * 8u);
        CHECK_EQ_U64(nes.ppu.status & 0x20u, 0x20u);
        CHECK_EQ_U64(nes.ppu.eval_count, 8u);
        if (case_index == 2u) {
            CHECK_EQ_U64(nes.ppu.eval_n, 12u);
            CHECK_EQ_U64(nes.ppu.eval_m, 0u);
        }
        nesturbator__ppu_run_until(&nes, 256u * 8u);
        CHECK_EQ_U64(nes.ppu.status & 0x20u, 0x20u);
    }
}

static void test_diagonal_overflow_skips_in_range_y(void)
{
    setup_diagonal_overflow();
    nes.ppu.oam[9u * 4u] = 0u; /* This Y is skipped after entry 8 misses. */
    nesturbator__ppu_run_until(&nes, 256u * 8u);
    CHECK_EQ_U64(nes.ppu.status & 0x20u, 0u);
    CHECK_EQ_U64(nes.ppu.eval_count, 8u);

    setup_diagonal_overflow();
    nes.ppu.oam[8u * 4u] = 0u;
    nesturbator__ppu_run_until(&nes, 81u * 8u);
    CHECK_EQ_U64(nes.ppu.status & 0x20u, 0u);
    nesturbator__ppu_run_until(&nes, 82u * 8u);
    CHECK_EQ_U64(nes.ppu.status & 0x20u, 0x20u);
}

static void test_diagonal_overflow_clears_on_prerender(void)
{
    setup_diagonal_overflow();
    nes.ppu.oam[9u * 4u + 1u] = 0u;
    nesturbator__ppu_run_until(&nes, (341u * 261u) * 8u);
    CHECK_EQ_U64(nes.ppu.status & 0x20u, 0x20u);
    nesturbator__ppu_run_until(&nes, (341u * 261u + 1u) * 8u);
    CHECK_EQ_U64(nes.ppu.scanline, 261u);
    CHECK_EQ_U64(nes.ppu.status & 0x20u, 0u);
    nesturbator__ppu_run_until(&nes, (341u * 262u) * 8u);
    CHECK_EQ_U64(nes.ppu.status & 0x20u, 0u);
    CHECK_EQ_U64(nes.ppu.eval_count, 8u);
    nesturbator__ppu_run_until(&nes, (341u * 262u + 84u) * 8u);
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
    test_prerender_wraps_sprite_rows_into_visible_scanline_zero();
    test_sprite_pixel_is_composed_and_hits_background();
    test_ninth_in_range_sprite_sets_overflow();
    test_diagonal_overflow_compares_non_y_bytes();
    test_diagonal_overflow_skips_in_range_y();
    test_diagonal_overflow_clears_on_prerender();
    test_left_clipping_and_x255_hit_boundary();
    test_sprite_is_limited_to_its_eight_pixel_row();
    test_priority_and_horizontal_flip_select_expected_pixels();
    test_8x16_selects_pattern_table_and_vertical_flip();
    test_oam_dma_copies_page_and_wraps_destination();
    test_dma_keeps_ppu_vblank_timing_while_stalling_cpu();
    CHECK_DONE();
}

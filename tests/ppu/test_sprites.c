/* Synthetic sprite pipeline cases; no copyrighted ROM content. */
#include <string.h>

#include "../check.h"
#include "internal.h"

static struct nesturbator nes;
static uint8_t chr[8192];
static uint16_t pixels[NESTURBATOR_WIDTH * NESTURBATOR_HEIGHT];

static void setup(void)
{
    memset(&nes, 0, sizeof nes);
    memset(nes.ppu.oam, 0xff, sizeof nes.ppu.oam);
    memset(chr, 0, sizeof chr);
    memset(pixels, 0, sizeof pixels);
    nes.ppu.scanline = 261u;
    /* The PPU reads pattern data through the board's page map. */
    for (unsigned i = 0u; i < 8u; ++i)
        nes.map.chr_r[i] = chr + i * 1024u;
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
    /* Eight selected sprites take eight dots each. The ninth Y is read at
       dots 129/130; after its miss, the diagonal walk advances both n and m. */
    const uint8_t compared_entries[] = {9u, 10u, 11u, 12u};
    const uint8_t compared_bytes[] = {1u, 2u, 3u, 0u};
    for (uint8_t case_index = 0u; case_index < 4u; case_index++) {
        setup_diagonal_overflow();
        uint32_t address = (uint32_t)compared_entries[case_index] * 4u;
        address += compared_bytes[case_index];
        nes.ppu.oam[address] = 0u;
        uint64_t comparison_dot = (uint64_t)(133u + 2u * case_index);
        for (uint8_t step = 0u; step <= case_index + 1u; step++) {
            uint64_t read_dot = 129u + 2u * step;
            nesturbator__ppu_run_until(&nes, read_dot * 8u);
            CHECK_EQ_U64(nes.ppu.eval_n, 8u + step);
            CHECK_EQ_U64(nes.ppu.eval_m, step & 3u);
            CHECK_EQ_U64(nes.ppu.eval_latch, step == case_index + 1u ? 0u : 0xffu);
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
    nesturbator__ppu_run_until(&nes, 129u * 8u);
    CHECK_EQ_U64(nesturbator__ppu_register_read(&nes, 0x2002u) & 0x20u, 0u);
    nesturbator__ppu_run_until(&nes, 130u * 8u);
    CHECK_EQ_U64(nesturbator__ppu_register_read(&nes, 0x2002u) & 0x20u, 0u);
    nesturbator__ppu_run_until(&nes, 131u * 8u);
    CHECK_EQ_U64(nesturbator__ppu_register_read(&nes, 0x2002u) & 0x20u, 0x20u);
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
    nesturbator__ppu_run_until(&nes, (341u * 262u + 133u) * 8u);
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
    /* Rendering is off so OAMADDR is not reset during tile loading while the DMA runs.
       (derived: rendering off keeps OAMADDR; NESdev PPU registers) */
    setup();
    nes.ppu.mask = 0u;
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
    nes.ppu.mask = 0u; /* (derived: rendering off keeps OAMADDR) */
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

/* ---- Owned OAMADDR and sprite-0 cases (Phase 19, D-10) ----
   Expected values are constants taken from NESdev text and AccuracyCoin error codes, labelled
   "(derived ...)"; none is read back from this PPU. Cases marked "reproducer" fail on a core that
   never clears OAMADDR during rendering and always evaluates from OAM[0]; cases marked "guard"
   pass on that core by design and stop an over-broad fix. */

/* Opaque background everywhere: tile 0 rows 0-7 have plane 0 set on every column. */
static void make_background_opaque(void)
{
    memset(chr, 0xff, 8u);
}

/* Reproducer. (derived: NESdev PPU registers, OAMADDR is set to 0 during ticks 257-320 of the
   pre-render and visible scanlines) */
static void test_oamaddr_cleared_after_dot_320_with_rendering(void)
{
    setup();
    nes.ppu.scanline = 0u;
    nes.ppu.oam_addr = 0x81u;
    nesturbator__ppu_run_until(&nes, 321u * 8u);
    CHECK_EQ_U64(nes.ppu.oam_addr, 0u);

    /* The pre-render line clears it too. */
    setup();
    nes.ppu.oam_addr = 0x81u;
    nesturbator__ppu_run_until(&nes, 321u * 8u);
    CHECK_EQ_U64(nes.ppu.oam_addr, 0u);
}

/* Guard. (derived: rendering off keeps OAMADDR; NESdev PPU registers limits the reset to the
   tile loading interval while rendering) */
static void test_oamaddr_kept_with_rendering_off(void)
{
    setup();
    nes.ppu.mask = 0u;
    nes.ppu.oam_addr = 0x81u;
    nesturbator__ppu_run_until(&nes, 341u * 262u * 8u);
    CHECK_EQ_U64(nes.ppu.oam_addr, 0x81u);
}

/* Reproducer. (derived: NESdev PPU registers, OAMADDR is set to 0 during each of ticks 257-320,
   so a write at dot 300 is cleared again on the next tile-loading dot) */
static void test_oamaddr_write_at_dot_300_is_cleared(void)
{
    setup();
    nes.ppu.scanline = 0u;
    nesturbator__ppu_run_until(&nes, 300u * 8u);
    nesturbator__ppu_register_write(&nes, 0x2003u, 0x77u);
    nesturbator__ppu_run_until(&nes, 321u * 8u);
    CHECK_EQ_U64(nes.ppu.oam_addr, 0u);
}

/* Guard. (derived: NESdev PPU registers, the pre-render line clears OAMADDR, so a VBlank
   $2003 write does not move where line 0 evaluation starts) Sprite zero for line 1 is OAM[0]. */
static void test_vblank_oamaddr_write_reset_before_line_0(void)
{
    setup();
    nes.ppu.scanline = 245u;
    nes.ppu.oam[0] = 0u; /* Y 0: in range for line 1 */
    nes.ppu.oam[1] = 1u;
    nes.ppu.oam[2] = 0u;
    nes.ppu.oam[3] = 0u;
    nesturbator__ppu_register_write(&nes, 0x2003u, 0x80u);
    nesturbator__ppu_run_until(&nes, (341u * 17u + 257u) * 8u); /* line 0, past dot 256 */
    CHECK_EQ_U64(nes.ppu.scanline, 0u);
    CHECK_EQ_U64(nes.ppu.eval_count, 1u);
    CHECK_EQ_U64(nes.ppu.eval_sprite_zero[0], 1u);
    CHECK_EQ_U64(nes.ppu.secondary_oam[0], 0u);
}

/* Reproducer. (derived: NESdev PPU registers, OAMADDR at dot 65 sets the evaluation start and
   the sprite there is treated as sprite 0.) OAMADDR $80 starts at OAM sprite 32. A copy at
   OAM[0] is not evaluated and so does not hit. */
static void test_oamaddr_80_makes_sprite_32_sprite_zero(void)
{
    setup();
    make_background_opaque();
    chr[32] = 0x80u; /* tile 2 row 0, column 0 */
    nes.ppu.scanline = 0u;
    nes.ppu.oam_addr = 0x80u;
    nes.ppu.oam[0] = 0u; /* copy of a sprite at OAM[0]: tile 2 at x 0 */
    nes.ppu.oam[1] = 2u;
    nes.ppu.oam[2] = 0u;
    nes.ppu.oam[3] = 0u;
    nes.ppu.oam[0x80] = 0u; /* sprite 32: tile 1 at x 100 */
    nes.ppu.oam[0x81] = 1u;
    nes.ppu.oam[0x82] = 0u;
    nes.ppu.oam[0x83] = 100u;
    nesturbator__ppu_run_until(&nes, (341u + 9u) * 8u); /* line 1, x 0-7 drawn */
    CHECK_EQ_U64(nes.ppu.status & 0x40u, 0u);
    nesturbator__ppu_run_until(&nes, (341u + 102u) * 8u); /* line 1, x 100 drawn */
    CHECK_EQ_U64(nes.ppu.status & 0x40u, 0x40u);
}

/* Reproducer. (derived: NESdev PPU registers, a misaligned OAMADDR is reinterpreted as a Y
   position and the following bytes are copied in order to secondary OAM.) OAMADDR $81 reads
   OAM[$81] as Y, then OAM[$82..$84] as tile, attribute and X. */
static void test_misaligned_oamaddr_81_copies_bytes_in_order(void)
{
    setup();
    nes.ppu.scanline = 0u;
    nes.ppu.oam_addr = 0x81u;
    nes.ppu.oam[0x81] = 0u;  /* Y: in range for line 1 */
    nes.ppu.oam[0x82] = 1u;  /* tile 1, row 0 is 0x80 */
    nes.ppu.oam[0x83] = 0u;  /* attribute */
    nes.ppu.oam[0x84] = 16u; /* X */
    nesturbator__ppu_run_until(&nes, 321u * 8u);
    CHECK_EQ_U64(nes.ppu.sprite_lo[0], 0x80u);
    CHECK_EQ_U64(nes.ppu.sprite_attr[0], 0u);
    CHECK_EQ_U64(nes.ppu.sprite_x[0], 16u);
}

/* Reproducer. (derived: NESdev PPU registers, evaluation stops at the end of OAM.) OAMADDR $FD
   reads OAM[$FD] as Y and OAM[$FE], OAM[$FF] as tile and attribute; the walk ends at the end of
   OAM and never indexes past it (checked under asan). */
static void test_oamaddr_fd_ends_at_end_of_oam(void)
{
    setup();
    nes.ppu.scanline = 0u;
    nes.ppu.oam_addr = 0xfdu;
    nes.ppu.oam[0u] = 0xffu;
    nes.ppu.oam[0xfd] = 0u; /* Y: in range for line 1 */
    nes.ppu.oam[0xfe] = 1u; /* tile 1 */
    nes.ppu.oam[0xff] = 0u; /* attribute */
    nesturbator__ppu_run_until(&nes, 321u * 8u);
    CHECK_EQ_U64(nes.ppu.eval_count, 1u);
    CHECK_EQ_U64(nes.ppu.sprite_lo[0], 0x80u);
    CHECK_EQ_U64(nes.ppu.sprite_attr[0], 0u);
}

/* Reproducer. (derived: NESdev PPU OAM, a sprite is in range only from its Y; AccuracyCoin
   Arbitrary Sprite zero error 2, the identity follows OAMADDR.) OAMADDR $80 with OAM[$80] out of
   range finds nothing: the in-range sprite at OAM[0] is never evaluated, so there is no hit. */
static void test_first_read_out_of_range_gives_no_sprite_zero(void)
{
    setup();
    make_background_opaque();
    nes.ppu.scanline = 0u;
    nes.ppu.oam_addr = 0x80u;
    nes.ppu.oam[0] = 0u; /* in range for line 1 */
    nes.ppu.oam[1] = 1u;
    nes.ppu.oam[2] = 0u;
    nes.ppu.oam[3] = 0u;
    nesturbator__ppu_run_until(&nes, 257u * 8u);
    CHECK_EQ_U64(nes.ppu.eval_count, 0u);
    nesturbator__ppu_run_until(&nes, 2u * 341u * 8u);
    CHECK_EQ_U64(nes.ppu.status & 0x40u, 0u);
}

/* Guard. (derived: NESdev PPU OAM, sprite 0 hit does not depend on sprite priority.) Sprite zero
   behind the background, with a second opaque sprite over it, still sets the hit. */
static void test_covered_sprite_zero_still_hits(void)
{
    setup();
    nes.ppu.oam[2] = 0x20u; /* sprite zero behind the background */
    nes.ppu.oam[4] = 0xffu; /* a second opaque sprite at the same place */
    nes.ppu.oam[5] = 1u;
    nes.ppu.oam[6] = 0u;
    nes.ppu.oam[7] = 0u;
    nesturbator__ppu_run_until(&nes, 341u * 8u + 64u);
    CHECK_EQ_U64(nes.ppu.status & 0x40u, 0x40u);
}

/* Guard. (derived: NESdev PPU OAM, sprite 0 hit conditions: the hit never sets at x = 255, so
   the last pixel that can hit is x = 254.) */
static void test_hit_on_dot_x254_not_x255(void)
{
    setup();
    make_background_opaque();
    nes.ppu.oam[3] = 254u;
    nesturbator__ppu_run_until(&nes, 341u * 8u + 254u * 8u); /* pixel 253 drawn */
    CHECK_EQ_U64(nes.ppu.status & 0x40u, 0u);
    nesturbator__ppu_run_until(&nes, 341u * 8u + 255u * 8u); /* pixel 254 drawn */
    CHECK_EQ_U64(nes.ppu.status & 0x40u, 0u);
    nesturbator__ppu_run_until(&nes, 341u * 8u + 256u * 8u); /* hit signal reaches status */
    CHECK_EQ_U64(nes.ppu.status & 0x40u, 0x40u);

    setup();
    make_background_opaque();
    nes.ppu.oam[3] = 255u;
    nesturbator__ppu_run_until(&nes, 341u * 8u + 256u * 8u); /* pixel 255 drawn */
    CHECK_EQ_U64(pixels[255], 0x2au);
    CHECK_EQ_U64(nes.ppu.status & 0x40u, 0u);
}

/* Guard. (derived: NESdev PPU OAM, a sprite with Y = $FF is never in range on a visible line.)
   Every OAM byte is $FF; no hit is set on any line of a whole rendered frame. */
static void test_all_oam_y_ff_gives_no_hit(void)
{
    setup();
    make_background_opaque();
    memset(nes.ppu.oam, 0xff, sizeof nes.ppu.oam);
    for (uint32_t line = 0u; line < 240u; line++) {
        nesturbator__ppu_run_until(&nes, 341u * (uint64_t)(line + 1u) * 8u);
        CHECK_EQ_U64(nes.ppu.status & 0x40u, 0u);
    }
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
    test_oamaddr_cleared_after_dot_320_with_rendering();
    test_oamaddr_kept_with_rendering_off();
    test_oamaddr_write_at_dot_300_is_cleared();
    test_vblank_oamaddr_write_reset_before_line_0();
    test_oamaddr_80_makes_sprite_32_sprite_zero();
    test_misaligned_oamaddr_81_copies_bytes_in_order();
    test_oamaddr_fd_ends_at_end_of_oam();
    test_first_read_out_of_range_gives_no_sprite_zero();
    test_covered_sprite_zero_still_hits();
    test_hit_on_dot_x254_not_x255();
    test_all_oam_y_ff_gives_no_hit();
    CHECK_DONE();
}

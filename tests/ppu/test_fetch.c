/* MAP-02, D-13: the fetch pipeline dot by dot. Every case loads a synthetic NROM fixture, sets
   the PPU position and registers directly, and single-steps with
   nesturbator__ppu_run_until(nes, ppu_ticks + 8). "After dot N" means the step that leaves
   ppu.dot == N. Expected values come from the NESdev Wiki "PPU scrolling" and "PPU rendering"
   formulas, written out in each comment; they are not copied from the implementation. The
   test board logs PPU address bus bit-12 edges as (level, tick). The dots here are 1-based; the
   MMC3 page's 260 and 324 are 0-based. */
#include <string.h>

#include "internal.h"
#include "../check.h"
#include "../mapper_test.h"
#include "ppu_fixture.h"

static void step(struct nesturbator *nes)
{
    nesturbator__ppu_run_until(nes, nes->ppu.ppu_ticks + 8u);
}

/* Steps until the PPU stands after (line, dot). Bounded so a wrong position cannot hang. */
static void step_to(struct nesturbator *nes, uint16_t line, uint16_t dot)
{
    for (unsigned guard = 0u; guard < 200000u; guard++) {
        if (nes->ppu.scanline == line && nes->ppu.dot == dot)
            return;
        step(nes);
    }
    CHECK(0);
}

/* A fixture with OAM full of $FF (no sprite falls on any line under test), the given PPUCTRL
   and PPUMASK, and the PPU standing after (line, dot). */
static struct nesturbator *start(uint8_t vertical, uint8_t control, uint8_t mask, uint16_t line,
                                 uint16_t dot)
{
    struct nesturbator *nes = ppu_fixture_load(vertical, 0u);
    memset(nes->ppu.oam, 0xff, sizeof nes->ppu.oam);
    nes->ppu.control = control;
    nes->ppu.mask = mask;
    nes->ppu.scanline = line;
    nes->ppu.dot = dot;
    return nes;
}

static void watch_a12(struct nesturbator *nes)
{
    mapper_test_install(nes, NESTURBATOR_WATCH_PPU_A12);
}

/* The log grew by exactly one edge since `before`, of the given level, stamped with the tick of
   the dot just processed. */
static void check_new_edge(const struct nesturbator *nes, unsigned before, uint8_t level)
{
    CHECK_EQ_U64(mapper_test_edge_count, before + 1u);
    if (mapper_test_edge_count == before + 1u) {
        CHECK_EQ_U64(mapper_test_edges[before].level, level);
        CHECK_EQ_U64(mapper_test_edges[before].tick, nes->ppu.ppu_ticks);
    }
}

/* PPU scrolling "Coarse X increment": the horizontal position of v is incremented at dots 8, 16,
   ..., 256, and a carry out of coarse X 31 toggles bit 10. */
static void test_coarse_x_increments(void)
{
    struct nesturbator *nes = start(0u, 0u, 0x18u, 0u, 0u);
    nes->ppu.v = 0x0000u;
    step_to(nes, 0u, 7u);
    CHECK_EQ_HEX(nes->ppu.v, 0x0000u);
    step(nes);
    CHECK_EQ_HEX(nes->ppu.v, 0x0001u);
    step_to(nes, 0u, 16u);
    CHECK_EQ_HEX(nes->ppu.v, 0x0002u);
    ppu_fixture_free(nes);

    nes = start(0u, 0u, 0x18u, 0u, 0u);
    step_to(nes, 0u, 7u);
    nes->ppu.v = 0x001fu;
    step(nes);
    CHECK_EQ_HEX(nes->ppu.v, 0x0400u); /* coarse X 31 wraps to 0 and toggles bit 10 */
    ppu_fixture_free(nes);
}

/* PPU scrolling "Y increment" at dot 256, after the coarse X increment of the same dot:
   fine Y below 7 adds 1; at 7 it clears and coarse Y 29 wraps with a bit 11 toggle, 31 wraps
   without one, otherwise coarse Y adds 1. */
static void check_y_increment(uint16_t before, uint16_t expected)
{
    struct nesturbator *nes = start(0u, 0u, 0x18u, 0u, 0u);
    step_to(nes, 0u, 255u);
    nes->ppu.v = before;
    step(nes);
    CHECK_EQ_HEX(nes->ppu.v, expected);
    ppu_fixture_free(nes);
}

static void test_y_increment_at_256(void)
{
    /* fine Y 7, coarse Y 29, coarse X 5: X becomes 6, fine Y 0, coarse Y 0, bit 11 toggles. */
    check_y_increment(0x73a5u, 0x0806u);
    /* coarse Y 31 wraps to 0 with no toggle. */
    check_y_increment(0x73e5u, 0x0006u);
    /* fine Y 2 simply becomes 3. */
    check_y_increment(0x2005u, 0x3006u);
}

/* PPU scrolling: at dot 257, v bits 0-4 and 10 are copied from t. */
static void test_horizontal_copy_at_257(void)
{
    struct nesturbator *nes = start(0u, 0u, 0x18u, 0u, 0u);
    nes->ppu.t = 0x041fu;
    step_to(nes, 0u, 256u);
    nes->ppu.v = 0x0000u;
    step(nes);
    CHECK_EQ_HEX(nes->ppu.v, 0x041fu);
    ppu_fixture_free(nes);

    /* No vertical copy on a visible line: v is untouched through dot 305. */
    nes = start(0u, 0u, 0x18u, 0u, 0u);
    nes->ppu.t = 0x6ba0u;
    step_to(nes, 0u, 279u);
    nes->ppu.v = 0x7be0u;
    step_to(nes, 0u, 305u);
    CHECK_EQ_HEX(nes->ppu.v, 0x7be0u);
    ppu_fixture_free(nes);
}

/* PPU scrolling: on the pre-render line, dots 280 through 304 copy v bits 5-9 and 11-14 from t
   on every dot. */
static void test_vertical_copy_on_prerender_only(void)
{
    struct nesturbator *nes = start(0u, 0u, 0x18u, 261u, 270u);
    nes->ppu.t = 0x6ba0u;
    step_to(nes, 261u, 278u);
    nes->ppu.v = 0x0000u;
    step(nes); /* dot 279 */
    CHECK_EQ_HEX(nes->ppu.v & 0x7be0u, 0x0000u);
    step(nes); /* dot 280 */
    CHECK_EQ_HEX(nes->ppu.v & 0x7be0u, 0x6ba0u);
    step_to(nes, 261u, 290u);
    nes->ppu.v = 0x0000u;
    step(nes); /* dot 291 copies again */
    CHECK_EQ_HEX(nes->ppu.v & 0x7be0u, 0x6ba0u);
    step_to(nes, 261u, 304u);
    nes->ppu.v = 0x0000u;
    step(nes); /* dot 305 does not */
    CHECK_EQ_HEX(nes->ppu.v, 0x0000u);
    ppu_fixture_free(nes);
}

/* PPU scrolling "Tile and attribute fetching": the nametable address is 0x2000 | (v & 0x0FFF),
   the attribute address 0x23C0 | (v & 0x0C00) | ((v >> 4) & 0x38) | ((v >> 2) & 0x07), and the
   pattern address is the PPUCTRL bit 4 table + tile * 16 + fine Y (+ 8 for the high plane). */
static void check_addresses(uint8_t control, uint16_t lo, uint16_t hi)
{
    struct nesturbator *nes = start(0u, control, 0x18u, 0u, 0u);
    nes->ppu.v = 0x10a2u; /* fine Y 1, coarse Y 5, coarse X 2 */
    nesturbator__ppu_write(nes, 0x20a2u, 0x35u);
    step_to(nes, 0u, 1u);
    CHECK_EQ_HEX(nes->ppu.bus_addr, 0x20a2u);
    step_to(nes, 0u, 2u);
    CHECK_EQ_HEX(nes->ppu.bg_nt, 0x35u);
    step_to(nes, 0u, 3u);
    /* 0x23C0 | 0 | ((0x10A2 >> 4) & 0x38 = 0x08) | ((0x10A2 >> 2) & 7 = 0) */
    CHECK_EQ_HEX(nes->ppu.bus_addr, 0x23c8u);
    step_to(nes, 0u, 5u);
    CHECK_EQ_HEX(nes->ppu.bus_addr, lo);
    step_to(nes, 0u, 7u);
    CHECK_EQ_HEX(nes->ppu.bus_addr, hi);
    ppu_fixture_free(nes);
}

static void test_fetch_addresses(void)
{
    check_addresses(0x00u, 0x0351u, 0x0359u); /* 0x35 * 16 + fine Y 1 */
    check_addresses(0x10u, 0x1351u, 0x1359u);
}

/* PPU rendering: the low pattern plane shifts in 0 and the high plane shifts in 1; the shifters
   are frozen outside dots 2-257 and 322-337. */
static void test_shifter_polarity_and_hold(void)
{
    struct nesturbator *nes = start(0u, 0u, 0x18u, 0u, 0u);
    step_to(nes, 0u, 1u);
    nes->ppu.bg_shift_lo = 0u;
    nes->ppu.bg_shift_hi = 0u;
    step(nes);
    CHECK_EQ_U64(nes->ppu.bg_shift_lo & 1u, 0u);
    CHECK_EQ_U64(nes->ppu.bg_shift_hi & 1u, 1u);
    step_to(nes, 0u, 257u);
    nes->ppu.bg_shift_lo = 0xa5a5u;
    nes->ppu.bg_shift_hi = 0x5a5au;
    nes->ppu.at_shift_lo = 0x3cu;
    nes->ppu.at_shift_hi = 0xc3u;
    step_to(nes, 0u, 321u);
    CHECK_EQ_HEX(nes->ppu.bg_shift_lo, 0xa5a5u);
    CHECK_EQ_HEX(nes->ppu.bg_shift_hi, 0x5a5au);
    CHECK_EQ_HEX(nes->ppu.at_shift_lo, 0x3cu);
    CHECK_EQ_HEX(nes->ppu.at_shift_hi, 0xc3u);
    ppu_fixture_free(nes);
}

/* PPU rendering with BG at $1000 and sprites at $0000 and none on the line: A12 rises on the
   pattern-low address dots 5, 13, ..., 253, falls on the nametable dots 9, 17, ..., 257, rises
   at 325 and 333 (the 0-based 324 and 332 of the MMC3 page) and falls at 329 and 337; the next
   line's dot 0 drives the first tile's low pattern address and rises. -1 is no edge. */
static int expected_edge_bg_at_1000(unsigned n)
{
    if ((n >= 5u && n <= 253u && (n - 5u) % 8u == 0u) || n == 325u || n == 333u || n == 341u)
        return 1;
    if ((n >= 9u && n <= 257u && (n - 9u) % 8u == 0u) || n == 329u || n == 337u)
        return 0;
    return -1;
}

static void test_a12_with_background_at_1000(void)
{
    struct nesturbator *nes = start(0u, 0x10u, 0x18u, 5u, 0u);
    watch_a12(nes);
    step_to(nes, 5u, 1u);
    mapper_test_edge_count = 0u;
    for (unsigned n = 2u; n <= 341u; n++) {
        unsigned before = mapper_test_edge_count;
        step(nes);
        int expect = expected_edge_bg_at_1000(n);
        if (expect < 0) {
            CHECK_EQ_U64(mapper_test_edge_count, before);
        } else {
            check_new_edge(nes, before, (uint8_t)expect);
        }
    }
    CHECK_EQ_U64(nes->ppu.scanline, 6u);
    CHECK_EQ_U64(nes->ppu.dot, 0u);
    ppu_fixture_free(nes);
}

/* Rendering off: the bus shows v on every dot, dot 0 included. [D-09] */
static void test_rendering_off_bus_shows_v(void)
{
    struct nesturbator *nes = start(0u, 0u, 0x00u, 0u, 0u);
    nes->ppu.v = 0x5aaau;
    for (unsigned n = 0u; n < 700u; n++) {
        step(nes);
        CHECK_EQ_HEX(nes->ppu.bus_addr, 0x1aaau); /* 0x5AAA & 0x3FFF */
    }
    ppu_fixture_free(nes);

    nes = start(0u, 0u, 0x00u, 0u, 0u);
    watch_a12(nes);
    nesturbator__ppu_register_write(nes, 0x2006u, 0x10u);
    CHECK_EQ_U64(mapper_test_edge_count, 0u); /* the first write does not change v */
    nesturbator__ppu_register_write(nes, 0x2006u, 0x00u);
    check_new_edge(nes, 0u, 1u);
    nesturbator__ppu_register_write(nes, 0x2006u, 0x00u);
    nesturbator__ppu_register_write(nes, 0x2006u, 0x00u);
    check_new_edge(nes, 1u, 0u);
    (void)nesturbator__ppu_register_read(nes, 0x2007u); /* PPUCTRL bit 2 clear: v 0 -> 1 */
    CHECK_EQ_HEX(nes->ppu.bus_addr, 0x0001u);
    ppu_fixture_free(nes);
}

/* D-09: lines 240-260 show v even with rendering on. A rise caused by a v change at dot 240 is
   reported on that dot and not deferred. */
static void test_post_render_line_shows_v(void)
{
    struct nesturbator *nes = start(0u, 0u, 0x18u, 240u, 230u);
    watch_a12(nes);
    step_to(nes, 240u, 239u);
    nes->ppu.v = 0x1234u;
    step(nes); /* dot 240 */
    CHECK_EQ_HEX(nes->ppu.bus_addr, 0x1234u);
    check_new_edge(nes, 0u, 1u);
    for (uint16_t dot = 241u; dot <= 258u; dot++) {
        step(nes);
        CHECK_EQ_HEX(nes->ppu.bus_addr, 0x1234u);
        CHECK_EQ_U64(mapper_test_edge_count, 1u);
    }
    ppu_fixture_free(nes);
}

/* D-09: the step from line 239 into line 240 shows v at dot 0, not a pattern address. */
static void test_wrap_into_post_render_shows_v(void)
{
    struct nesturbator *nes = start(0u, 0u, 0x18u, 239u, 330u);
    nes->ppu.v = 0x1234u;
    step_to(nes, 240u, 0u);
    CHECK_EQ_HEX(nes->ppu.bus_addr, nes->ppu.v & 0x3fffu);
    ppu_fixture_free(nes);
}

/* Rendering switched off in mid-line: the first dot after the write shows v (D-09). */
static void test_rendering_switched_off_mid_line(void)
{
    struct nesturbator *nes = start(0u, 0x10u, 0x18u, 10u, 0u);
    watch_a12(nes);
    step_to(nes, 10u, 102u); /* a pattern-low read dot with A12 high */
    CHECK_EQ_U64(nes->ppu.bus_addr & 0x1000u, 0x1000u);
    nes->ppu.v = 0x0085u;
    nes->ppu.mask = 0u;
    unsigned before = mapper_test_edge_count;
    step(nes);
    CHECK_EQ_HEX(nes->ppu.bus_addr, 0x0085u);
    check_new_edge(nes, before, 0u);
    ppu_fixture_free(nes);
}

/* D-10: a read dot takes bits 8-13 from live state (PPUCTRL bit 4 and the fetched tile) and
   bits 0-7 from the ALE latch. With nothing changed A12 holds across the pair; a PPUCTRL write
   between them moves it on the read dot. */
static void test_read_dot_high_bits(void)
{
    struct nesturbator *nes = start(0u, 0x10u, 0x18u, 0u, 0u);
    watch_a12(nes);
    nes->ppu.v = 0x5000u; /* fine Y 5 */
    nesturbator__ppu_write(nes, 0x2000u, 0x35u);
    step_to(nes, 0u, 5u);
    CHECK_EQ_HEX(nes->ppu.bus_addr, 0x1355u); /* 0x1000 | 0x35 * 16 | 5 */
    unsigned before = mapper_test_edge_count;
    step(nes);
    CHECK_EQ_HEX(nes->ppu.bus_addr, 0x1355u);
    CHECK_EQ_U64(mapper_test_edge_count, before);
    ppu_fixture_free(nes);

    nes = start(0u, 0x10u, 0x18u, 0u, 0u);
    watch_a12(nes);
    nes->ppu.v = 0x5000u;
    nesturbator__ppu_write(nes, 0x2000u, 0x35u);
    step_to(nes, 0u, 5u);
    nesturbator__ppu_register_write(nes, 0x2000u, 0x00u);
    before = mapper_test_edge_count;
    step(nes);
    CHECK_EQ_HEX(nes->ppu.bus_addr, 0x0355u);
    check_new_edge(nes, before, 0u);
    ppu_fixture_free(nes);
}

/* A $2006 second write applies at once. A write after dot 255 meets the dot-256 coarse X and Y
   increments; a write after dot 257 is not undone by the horizontal copy. [D-11] */
static void test_2006_adjacency(void)
{
    struct nesturbator *nes = start(0u, 0u, 0x18u, 0u, 0u);
    step_to(nes, 0u, 255u);
    nesturbator__ppu_register_write(nes, 0x2006u, 0x05u);
    nesturbator__ppu_register_write(nes, 0x2006u, 0x43u);
    CHECK_EQ_HEX(nes->ppu.v, 0x0543u);
    step(nes); /* dot 256: coarse X 3 -> 4 and fine Y 0 -> 1 */
    CHECK_EQ_HEX(nes->ppu.v, 0x1544u);
    ppu_fixture_free(nes);

    nes = start(0u, 0u, 0x18u, 0u, 0u);
    step_to(nes, 0u, 258u);
    nesturbator__ppu_register_write(nes, 0x2006u, 0x05u);
    nesturbator__ppu_register_write(nes, 0x2006u, 0x43u);
    step_to(nes, 0u, 320u);
    CHECK_EQ_HEX(nes->ppu.v, 0x0543u);
    ppu_fixture_free(nes);
}

/* PPU scrolling "$2007 reads and writes": while rendering, an access increments coarse X and Y
   together and ignores PPUCTRL bit 2. */
static void test_2007_while_rendering(void)
{
    struct nesturbator *nes = start(0u, 0x04u, 0x18u, 10u, 0u);
    step_to(nes, 10u, 100u);
    nes->ppu.v = 0x2085u;
    (void)nesturbator__ppu_register_read(nes, 0x2007u);
    CHECK_EQ_HEX(nes->ppu.v, 0x3086u); /* coarse X 5 -> 6, fine Y 2 -> 3 */
    ppu_fixture_free(nes);
}

/* Dot 0 of a rendering line drives the low pattern address of the pending tile. On an odd
   frame the skip leaves the dot-339 nametable address on the bus and drops the dot-340 read, so
   with PPUCTRL bit 4 set no rise is logged at dot 0 and the first rise of line 0 is at dot 5,
   where an even frame logged one at dot 0. Recorded as a Phase 9 input. */
static void test_dot_zero_and_odd_frame(void)
{
    /* Even frame, from a visible line. */
    struct nesturbator *nes = start(0u, 0x10u, 0x18u, 0u, 320u);
    watch_a12(nes);
    nesturbator__ppu_write(nes, 0x2001u, 0x35u); /* the second tile's nametable byte */
    step_to(nes, 0u, 340u);
    unsigned before = mapper_test_edge_count;
    step(nes);
    CHECK_EQ_U64(nes->ppu.scanline, 1u);
    CHECK_EQ_U64(nes->ppu.dot, 0u);
    CHECK_EQ_U64(nes->ppu.bg_nt, 0x35u);
    CHECK_EQ_HEX(nes->ppu.bus_addr,
                 0x1000u | ((unsigned)nes->ppu.bg_nt << 4) | ((nes->ppu.v >> 12) & 7u));
    check_new_edge(nes, before, 1u);
    ppu_fixture_free(nes);

    /* Even frame, from the pre-render line. */
    nes = start(0u, 0x10u, 0x18u, 261u, 320u);
    watch_a12(nes);
    nesturbator__ppu_write(nes, 0x2001u, 0x35u);
    step_to(nes, 261u, 340u);
    before = mapper_test_edge_count;
    step(nes);
    CHECK_EQ_U64(nes->ppu.scanline, 0u);
    CHECK_EQ_U64(nes->ppu.dot, 0u);
    CHECK_EQ_U64(nes->ppu.bg_nt, 0x35u);
    CHECK_EQ_HEX(nes->ppu.bus_addr,
                 0x1000u | ((unsigned)nes->ppu.bg_nt << 4) | ((nes->ppu.v >> 12) & 7u));
    check_new_edge(nes, before, 1u);
    ppu_fixture_free(nes);

    /* Odd frame. */
    nes = start(0u, 0x10u, 0x18u, 261u, 335u);
    nes->ppu.odd_frame = 1u;
    watch_a12(nes);
    step_to(nes, 261u, 339u);
    CHECK_EQ_HEX(nes->ppu.bus_addr, 0x2000u | (nes->ppu.v & 0x0fffu));
    uint16_t nt_bus = nes->ppu.bus_addr;
    before = mapper_test_edge_count;
    step(nes);
    CHECK_EQ_U64(nes->ppu.scanline, 0u);
    CHECK_EQ_U64(nes->ppu.dot, 0u);
    CHECK_EQ_HEX(nes->ppu.bus_addr, nt_bus);
    CHECK_EQ_U64(mapper_test_edge_count, before);
    for (unsigned dot = 1u; dot <= 4u; dot++) {
        step(nes);
        CHECK_EQ_U64(mapper_test_edge_count, before);
    }
    step(nes); /* dot 5: the low pattern address of the first tile */
    check_new_edge(nes, before, 1u);
    ppu_fixture_free(nes);
}

int main(void)
{
    test_coarse_x_increments();
    test_y_increment_at_256();
    test_horizontal_copy_at_257();
    test_vertical_copy_on_prerender_only();
    test_fetch_addresses();
    test_shifter_polarity_and_hold();
    test_a12_with_background_at_1000();
    test_rendering_off_bus_shows_v();
    test_post_render_line_shows_v();
    test_wrap_into_post_render_shows_v();
    test_rendering_switched_off_mid_line();
    test_read_dot_high_bits();
    test_2006_adjacency();
    test_2007_while_rendering();
    test_dot_zero_and_odd_frame();
    CHECK_DONE();
}

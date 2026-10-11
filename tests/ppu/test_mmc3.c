/* BOARD-03, D-04, D-09: the MMC3 scanline counter clocked by the real PPU's A12 edges. Every case
   loads a synthetic mapper 4 image and sets PPUCTRL and PPUMASK directly. Frame cases run the
   real CPU and PPU through nesturbator_run_frame; dot cases single-step the PPU and advance
   nes->cpu_cycle by one every third dot, as the bus does. Expected behaviour comes from NESdev
   Wiki "MMC3" ("IRQ Specifics"): the counter clocks on a rise of A12 that follows at least three
   falling edges of M2 of low, so with BG at $0000 and sprites at $1000 it clocks once per
   rendered line (241 per frame, pre-render included), empty sprite slots included.
   The 2C02 odd-frame extra clock (research pitfall 10) is not modelled and not tested. */
#include <string.h>

#include "internal.h"
#include "../check.h"
#include "../mapper_test.h"
#include "ppu_fixture.h"

#define LINES_CLOCKED 241u /* 240 visible lines plus the pre-render line */

static void run_frame(struct nesturbator *nes)
{
    static uint16_t video[256u * 240u];
    static int16_t audio[2048];
    nesturbator_frame io;
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = video;
    io.video_pitch = 256u;
    io.audio = audio;
    io.audio_capacity = 2048u;
    CHECK_EQ_U64(nesturbator_run_frame((nesturbator *)nes, &io), NESTURBATOR_OK);
}

/* Latch $FF and a reload request, through the board's own register decode. After n >= 1 clocks
   the counter reads 256 - n, because the first clock reloads $FF and each later one subtracts 1. */
static void arm_counter(struct nesturbator *nes)
{
    nes->map.ops.cpu_write(nes, 0xc000u, 0xffu, nes->cpu_cycle);
    nes->map.ops.cpu_write(nes, 0xc001u, 0x00u, nes->cpu_cycle);
}

static unsigned clocks(const struct nesturbator *nes)
{
    if (nes->mapper.reg.mmc3.reload != 0u)
        return 0u;
    return 256u - nes->mapper.reg.mmc3.counter;
}

/* An MMC3 fixture with OAM clear of sprites in range (Y $FF) and the given registers. */
static struct nesturbator *start(uint8_t control, uint8_t mask)
{
    struct nesturbator *nes = ppu_fixture_load_mmc3(0u);
    memset(nes->ppu.oam, 0xff, sizeof nes->ppu.oam);
    nes->ppu.control = control;
    nes->ppu.mask = mask;
    return nes;
}

/* Clocks seen in one whole frame after a settling frame. Any 262-line window holds each line
   once, so the count does not depend on where the frame call begins. */
static unsigned frame_clocks(struct nesturbator *nes)
{
    run_frame(nes);
    arm_counter(nes);
    run_frame(nes);
    return clocks(nes);
}

/* Tracer: BG $0000, sprites $1000 (PPUCTRL bit 3), both layers on. */
static void test_bg0000_sprites1000_241_clocks(void)
{
    struct nesturbator *nes = start(0x08u, 0x18u);
    CHECK_EQ_U64(frame_clocks(nes), LINES_CLOCKED);
    ppu_fixture_free(nes);
}

/* Sprites in range fetch their real tile instead of the $FF filler; the high table still clocks
   once per line. */
static void test_sprites_in_range_still_one_per_line(void)
{
    struct nesturbator *nes = start(0x08u, 0x18u);
    for (unsigned i = 0u; i < 64u; i++) {
        nes->ppu.oam[i * 4u + 0u] = (uint8_t)(i * 3u); /* Y */
        nes->ppu.oam[i * 4u + 1u] = (uint8_t)i;
        nes->ppu.oam[i * 4u + 3u] = (uint8_t)(i * 4u);
    }
    CHECK_EQ_U64(frame_clocks(nes), LINES_CLOCKED);
    ppu_fixture_free(nes);
}

/* 8x16 sprites (PPUCTRL bit 5) select the table from the tile's bit 0: the empty-slot tile $FF
   reads $1FE0 and up, so BG at $0000 still clocks once per line. */
static void test_empty_slots_8x16(void)
{
    struct nesturbator *nes = start(0x20u, 0x18u);
    CHECK_EQ_U64(frame_clocks(nes), LINES_CLOCKED);
    ppu_fixture_free(nes);
}

/* BG and sprites both at $0000 never raise A12 while rendering: no clocks. */
static void test_both_tables_0000_no_clocks(void)
{
    struct nesturbator *nes = start(0x00u, 0x18u);
    CHECK_EQ_U64(frame_clocks(nes), 0u);
    ppu_fixture_free(nes);
}

/* BG at $1000 and sprites at $0000 (PPUCTRL bit 4): once per visible line at dot 325. The
   pre-render line clocks twice, since A12 has been low through lines 240-260 (the bus shows v)
   and rises at dot 5 as well, then again at 325, so a frame holds 242. */
static void test_bg1000_sprites0000_one_per_line(void)
{
    struct nesturbator *nes = start(0x10u, 0x18u);
    CHECK_EQ_U64(frame_clocks(nes), LINES_CLOCKED + 1u);
    ppu_fixture_free(nes);
}

static void step(struct nesturbator *nes, unsigned *dots)
{
    nesturbator__ppu_run_until(nes, nes->ppu.ppu_ticks + 8u);
    if (++*dots % 3u == 0u)
        nes->cpu_cycle++;
}

static void step_to(struct nesturbator *nes, unsigned *dots, uint16_t line, uint16_t dot)
{
    for (unsigned guard = 0u; guard < 200000u; guard++) {
        if (nes->ppu.scanline == line && nes->ppu.dot == dot)
            return;
        step(nes, dots);
    }
    CHECK(0);
}

/* With BG at $1000 the sprite fetches (table $0000) hold A12 low from dot 257 to the first tile
   of the next line's prefetch at dot 325 (test_fetch.c: the rise at 325, 68 dots or 22 CPU
   cycles later). That rise is the line's one clock; the rises at 5, 13, ... follow 4-dot lows. */
static void test_bg1000_clock_at_dot_325(void)
{
    struct nesturbator *nes = start(0x10u, 0x18u);
    unsigned dots = 0u;
    nes->ppu.scanline = 5u;
    nes->ppu.dot = 0u;
    step_to(nes, &dots, 5u, 323u);
    arm_counter(nes);
    step(nes, &dots); /* dot 324 still low */
    CHECK_EQ_U64(clocks(nes), 0u);
    step(nes, &dots); /* dot 325: the rise */
    CHECK_EQ_U64(nes->ppu.dot, 325u);
    CHECK_EQ_U64(clocks(nes), 1u);
    step_to(nes, &dots, 6u, 323u); /* one more line, one more clock, none before dot 325 */
    CHECK_EQ_U64(clocks(nes), 1u);
    step(nes, &dots);
    step(nes, &dots);
    CHECK_EQ_U64(clocks(nes), 2u);
    ppu_fixture_free(nes);
}

/* With sprites at $1000 and BG at $0000 the clock is the first sprite fetch after the 257-dot
   horizontal copy: the rise on the empty-slot pattern fetch, 4-dot gaps after it filtered. */
static void test_sprites1000_one_clock_not_eight(void)
{
    struct nesturbator *nes = start(0x08u, 0x18u);
    unsigned dots = 0u;
    nes->ppu.scanline = 10u;
    nes->ppu.dot = 0u;
    step_to(nes, &dots, 10u, 250u);
    arm_counter(nes);
    step_to(nes, &dots, 11u, 250u);
    CHECK_EQ_U64(clocks(nes), 1u); /* eight sprite fetches, one clock */
    ppu_fixture_free(nes);
}

/* Negative control (PITFALLS pitfall 7): with sprites at $1000 the PPU raises A12 once per sprite
   fetch, eight rises on a line with the 4-dot lows between them, yet the board clocks once. */
static void test_unfiltered_rises_are_many(void)
{
    struct nesturbator *nes = start(0x08u, 0x18u);
    unsigned dots = 0u;
    nes->ppu.scanline = 10u;
    nes->ppu.dot = 0u;
    step_to(nes, &dots, 10u, 250u);
    mapper_test_install(nes, NESTURBATOR_WATCH_PPU_A12);
    step_to(nes, &dots, 11u, 250u);
    unsigned rises = 0u;
    for (unsigned i = 0u; i < mapper_test_edge_count; i++)
        rises += mapper_test_edges[i].level;
    CHECK(rises >= 8u);
    ppu_fixture_free(nes);
}

/* Rendering off: the bus shows v, so a $2006 pair is a PPU address change. After A12 has been
   low for three CPU cycles a rise to $1000 clocks once; after two it does not. */
static void test_2006_rise_after_low_wait(void)
{
    struct nesturbator *nes = start(0x00u, 0x00u);
    nes->cpu_cycle = 100u;
    nesturbator__ppu_register_write(nes, 0x2006u, 0x10u);
    nesturbator__ppu_register_write(nes, 0x2006u, 0x00u); /* rise, ignored */
    nes->cpu_cycle = 110u;
    nesturbator__ppu_register_write(nes, 0x2006u, 0x00u);
    nesturbator__ppu_register_write(nes, 0x2006u, 0x00u); /* A12 falls at cycle 110 */
    arm_counter(nes);
    nes->cpu_cycle = 112u;
    nesturbator__ppu_register_write(nes, 0x2006u, 0x10u);
    nesturbator__ppu_register_write(nes, 0x2006u, 0x00u); /* two cycles low: filtered */
    CHECK_EQ_U64(clocks(nes), 0u);
    nes->cpu_cycle = 120u;
    nesturbator__ppu_register_write(nes, 0x2006u, 0x00u);
    nesturbator__ppu_register_write(nes, 0x2006u, 0x00u); /* falls at 120 */
    nes->cpu_cycle = 123u;
    nesturbator__ppu_register_write(nes, 0x2006u, 0x10u);
    nesturbator__ppu_register_write(nes, 0x2006u, 0x00u); /* three cycles low: clocks */
    CHECK_EQ_U64(clocks(nes), 1u);
    ppu_fixture_free(nes);
}

/* $2007 steps v across $0FFF to $1000 (PPUCTRL bit 2 clear: +1) after the same low wait. */
static void test_2007_step_across_0fff(void)
{
    struct nesturbator *nes = start(0x00u, 0x00u);
    nes->cpu_cycle = 50u;
    nesturbator__ppu_register_write(nes, 0x2006u, 0x10u);
    nesturbator__ppu_register_write(nes, 0x2006u, 0x00u);
    nes->cpu_cycle = 60u;
    nesturbator__ppu_register_write(nes, 0x2006u, 0x0fu);
    nesturbator__ppu_register_write(nes, 0x2006u, 0xffu); /* v $0FFF, A12 fell at 60 */
    arm_counter(nes);
    nes->cpu_cycle = 64u;
    (void)nesturbator__ppu_register_read(nes, 0x2007u);
    CHECK_EQ_HEX(nes->ppu.v, 0x1000u);
    CHECK_EQ_U64(clocks(nes), 1u);
    ppu_fixture_free(nes);
}

/* D-04: a board without the A12 watch bit is never called, whatever the hook is. */
static void test_non_mmc3_board_gets_no_a12_call(void)
{
    struct nesturbator *nes = ppu_fixture_load(0u, 0u);
    memset(nes->ppu.oam, 0xff, sizeof nes->ppu.oam);
    nes->ppu.control = 0x08u;
    nes->ppu.mask = 0x18u;
    CHECK_EQ_U64(nes->map.watch & NESTURBATOR_WATCH_PPU_A12, 0u);
    mapper_test_edge_count = 0u;
    mapper_test_fill(&nes->map.ops); /* hook set; the watch mask stays NROM's */
    run_frame(nes);
    CHECK_EQ_U64(mapper_test_edge_count, 0u);
    ppu_fixture_free(nes);
}

int main(void)
{
    test_bg0000_sprites1000_241_clocks();
    test_sprites_in_range_still_one_per_line();
    test_empty_slots_8x16();
    test_both_tables_0000_no_clocks();
    test_bg1000_sprites0000_one_per_line();
    test_bg1000_clock_at_dot_325();
    test_sprites1000_one_clock_not_eight();
    test_unfiltered_rises_are_many();
    test_2006_rise_after_low_wait();
    test_2007_step_across_0fff();
    test_non_mmc3_board_gets_no_a12_call();
    CHECK_DONE();
}

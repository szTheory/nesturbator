/* MAP-02, D-14, D-15: mid-frame $2005/$2006 splits, scanline by scanline, against origins derived
   by hand from the NESdev Wiki "PPU scrolling" text. Self-checking, no golden hash.

   The fixture is an NROM image with vertical mirroring and CHR RAM; every byte is generated here
   from a fixed-seed integer LCG and written through $2006/$2007 with rendering off. A plain
   sampler computes each expected row from those bytes without the PPU. The plane is 512 pixels
   wide (two nametables side by side) and Y runs 0-479, but vertical mirroring makes nametables 2
   and 3 copies of 0 and 1, so content repeats every 240 rows and the uniqueness check works
   modulo 240.

   Register writes land after dot N of a line has been processed (D-14). Every write stays at
   least 2 dots from 256, 257, 280 and 304 (D-15); those edges belong to ppu.fetch.

   Register rules used below (PPU scrolling, "Register controller"):
   - $2005 first write: t bits 0-4 = d bits 3-7 (coarse X), fine X = d bits 0-2, w = 1.
   - $2005 second write: t bits 5-9 = d bits 3-7 (coarse Y), t bits 12-14 = d bits 0-2, w = 0.
   - $2006 first write: t bits 8-13 = d bits 0-5, t bit 14 cleared, w = 1.
   - $2006 second write: t bits 0-7 = d, then v = t at once, w = 0.
   - Dot 257 of each line copies t's horizontal bits to v; the pre-render dots 280-304 copy its
     vertical bits; v increments Y at dot 256. */
#include <stdio.h>
#include <string.h>

#include "internal.h"
#include "../check.h"
#include "ppu_fixture.h"

#define PLANE_W 512u
#define PLANE_H 480u
#define CONTENT_H 240u

static uint8_t chr_data[4096];
static uint8_t nt_tiles[2][960];
static uint8_t nt_attr[2][64];
static uint8_t bg_palette[16];
static uint16_t frame[NESTURBATOR_WIDTH * NESTURBATOR_HEIGHT];

static uint32_t lcg_state;

static uint8_t lcg_next(void)
{
    lcg_state = lcg_state * 1103515245u + 12345u;
    return (uint8_t)(lcg_state >> 16);
}

/* Fixed-seed content: CHR, nametable tiles ((cx*37 + cy*11 + nt*101) & 0xff), attributes from the
   LCG, and 12 distinct background colours (subpalette s colour c is 1 + 3s + c - 1). */
static void build_content(void)
{
    lcg_state = 0x20240501u;
    for (unsigned i = 0u; i < sizeof chr_data; i++)
        chr_data[i] = lcg_next();
    for (unsigned nt = 0u; nt < 2u; nt++) {
        for (unsigned cy = 0u; cy < 30u; cy++)
            for (unsigned cx = 0u; cx < 32u; cx++)
                nt_tiles[nt][cy * 32u + cx] = (uint8_t)(cx * 37u + cy * 11u + nt * 101u);
        for (unsigned i = 0u; i < 64u; i++)
            nt_attr[nt][i] = lcg_next();
    }
    bg_palette[0] = 0x0fu;
    for (unsigned s = 0u; s < 4u; s++)
        for (unsigned c = 1u; c < 4u; c++)
            bg_palette[s * 4u + c] = (uint8_t)(1u + s * 3u + (c - 1u));
}

/* The native pixel at plane position (px, py), from the bytes above only. */
static uint16_t sample(unsigned px, unsigned py)
{
    px %= PLANE_W;
    py %= PLANE_H;
    unsigned bank = px / 256u;
    unsigned cx = (px % 256u) / 8u;
    unsigned fx = px % 8u;
    unsigned row = py % CONTENT_H;
    unsigned cy = row / 8u;
    unsigned fy = row % 8u;
    unsigned tile = nt_tiles[bank][cy * 32u + cx];
    unsigned lo = chr_data[tile * 16u + fy];
    unsigned hi = chr_data[tile * 16u + 8u + fy];
    unsigned colour = ((lo >> (7u - fx)) & 1u) | (((hi >> (7u - fx)) & 1u) << 1);
    unsigned attr = nt_attr[bank][(cy / 4u) * 8u + cx / 4u];
    unsigned quadrant = ((cy & 2u) << 1) | (cx & 2u);
    unsigned sub = (attr >> quadrant) & 3u;
    return colour == 0u ? bg_palette[0] : bg_palette[sub * 4u + colour];
}

struct segment {
    unsigned first_line;
    unsigned x; /* plane X of the leftmost pixel, 0-511 */
    unsigned y; /* plane Y of the segment's first line, 0-479 */
};

struct event {
    uint16_t line;
    uint16_t dot;
    uint16_t reg;
    uint8_t value;
};

struct scroll_case {
    const char *name;
    uint8_t initial_x;
    uint8_t initial_y;
    const struct event *events;
    unsigned event_count;
    const struct segment *segments;
    unsigned segment_count;
};

/* Case A, pre-render Y. A second $2005 write inside the pre-render dots 280-304 is picked up
   by the vertical copies that follow it (PPU scrolling: "If rendering is enabled, at the end of
   vblank, shortly after the horizontal bits are copied from t to v at dot 257, the PPU will
   repeatedly copy the vertical bits from t to v from dots 280 to 304"); one after dot 304 is
   not. The first $2005 write (X = 0x4D: coarse 9, fine 5) lands at dot 200 and reaches v at the
   dot-257 copy; its fine X applies at once. Initial Y is 0x28 (40); the new Y 0x6B is 107. */
static const struct event case_a_events_inside[] = {
    {261u, 200u, 0x2005u, 0x4du},
    {261u, 290u, 0x2005u, 0x6bu},
};
static const struct segment case_a_inside[] = {{0u, 77u, 107u}};
static const struct event case_a_events_after[] = {
    {261u, 200u, 0x2005u, 0x4du},
    {261u, 310u, 0x2005u, 0x6bu},
};
static const struct segment case_a_after[] = {{0u, 77u, 40u}};

/* Case B, X-only split (SMB style). PPU scrolling: "the Y scroll [of v] is not affected by a
   mid-frame $2005 Y write; v's vertical bits change only by increments and by the $2006 route".
   The horizontal copy at dot 257 gives the next line the new coarse X; X = 0x58 (88) is a
   multiple of 8, so the fine X (0) does not change at the write and line 31 itself is not
   altered. The Y byte 0xB0 must be ignored. Written before dot 257 the new X starts at line 32;
   written after it, at line 33. */
static const struct event case_b_events_before[] = {
    {31u, 250u, 0x2005u, 0x58u},
    {31u, 250u, 0x2005u, 0xb0u},
};
static const struct segment case_b_before[] = {{0u, 0u, 0u}, {32u, 88u, 32u}};
static const struct event case_b_events_after[] = {
    {31u, 262u, 0x2005u, 0x58u},
    {31u, 262u, 0x2005u, 0xb0u},
};
static const struct segment case_b_after[] = {{0u, 0u, 0u}, {33u, 88u, 33u}};

/* Case C, fine X at once. PPU scrolling: fine X is a separate register read live for each
   pixel, while coarse X reaches v only at the next dot-257 copy. Initial X 0x28 is coarse 5.
   The write after dot 260 of line 100 (X = 0x35: coarse 6, fine 5) is too late for line 100's
   pixels and for line 100's copy at 257, so line 101 uses old coarse 5 with the new fine 5
   (45), and line 102 uses coarse 6 (53). */
static const struct event case_c_events[] = {{100u, 260u, 0x2005u, 0x35u}};
static const struct segment case_c[] = {{0u, 40u, 0u}, {101u, 45u, 101u}, {102u, 53u, 102u}};

/* Case D, $2006/$2005/$2005/$2006 (PPU scrolling "Split X/Y scroll"). Writes 0x04, 0x43, 0xA5,
   0x14 give t = 0x0400 after the first, 0x3500 after the Y write (coarse Y 8, fine Y 3, nametable
   bit 10 kept), 0x3514 after the X write (coarse X 20, fine X 5), and the last write sets
   v = t = 0x3514 at once: nametable 1, X = 256 + 20 * 8 + 5 = 421, Y = 8 * 8 + 3 = 67. Line 120's
   pixels are done, so line 121 is the first to show it, from the prefetch at dots 321-336. */
static const struct event case_d_events[] = {
    {120u, 262u, 0x2006u, 0x04u},
    {120u, 270u, 0x2005u, 0x43u},
    {120u, 278u, 0x2005u, 0xa5u},
    {120u, 290u, 0x2006u, 0x14u},
};
static const struct segment case_d[] = {{0u, 19u, 16u}, {121u, 421u, 67u}};

static const struct scroll_case cases[] = {
    {"A pre-render Y write inside 280-304", 0x00u, 0x28u, case_a_events_inside, 2u, case_a_inside,
     1u},
    {"A pre-render Y write after 304", 0x00u, 0x28u, case_a_events_after, 2u, case_a_after, 1u},
    {"B X-only split before dot 257", 0x00u, 0x00u, case_b_events_before, 2u, case_b_before, 2u},
    {"B X-only split after dot 257", 0x00u, 0x00u, case_b_events_after, 2u, case_b_after, 2u},
    {"C fine X at once", 0x28u, 0x00u, case_c_events, 1u, case_c, 3u},
    {"D $2006/$2005/$2005/$2006", 0x13u, 0x10u, case_d_events, 4u, case_d, 2u},
};

#define CASE_COUNT (sizeof cases / sizeof cases[0])

static void expected_origin(const struct scroll_case *c, unsigned line, unsigned *x, unsigned *y)
{
    const struct segment *seg = &c->segments[0];
    for (unsigned i = 0u; i < c->segment_count; i++)
        if (c->segments[i].first_line <= line)
            seg = &c->segments[i];
    *x = seg->x;
    *y = (seg->y + line - seg->first_line) % PLANE_H;
}

static void expected_row(unsigned x, unsigned y, uint16_t *row)
{
    for (unsigned i = 0u; i < NESTURBATOR_WIDTH; i++)
        row[i] = sample(x + i, y);
}

/* Counts origins in the 512 x 240 content plane, other than (x, y) modulo 240, whose 256-pixel
   row equals the one at (x, y). */
static unsigned count_other_origins(unsigned x, unsigned y)
{
    unsigned found = 0u;
    for (unsigned oy = 0u; oy < CONTENT_H; oy++) {
        for (unsigned ox = 0u; ox < PLANE_W; ox++) {
            if (ox == x % PLANE_W && oy == y % CONTENT_H)
                continue;
            unsigned i = 0u;
            while (i < NESTURBATOR_WIDTH && sample(ox + i, oy) == sample(x + i, y))
                i++;
            if (i == NESTURBATOR_WIDTH)
                found++;
        }
    }
    return found;
}

/* Failure path only: which origin does this row come from? */
static int find_origin(const uint16_t *row, unsigned *fx, unsigned *fy)
{
    for (unsigned oy = 0u; oy < CONTENT_H; oy++) {
        for (unsigned ox = 0u; ox < PLANE_W; ox++) {
            unsigned i = 0u;
            while (i < NESTURBATOR_WIDTH && sample(ox + i, oy) == row[i])
                i++;
            if (i == NESTURBATOR_WIDTH) {
                *fx = ox;
                *fy = oy;
                return 1;
            }
        }
    }
    return 0;
}

static void step_to(struct nesturbator *nes, uint16_t line, uint16_t dot)
{
    for (unsigned guard = 0u; guard < 200000u; guard++) {
        if (nes->ppu.scanline == line && nes->ppu.dot == dot)
            return;
        nesturbator__ppu_run_until(nes, nes->ppu.ppu_ticks + 8u);
    }
    CHECK(0);
}

static void write_stream(struct nesturbator *nes, uint16_t addr, const uint8_t *bytes, unsigned n)
{
    nesturbator__ppu_register_write(nes, 0x2006u, (uint8_t)(addr >> 8));
    nesturbator__ppu_register_write(nes, 0x2006u, (uint8_t)(addr & 0xffu));
    for (unsigned i = 0u; i < n; i++)
        nesturbator__ppu_register_write(nes, 0x2007u, bytes[i]);
}

static void run_case(const struct scroll_case *c)
{
    struct nesturbator *nes = ppu_fixture_load(1u, 1u);
    memset(nes->ppu.oam, 0xff, sizeof nes->ppu.oam);
    memset(frame, 0xff, sizeof frame);
    nes->ppu.video_output = frame;
    nes->ppu.video_pitch = NESTURBATOR_WIDTH;

    /* Content goes in with rendering off. */
    write_stream(nes, 0x0000u, chr_data, sizeof chr_data);
    for (unsigned nt = 0u; nt < 2u; nt++) {
        write_stream(nes, (uint16_t)(0x2000u + nt * 0x400u), nt_tiles[nt], 960u);
        write_stream(nes, (uint16_t)(0x23c0u + nt * 0x400u), nt_attr[nt], 64u);
    }
    write_stream(nes, 0x3f00u, bg_palette, sizeof bg_palette);

    /* The pre-render line, rendering on, initial scroll before dot 257. */
    nes->ppu.scanline = 261u;
    nes->ppu.dot = 0u;
    nesturbator__ppu_register_write(nes, 0x2000u, 0x00u);
    nesturbator__ppu_register_write(nes, 0x2005u, c->initial_x);
    nesturbator__ppu_register_write(nes, 0x2005u, c->initial_y);
    nesturbator__ppu_register_write(nes, 0x2001u, 0x0au); /* background and its left column */
    for (unsigned i = 0u; i < c->event_count; i++) {
        step_to(nes, c->events[i].line, c->events[i].dot);
        nesturbator__ppu_register_write(nes, c->events[i].reg, c->events[i].value);
    }
    step_to(nes, 239u, 256u);

    unsigned bad_lines = 0u;
    for (unsigned line = 0u; line < NESTURBATOR_HEIGHT; line++) {
        unsigned ex;
        unsigned ey;
        uint16_t want[NESTURBATOR_WIDTH];
        const uint16_t *got = &frame[line * NESTURBATOR_WIDTH];
        expected_origin(c, line, &ex, &ey);
        expected_row(ex, ey, want);
        if (memcmp(want, got, sizeof want) == 0)
            continue;
        if (bad_lines++ == 0u) {
            unsigned ax = 0u;
            unsigned ay = 0u;
            int known = find_origin(got, &ax, &ay);
            fprintf(stderr,
                    "%s: scanline %u expected X %u (coarse %u fine %u) Y %u (coarse %u fine %u), ",
                    c->name, line, ex, (ex % 256u) / 8u, ex % 8u, ey, (ey % 240u) / 8u, ey % 8u);
            if (known != 0)
                fprintf(stderr, "actual X %u (coarse %u fine %u) Y %u (coarse %u fine %u)\n", ax,
                        (ax % 256u) / 8u, ax % 8u, ay, (ay % 240u) / 8u, ay % 8u);
            else
                fprintf(stderr, "actual row matches no origin\n");
        }
    }
    if (bad_lines != 0u) {
        fprintf(stderr, "%s: %u scanlines differ\n", c->name, bad_lines);
        check_failures++;
    }
    ppu_fixture_free(nes);
}

/* Before rendering: for the first line of every segment, and the last line of the frame, no
   other origin gives the same 256-pixel row, so a match cannot come from a wrong origin. */
static void check_fixture_is_unambiguous(void)
{
    for (unsigned k = 0u; k < CASE_COUNT; k++) {
        const struct scroll_case *c = &cases[k];
        for (unsigned i = 0u; i <= c->segment_count; i++) {
            unsigned line = i < c->segment_count ? c->segments[i].first_line : 239u;
            unsigned x;
            unsigned y;
            expected_origin(c, line, &x, &y);
            unsigned others = count_other_origins(x, y);
            if (others != 0u)
                fprintf(stderr, "%s: line %u row (X %u, Y %u) also matches %u other origins\n",
                        c->name, line, x, y, others);
            CHECK_EQ_U64(others, 0u);
        }
    }
}

int main(void)
{
    build_content();
    check_fixture_is_unambiguous();
    for (unsigned k = 0u; k < CASE_COUNT; k++)
        run_case(&cases[k]);
    CHECK_DONE();
}

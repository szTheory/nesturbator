/* PPU memory, registers, dot clock and native background output. */
#include "internal.h"
#include <string.h>

/* Drives the PPU address bus, the 14 bits on the pins, and reports bit 12 to the board. Every
   PPU address goes through here, so this is the only writer of ppu.bus_addr. Both edges are
   reported with the tick of the dot that caused them and the core applies no filter: a board
   such as the MMC3 decides which edges count. [NESdev Wiki, MMC3 and PPU rendering] The dots
   in this file are 1-based; the MMC3 page's 260 and 324 are 0-based, so the same edges are at
   dots 261 and 325 here. */
static void set_bus(struct nesturbator *nes, uint16_t addr)
{
    addr &= 0x3fffu;
    nes->ppu.bus_addr = addr;
    uint8_t level = (uint8_t)((addr >> 12) & 1u);
    if (level != nes->map.a12) {
        nes->map.a12 = level;
        if ((nes->map.watch & NESTURBATOR_WATCH_PPU_A12) != 0u)
            nes->map.ops.ppu_a12(nes, level, nes->ppu.ppu_ticks);
    }
}

/* The fetch pipeline runs when background or sprites are on, on the visible lines and the
   pre-render line. Otherwise the bus shows v. [NESdev Wiki, PPU rendering and PPU scrolling] */
static int rendering_active(const struct nesturbator *nes)
{
    return (nes->ppu.mask & 0x18u) != 0u && (nes->ppu.scanline < 240u || nes->ppu.scanline == 261u);
}

/* Coarse X increment with nametable wrap. [NESdev Wiki, PPU scrolling "Wrapping around"] */
static void inc_coarse_x(struct nesturbator *nes)
{
    uint16_t v = nes->ppu.v;
    if ((v & 0x001fu) == 31u)
        v = (uint16_t)((v & ~0x001fu) ^ 0x0400u);
    else
        v = (uint16_t)(v + 1u);
    nes->ppu.v = v;
}

/* Y increment: fine Y, then coarse Y, which wraps at 29 with a vertical nametable switch and at
   31 without one. [NESdev Wiki, PPU scrolling "Wrapping around"] */
static void inc_y(struct nesturbator *nes)
{
    uint16_t v = nes->ppu.v;
    if ((v & 0x7000u) != 0x7000u) {
        v = (uint16_t)(v + 0x1000u);
    } else {
        v = (uint16_t)(v & ~0x7000u);
        uint16_t y = (uint16_t)((v & 0x03e0u) >> 5);
        if (y == 29u) {
            y = 0u;
            v ^= 0x0800u;
        } else if (y == 31u) {
            y = 0u;
        } else {
            y = (uint16_t)(y + 1u);
        }
        v = (uint16_t)((v & ~0x03e0u) | (uint16_t)(y << 5));
    }
    nes->ppu.v = v;
}

/* Dot 257: v takes the horizontal bits of t. [NESdev Wiki, PPU scrolling] */
static void copy_horizontal(struct nesturbator *nes)
{
    nes->ppu.v = (uint16_t)((nes->ppu.v & ~0x041fu) | (nes->ppu.t & 0x041fu));
}

/* Pre-render dots 280-304: v takes the vertical bits of t. [NESdev Wiki, PPU scrolling] */
static void copy_vertical(struct nesturbator *nes)
{
    nes->ppu.v = (uint16_t)((nes->ppu.v & ~0x7be0u) | (nes->ppu.t & 0x7be0u));
}

/* One helper per access kind computes the address from live state, so an address dot and its
   read dot use the same formula and cannot diverge. [NESdev Wiki, PPU scrolling "Tile and
   attribute fetching" and PPU rendering] */
static uint16_t nt_address(const struct nesturbator *nes)
{
    return (uint16_t)(0x2000u | (nes->ppu.v & 0x0fffu));
}

static uint16_t at_address(const struct nesturbator *nes)
{
    uint16_t v = nes->ppu.v;
    return (uint16_t)(0x23c0u | (v & 0x0c00u) | ((v >> 4) & 0x38u) | ((v >> 2) & 0x07u));
}

/* Bit 12 is PPUCTRL bit 4, bits 4-11 are the fetched tile and bits 0-2 are fine Y; the high
   plane is 8 higher. */
static uint16_t bg_pattern_address(const struct nesturbator *nes, unsigned plane)
{
    uint32_t addr = ((uint32_t)(nes->ppu.control & 0x10u) << 8) | ((uint32_t)nes->ppu.bg_nt << 4) |
                    (uint32_t)((nes->ppu.v >> 12) & 7u);
    return (uint16_t)(addr + (plane != 0u ? 8u : 0u));
}

/* An address dot drives the bus and ALE keeps the low 8 bits. */
static void address_dot(struct nesturbator *nes, uint16_t addr)
{
    set_bus(nes, addr);
    nes->ppu.ale_latch = (uint8_t)(addr & 0xffu);
}

/* A read dot rebuilds the high 6 bits from live state and takes the low 8 from ALE, then reads.
   If PPUCTRL or the fetched state changed since the address dot, A12 can change here.
   [NESdev Wiki, PPU rendering] */
static uint8_t read_dot(struct nesturbator *nes, uint16_t live)
{
    set_bus(nes, (uint16_t)((live & 0x3f00u) | nes->ppu.ale_latch));
    return nesturbator__ppu_read(nes, nes->ppu.bus_addr);
}

/* Pattern shifters shift in 0 for the low plane and 1 for the high plane; the attribute
   shifters shift in their latch. [NESdev Wiki, PPU rendering "constants 1 for the high
   bitplane and 0 for the low bitplane"] */
static void shift_background(struct nesturbator *nes)
{
    struct nesturbator__ppu *ppu = &nes->ppu;
    ppu->bg_shift_lo = (uint16_t)((uint32_t)ppu->bg_shift_lo << 1);
    ppu->bg_shift_hi = (uint16_t)(((uint32_t)ppu->bg_shift_hi << 1) | 1u);
    ppu->at_shift_lo = (uint8_t)((ppu->at_shift_lo << 1) | ppu->at_latch_lo);
    ppu->at_shift_hi = (uint8_t)((ppu->at_shift_hi << 1) | ppu->at_latch_hi);
}

static void reload_shifters(struct nesturbator *nes)
{
    struct nesturbator__ppu *ppu = &nes->ppu;
    ppu->bg_shift_lo = (uint16_t)((ppu->bg_shift_lo & 0xff00u) | ppu->bg_lo);
    ppu->bg_shift_hi = (uint16_t)((ppu->bg_shift_hi & 0xff00u) | ppu->bg_hi);
    ppu->at_latch_lo = (uint8_t)(ppu->bg_at & 1u);
    ppu->at_latch_hi = (uint8_t)((ppu->bg_at >> 1) & 1u);
}

/* The eight-dot tile fetch: NT, AT, pattern low, pattern high, two dots each,
   chosen by one switch on (dot - 1) & 7. The coarse X increment follows the high-plane read,
   and Y follows it at dot 256. [NESdev Wiki, PPU rendering and PPU scrolling] */
static void background_access(struct nesturbator *nes)
{
    struct nesturbator__ppu *ppu = &nes->ppu;
    switch ((ppu->dot - 1u) & 7u) {
    case 0u:
        address_dot(nes, nt_address(nes));
        break;
    case 1u:
        ppu->bg_nt = read_dot(nes, nt_address(nes));
        break;
    case 2u:
        address_dot(nes, at_address(nes));
        break;
    case 3u: {
        uint8_t byte = read_dot(nes, at_address(nes));
        ppu->bg_at = (uint8_t)((byte >> (((ppu->v >> 4) & 4u) | (ppu->v & 2u))) & 3u);
        break;
    }
    case 4u:
        address_dot(nes, bg_pattern_address(nes, 0u));
        break;
    case 5u:
        ppu->bg_lo = read_dot(nes, bg_pattern_address(nes, 0u));
        break;
    case 6u:
        address_dot(nes, bg_pattern_address(nes, 1u));
        break;
    default:
        ppu->bg_hi = read_dot(nes, bg_pattern_address(nes, 1u));
        inc_coarse_x(nes);
        if (ppu->dot == 256u)
            inc_y(nes);
        break;
    }
}

static void sprite_evaluate(struct nesturbator *nes)
{
    struct nesturbator__ppu *ppu = &nes->ppu;
    if (ppu->scanline > 239u && ppu->scanline != 261u)
        return;
    if (ppu->dot == 65u) {
        ppu->eval_count = 0u;
        ppu->eval_n = 0u;
        ppu->eval_m = 0u;
        ppu->eval_remaining = 0u;
        ppu->eval_copy_remaining = 0u;
        /* Pre-render evaluation prepares visible scanline zero; Y+1 wraps
           at eight bits on the 2C02. [HWP.02][HWP.06] */
        ppu->eval_target = ppu->scanline == 261u ? 0u : (uint8_t)(ppu->scanline + 1u);
        memset(ppu->secondary_oam, 0xff, sizeof ppu->secondary_oam);
    }
    if ((ppu->mask & 0x18u) == 0u)
        return;
    if (ppu->dot < 65u || ppu->dot > 256u)
        return;
    if (ppu->eval_n >= 64u)
        return;

    /* The 2C02 alternates primary-OAM reads and secondary-OAM writes. A
       selected Y is followed by three odd/even byte-copy pairs before n
       advances; after eight slots fill, a miss advances both n and m, so
       non-Y bytes can be mistaken for Y. [HWP.06] */
    if ((ppu->dot & 1u) != 0u) {
        ppu->eval_latch = ppu->oam[(uint16_t)ppu->eval_n * 4u + ppu->eval_m];
        return;
    }
    if (ppu->eval_copy_remaining != 0u) {
        uint16_t slot_base = (uint16_t)(ppu->eval_count - 1u) * 4u;
        ppu->secondary_oam[slot_base + ppu->eval_m] = ppu->eval_latch;
        ppu->eval_copy_remaining--;
        ppu->eval_m = (uint8_t)((ppu->eval_m + 1u) & 3u);
        if (ppu->eval_m == 0u)
            ppu->eval_n++;
        return;
    }
    if (ppu->eval_remaining != 0u) {
        ppu->eval_remaining--;
        ppu->eval_m = (uint8_t)((ppu->eval_m + 1u) & 3u);
        if (ppu->eval_m == 0u)
            ppu->eval_n++;
        return;
    }
    uint16_t top = (uint16_t)ppu->eval_latch + 1u;
    uint8_t height = (ppu->control & 0x20u) != 0u ? 16u : 8u;
    uint8_t wraps_to_first_row = ppu->eval_target == 0u && top == 256u;
    if (wraps_to_first_row != 0u ||
        ((uint16_t)ppu->eval_target >= top && (uint16_t)ppu->eval_target < top + height)) {
        if (ppu->eval_count < 8u) {
            uint8_t slot = ppu->eval_count++;
            ppu->secondary_oam[(uint16_t)slot * 4u] = ppu->eval_latch;
            ppu->eval_sprite_zero[slot] = ppu->eval_n == 0u;
            ppu->eval_m = 1u;
            ppu->eval_copy_remaining = 3u;
            return;
        } else {
            if (ppu->scanline != 261u)
                ppu->status |= 0x20u;
            ppu->eval_remaining = 3u;
            ppu->eval_m = (uint8_t)((ppu->eval_m + 1u) & 3u);
            if (ppu->eval_m == 0u)
                ppu->eval_n++;
            return;
        }
    }
    ppu->eval_n++;
    if (ppu->eval_count == 8u)
        ppu->eval_m = (uint8_t)((ppu->eval_m + 1u) & 3u);
}

static uint8_t reverse_bits(uint8_t value)
{
    value = (uint8_t)(((value & 0x55u) << 1) | ((value >> 1) & 0x55u));
    value = (uint8_t)(((value & 0x33u) << 2) | ((value >> 2) & 0x33u));
    return (uint8_t)((value << 4) | (value >> 4));
}

static void sprite_fetch(struct nesturbator *nes)
{
    struct nesturbator__ppu *ppu = &nes->ppu;
    if ((ppu->scanline > 239u && ppu->scanline != 261u) || ppu->dot != 257u)
        return;
    uint8_t height = (ppu->control & 0x20u) != 0u ? 16u : 8u;
    uint8_t target = ppu->scanline == 261u ? 0u : (uint8_t)(ppu->scanline + 1u);
    for (uint8_t i = 0u; i < ppu->eval_count; i++) {
        uint16_t base = (uint16_t)i * 4u;
        uint8_t y = ppu->secondary_oam[base];
        uint8_t tile = ppu->secondary_oam[base + 1u];
        uint8_t attr = ppu->secondary_oam[base + 2u];
        uint16_t top = (uint16_t)y + 1u;
        uint8_t row = target == 0u && top == 256u ? 0u : (uint8_t)((uint16_t)target - top);
        if ((attr & 0x80u) != 0u)
            row = (uint8_t)(height - 1u - row);
        uint16_t pattern;
        if (height == 16u) {
            pattern = (uint16_t)((tile & 1u) * 0x1000u + (tile & 0xfeu) * 16u);
            if (row >= 8u) {
                pattern += 16u;
                row = (uint8_t)(row - 8u);
            }
        } else {
            pattern = (ppu->control & 0x08u) != 0u ? 0x1000u : 0u;
            pattern = (uint16_t)(pattern + (uint16_t)tile * 16u);
        }
        ppu->sprite_lo[i] = nesturbator__ppu_read(nes, (uint16_t)(pattern + row));
        ppu->sprite_hi[i] = nesturbator__ppu_read(nes, (uint16_t)(pattern + row + 8u));
        if ((attr & 0x40u) != 0u) {
            ppu->sprite_lo[i] = reverse_bits(ppu->sprite_lo[i]);
            ppu->sprite_hi[i] = reverse_bits(ppu->sprite_hi[i]);
        }
        ppu->sprite_x[i] = ppu->secondary_oam[base + 3u];
        ppu->sprite_attr[i] = attr;
        ppu->sprite_zero[i] = ppu->eval_sprite_zero[i];
    }
    ppu->sprite_count = ppu->eval_count;
}

/* One dot of the fetch pipeline, run at the end of the dot (the existing 8-tick boundary). With
   rendering off, or on lines 240-260, the bus shows v on every dot. Otherwise: shift, reload,
   then the dot's access. Background tiles use dots 1-256 and 321-336, sprite slots 257-320 and
   two dummy nametable accesses 337-340. [NESdev Wiki, PPU rendering] */
static void fetch_step(struct nesturbator *nes)
{
    struct nesturbator__ppu *ppu = &nes->ppu;
    uint16_t dot = ppu->dot;
    if (dot > 340u)
        return;
    if (!rendering_active(nes)) {
        if (dot == 257u)
            ppu->sprite_count = ppu->eval_count;
        set_bus(nes, ppu->v);
        return;
    }
    if ((dot >= 2u && dot <= 257u) || (dot >= 322u && dot <= 337u))
        shift_background(nes);
    if ((dot & 7u) == 1u && ((dot >= 9u && dot <= 257u) || dot == 329u || dot == 337u))
        reload_shifters(nes);
    if (dot <= 256u || (dot >= 321u && dot <= 336u)) {
        background_access(nes);
    } else if (dot <= 320u) {
        /* The garbage nametable address at dot 257 is taken from v before the horizontal copy. */
        if (dot == 257u) {
            address_dot(nes, nt_address(nes));
            copy_horizontal(nes);
            ppu->sprite_count = ppu->eval_count;
        }
        /* Pre-render only: the vertical copy repeats on 280-304. */
        if (ppu->scanline == 261u && dot >= 280u && dot <= 304u)
            copy_vertical(nes);
    } else if ((dot & 1u) != 0u) {
        address_dot(nes, nt_address(nes));
    } else {
        (void)read_dot(nes, nt_address(nes));
    }
}

/* The background colour comes from the shifters: pattern bit 15 - fine X and attribute bit
   7 - fine X. Colour zero aliases the universal backdrop for every subpalette. [HWP.07] */
static uint16_t background_color(const struct nesturbator *nes, uint32_t x, uint8_t *opaque)
{
    const struct nesturbator__ppu *ppu = &nes->ppu;
    uint8_t color = 0u;
    uint8_t subpalette = 0u;
    if ((ppu->mask & 0x08u) != 0u && (x >= 8u || (ppu->mask & 0x02u) != 0u)) {
        unsigned pattern_bit = 15u - ppu->fine_x;
        unsigned attribute_bit = 7u - ppu->fine_x;
        color = (uint8_t)(((ppu->bg_shift_lo >> pattern_bit) & 1u) |
                          (((ppu->bg_shift_hi >> pattern_bit) & 1u) << 1));
        subpalette = (uint8_t)(((ppu->at_shift_lo >> attribute_bit) & 1u) |
                               (((ppu->at_shift_hi >> attribute_bit) & 1u) << 1));
    }

    *opaque = color != 0u;
    uint8_t palette_index = color == 0u ? 0u : (uint8_t)(subpalette * 4u + color);
    uint8_t palette_value = (uint8_t)(ppu->palette[palette_index] & 0x3fu);
    if ((ppu->mask & 0x01u) != 0u)
        palette_value &= 0x30u;
    return (uint16_t)(palette_value | (uint16_t)((ppu->mask >> 5) & 7u) << 6);
}

static uint16_t compose_pixel(struct nesturbator *nes, uint32_t x)
{
    uint8_t bg_opaque = 0u;
    uint16_t bg = background_color(nes, x, &bg_opaque);
    struct nesturbator__ppu *ppu = &nes->ppu;
    if ((ppu->mask & 0x10u) == 0u || (x < 8u && (ppu->mask & 0x04u) == 0u))
        return bg;
    for (uint8_t i = 0u; i < ppu->sprite_count; i++) {
        uint8_t sx = ppu->sprite_x[i];
        if (x < sx || x >= (uint32_t)sx + 8u)
            continue;
        uint8_t bit = (uint8_t)(7u - (x - sx));
        uint8_t color =
            (uint8_t)(((ppu->sprite_lo[i] >> bit) & 1u) | (((ppu->sprite_hi[i] >> bit) & 1u) << 1));
        if (color == 0u)
            continue;
        if (ppu->sprite_zero[i] != 0u && bg_opaque != 0u && x != 255u &&
            (ppu->mask & 0x08u) != 0u && (x >= 8u || (ppu->mask & 0x06u) == 0x06u))
            ppu->status |= 0x40u;
        if (bg_opaque != 0u && (ppu->sprite_attr[i] & 0x20u) != 0u)
            return bg;
        uint8_t palette = (uint8_t)(0x10u + (ppu->sprite_attr[i] & 3u) * 4u + color);
        uint8_t value = (uint8_t)(ppu->palette[palette] & 0x3fu);
        if ((ppu->mask & 1u) != 0u)
            value &= 0x30u;
        return (uint16_t)(value | (uint16_t)((ppu->mask >> 5) & 7u) << 6);
    }
    return bg;
}

uint8_t nesturbator__ppu_read(struct nesturbator *nes, uint16_t addr)
{
    addr &= 0x3fffu;
    if (addr < 0x2000u) {
        const uint8_t *page = nes->map.chr_r[addr >> 10];
        return page != NULL ? page[addr & 0x3ffu] : 0u;
    }
    if (addr < 0x3f00u) {
        uint16_t offset = (uint16_t)((addr - 0x2000u) & 0x0fffu);
        uint16_t table = (uint16_t)(offset >> 10);
        uint16_t within = (uint16_t)(offset & 0x03ffu);
        /* Mirroring is the board's nt[] map: one 1 KiB CIRAM bank per nametable.
           For NROM it comes from iNES flags 6 bit 0. Four-screen boards are
           rejected at load. [HWP.14] */
        uint16_t ciram = (uint16_t)(nes->map.nt[table] * 0x400u + within);
        return nes->ppu.nametable[ciram];
    }
    addr = (uint16_t)((addr - 0x3f00u) & 0x1fu);
    if ((addr & 0x13u) == 0x10u)
        addr &= 0x0fu;
    return (uint8_t)(nes->ppu.palette[addr] & 0x3fu);
}

void nesturbator__ppu_write(struct nesturbator *nes, uint16_t addr, uint8_t value)
{
    addr &= 0x3fffu;
    if (addr < 0x2000u) {
        uint8_t *page = nes->map.chr_w[addr >> 10];
        if (page != NULL)
            page[addr & 0x3ffu] = value;
    } else if (addr < 0x3f00u) {
        uint16_t offset = (uint16_t)((addr - 0x2000u) & 0x0fffu);
        uint16_t table = (uint16_t)(offset >> 10);
        uint16_t within = (uint16_t)(offset & 0x03ffu);
        uint16_t ciram = (uint16_t)(nes->map.nt[table] * 0x400u + within);
        nes->ppu.nametable[ciram] = value;
    } else {
        addr = (uint16_t)((addr - 0x3f00u) & 0x1fu);
        if ((addr & 0x13u) == 0x10u)
            addr &= 0x0fu;
        nes->ppu.palette[addr] = (uint8_t)(value & 0x3fu);
    }
}

void nesturbator__ppu_run_until(struct nesturbator *nes, uint64_t ticks)
{
    while (nes->ppu.ppu_ticks + 8u <= ticks) {
        nes->ppu.ppu_ticks += 8u;
        /* Model the analog PPU I/O bus decay at the conservative 30 ms end
           of the measured 3-30 ms range. This avoids platform time sources. [HWP.04] */
        if (nes->ppu.io_bus_age < 160000u) {
            nes->ppu.io_bus_age++;
            if (nes->ppu.io_bus_age == 160000u)
                nes->ppu.io_bus = 0u;
        }
        nes->ppu.dot++;
        /* NTSC vblank starts at scanline 241 dot 1 and ends at 261 dot 1.
           Enabling NMI with vblank active raises the CPU's pending edge. [HWP.03] */
        if (nes->ppu.scanline == 241u && nes->ppu.dot == 1u) {
            if (nes->ppu.vblank_suppress == 0u) {
                nes->ppu.status |= 0x80u;
                if ((nes->ppu.control & 0x80u) != 0u)
                    nes->cpu.nmi_pending = 1u;
            }
            nes->ppu.vblank_suppress = 0u;
        }
        if (nes->ppu.scanline == 261u && nes->ppu.dot == 1u) {
            nes->ppu.status &= 0x1fu;
            nes->cpu.nmi_pending = 0u;
            /* The reset signal "is set on reset and cleared at the end of VBlank, by the
               same signal that clears the VBlank, sprite 0, and overflow flags". [NESdev
               Wiki, PPU power up state] */
            nes->ppu.reset_flag = 0u;
        }
        sprite_evaluate(nes);
        sprite_fetch(nes);
        /* Rendering skips pre-render dot 340 on odd NTSC frames. [HWP.03] The skip lands on
           scanline 0 dot 0 without the dot-340 read, so the bus keeps the dot-339 nametable
           address. NESdev "PPU rendering" describes the last dummy fetch replacing the idle
           tick instead; the read is hash-neutral and A12-neutral (a nametable address has
           A12 low), but with PPUCTRL bit 4 set an even frame rises at dot 0 and an odd one does
           not. */
        if (nes->ppu.scanline == 261u && nes->ppu.dot == 340u && nes->ppu.odd_frame != 0u &&
            (nes->ppu.mask & 0x18u) != 0u) {
            nes->ppu.scanline = 0u;
            nes->ppu.dot = 0u;
            nes->ppu.odd_frame = 0u;
            continue;
        }
        fetch_step(nes);
        if (nes->ppu.dot == 341u) {
            nes->ppu.dot = 0;
            nes->ppu.scanline++;
            if (nes->ppu.scanline == 262u) {
                nes->ppu.scanline = 0;
                nes->ppu.odd_frame ^= 1u;
            }
            /* Dot 0 is idle; on a rendering line the bus shows the low-plane address of the
               pending tile, otherwise v. [NESdev Wiki, PPU rendering] */
            if (rendering_active(nes))
                set_bus(nes, bg_pattern_address(nes, 0u));
            else
                set_bus(nes, nes->ppu.v);
        }
        /* Visible pixels are produced as the PPU crosses each visible dot.
           Background tile/attribute addressing follows the 2C02 scroll fields. [HWP.02][HWP.05] */
        if (nes->ppu.scanline < NESTURBATOR_HEIGHT && nes->ppu.dot >= 1u &&
            nes->ppu.dot <= NESTURBATOR_WIDTH && nes->ppu.video_output != NULL) {
            uint32_t x = (uint32_t)nes->ppu.dot - 1u;
            uint32_t y = nes->ppu.scanline;
            nes->ppu.video_output[(size_t)y * nes->ppu.video_pitch + x] = compose_pixel(nes, x);
        }
    }
}

/* A $2007 access adds 1 or 32 to v. While rendering it instead applies a coarse X and a Y
   increment together, whatever PPUCTRL bit 2 says. [NESdev Wiki, PPU scrolling "$2007
   reads and writes"] With rendering off the bus follows v at once. */
static void advance_v_after_data_access(struct nesturbator *nes)
{
    if (rendering_active(nes)) {
        inc_coarse_x(nes);
        inc_y(nes);
    } else {
        nes->ppu.v = (uint16_t)((nes->ppu.v + ((nes->ppu.control & 4u) ? 32u : 1u)) & 0x7fffu);
        set_bus(nes, nes->ppu.v);
    }
}

uint8_t nesturbator__ppu_register_read(struct nesturbator *nes, uint16_t reg)
{
    uint8_t value = nes->ppu.io_bus;
    if (reg == 0x2002u) {
        value = (uint8_t)((nes->ppu.status & 0xe0u) | (nes->ppu.io_bus & 0x1fu));
        nes->ppu.status &= 0x7fu;
        /* A read on scanline 241 before dot 1 suppresses this frame's flag
           and NMI edge. Reads at/after dot 1 observe and clear the flag. [HWP.03] */
        if (nes->ppu.scanline == 241u && nes->ppu.dot == 0u)
            nes->ppu.vblank_suppress = 1u;
        nes->ppu.address_latch = 0;
        nes->cpu.nmi_pending = 0u;
    } else if (reg == 0x2004u) {
        value = nes->ppu.oam[nes->ppu.oam_addr];
    } else if (reg == 0x2007u) {
        uint8_t memory_value = nesturbator__ppu_read(nes, nes->ppu.v);
        if (nes->ppu.v < 0x3f00u) {
            uint8_t old = nes->ppu.read_buffer;
            nes->ppu.read_buffer = memory_value;
            value = old;
        } else {
            /* Palette RAM returns immediately but the external PPU bus fetch
               still fills the read buffer from its $2Fxx nametable mirror. [CF.01] */
            nes->ppu.read_buffer = nesturbator__ppu_read(nes, (uint16_t)(nes->ppu.v - 0x1000u));
            value = nesturbator__ppu_read(nes, nes->ppu.v);
            value = (uint8_t)((value & 0x3fu) | (nes->ppu.io_bus & 0xc0u));
            if ((nes->ppu.mask & 1u) != 0u)
                value &= 0xf0u;
        }
        advance_v_after_data_access(nes);
    }
    nes->ppu.io_bus = value;
    nes->ppu.io_bus_age = 0u;
    return value;
}

/* Soft reset. NESdev Wiki "PPU power up state": "The PPU comes out of power and reset at the
   top of the picture", so the position becomes scanline 0 dot 0. Writes to PPUCTRL, PPUMASK,
   PPUSCROLL and PPUADDR "are ignored if earlier than ~29658 CPU clocks after reset"; this
   implementation ignores them until scanline 261 dot 1, which is 29,667 CPU cycles from the
   start of the reset (29,660 from the first instruction). The VBL flag is "unchanged by reset"
   and so are v, OAM address and video memory. Control, mask, the w latch, t, fine X, the read
   buffer and the NMI latches are cleared. vblank_suppress belongs to the position left behind
   and is cleared with it. The caller catches the PPU up to the CPU first. */
void nesturbator__ppu_reset(struct nesturbator *nes)
{
    nes->ppu.scanline = 0u;
    nes->ppu.dot = 0u;
    nes->ppu.control = 0u;
    nes->ppu.mask = 0u;
    nes->ppu.address_latch = 0u;
    nes->ppu.t = 0u;
    nes->ppu.fine_x = 0u;
    nes->ppu.read_buffer = 0u;
    nes->ppu.odd_frame = 0u;
    nes->ppu.vblank_suppress = 0u;
    /* The PPU file may not call cpu.c (bus.unit link trap), so the CPU's latches are set here. */
    nes->cpu.nmi_pending = 0u;
    nes->cpu.nmi_prev = 0u;
    nes->ppu.reset_flag = 1u;
}

void nesturbator__ppu_register_write(struct nesturbator *nes, uint16_t reg, uint8_t value)
{
    /* PPU I/O retains its last register write independently of CPU bus traffic. [HWP.04] */
    nes->ppu.io_bus = value;
    nes->ppu.io_bus_age = 0u;
    /* After a reset these four registers ignore writes, without touching the w latch. [NESdev
       Wiki, PPU registers and PPU power up state] */
    if (nes->ppu.reset_flag != 0u &&
        (reg == 0x2000u || reg == 0x2001u || reg == 0x2005u || reg == 0x2006u))
        return;
    if (reg == 0x2000u) {
        nes->ppu.control = value;
        nes->ppu.t = (uint16_t)((nes->ppu.t & 0xf3ffu) | ((value & 3u) << 10));
        if ((value & 0x80u) != 0u && (nes->ppu.status & 0x80u) != 0u)
            nes->cpu.nmi_pending = 1u;
        else
            nes->cpu.nmi_pending = 0u;
    } else if (reg == 0x2001u) {
        nes->ppu.mask = value;
    } else if (reg == 0x2003u) {
        nes->ppu.oam_addr = value;
    } else if (reg == 0x2004u) {
        nes->ppu.oam[nes->ppu.oam_addr++] = value;
    } else if (reg == 0x2005u) {
        if (nes->ppu.address_latch == 0u) {
            nes->ppu.fine_x = (uint8_t)(value & 7u);
            nes->ppu.t = (uint16_t)((nes->ppu.t & 0xffe0u) | (value >> 3));
        } else {
            nes->ppu.t =
                (uint16_t)((uint32_t)(nes->ppu.t & 0x8c1fu) | ((uint32_t)(value & 0xf8u) << 2) |
                           ((uint32_t)(value & 7u) << 12));
        }
        nes->ppu.address_latch ^= 1u;
    } else if (reg == 0x2006u) {
        if (nes->ppu.address_latch == 0u) {
            nes->ppu.t =
                (uint16_t)((uint32_t)(nes->ppu.t & 0x00ffu) | ((uint32_t)(value & 0x3fu) << 8));
        } else {
            nes->ppu.t = (uint16_t)((nes->ppu.t & 0x7f00u) | value);
            nes->ppu.v = nes->ppu.t;
            if (!rendering_active(nes))
                set_bus(nes, nes->ppu.v);
        }
        nes->ppu.address_latch ^= 1u;
    } else if (reg == 0x2007u) {
        nesturbator__ppu_write(nes, nes->ppu.v, value);
        advance_v_after_data_access(nes);
    }
}

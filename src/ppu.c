/* PPU memory, registers, dot clock and native background output. */
#include "internal.h"
#include <string.h>

static uint16_t background_pixel(struct nesturbator *nes, uint32_t x, uint32_t y, uint8_t *opaque)
{
    uint8_t color = 0u;
    uint8_t subpalette = 0u;
    if ((nes->ppu.mask & 0x08u) != 0u && (x >= 8u || (nes->ppu.mask & 0x02u) != 0u)) {
        uint32_t scroll_x = (uint32_t)(nes->ppu.t & 0x001fu) * 8u + nes->ppu.fine_x + x;
        uint32_t scroll_y = (uint32_t)((nes->ppu.t >> 5) & 0x001fu) * 8u +
                            (uint32_t)((nes->ppu.t >> 12) & 7u) + y;
        uint32_t tile_x = scroll_x / 8u;
        uint32_t tile_y = scroll_y / 8u;
        uint32_t fine_x = scroll_x & 7u;
        uint32_t fine_y = scroll_y & 7u;
        uint32_t nt_x = (uint32_t)((nes->ppu.t >> 10) & 1u) + tile_x / 32u;
        uint32_t nt_y = (uint32_t)((nes->ppu.t >> 11) & 1u) + tile_y / 30u;
        uint32_t table = (nt_y & 1u) * 2u + (nt_x & 1u);
        uint16_t nt_base = (uint16_t)(0x2000u + table * 0x400u);
        uint32_t column = tile_x & 31u;
        uint32_t row = tile_y % 30u;
        uint8_t tile = nesturbator__ppu_read(nes, (uint16_t)(nt_base + row * 32u + column));
        uint16_t pattern_base = (nes->ppu.control & 0x10u) != 0u ? 0x1000u : 0u;
        uint16_t pattern = (uint16_t)(pattern_base + (uint16_t)tile * 16u + fine_y);
        uint8_t shift = (uint8_t)(7u - fine_x);
        uint8_t lo = nesturbator__ppu_read(nes, pattern);
        uint8_t hi = nesturbator__ppu_read(nes, (uint16_t)(pattern + 8u));
        color = (uint8_t)(((lo >> shift) & 1u) | (((hi >> shift) & 1u) << 1));
        if (color != 0u) {
            uint16_t attribute = (uint16_t)(nt_base + 0x03c0u + (row / 4u) * 8u + column / 4u);
            uint8_t attr = nesturbator__ppu_read(nes, attribute);
            uint8_t quadrant = (uint8_t)(((row & 2u) << 1) | (column & 2u));
            subpalette = (uint8_t)((attr >> quadrant) & 3u);
        }
    }

    *opaque = color != 0u;
    /* Universal colour zero aliases $3F00 for every background subpalette. [HWP.07] */
    uint8_t palette_index = color == 0u ? 0u : (uint8_t)(subpalette * 4u + color);
    uint8_t palette_value = (uint8_t)(nes->ppu.palette[palette_index] & 0x3fu);
    if ((nes->ppu.mask & 0x01u) != 0u)
        palette_value &= 0x30u;
    return (uint16_t)(palette_value | (uint16_t)((nes->ppu.mask >> 5) & 7u) << 6);
}

static void sprite_evaluate(struct nesturbator *nes)
{
    struct nesturbator__ppu *ppu = &nes->ppu;
    if (ppu->scanline > 239u)
        return;
    if (ppu->dot == 65u) {
        ppu->sprite_count = 0u;
        ppu->eval_n = 0u;
        ppu->eval_target = (uint8_t)(ppu->scanline + 1u);
        memset(ppu->secondary_oam, 0xff, sizeof ppu->secondary_oam);
    }
    if ((ppu->mask & 0x18u) == 0u)
        return;
    if (ppu->dot < 65u || ppu->dot > 256u)
        return;
    if (ppu->eval_n >= 64u)
        return;

    /* The 2C02 clears secondary OAM then alternates primary-OAM reads and
       evaluation during dots 1-256. This captures each candidate when its
       Y byte is evaluated, rather than sampling all of OAM at pixel time.
       [HWP.06] */
    if ((ppu->dot & 1u) != 0u) {
        ppu->eval_latch = ppu->oam[(uint16_t)ppu->eval_n * 4u];
        return;
    }
    uint16_t top = (uint16_t)ppu->eval_latch + 1u;
    uint8_t height = (ppu->control & 0x20u) != 0u ? 16u : 8u;
    if ((uint16_t)ppu->eval_target >= top && (uint16_t)ppu->eval_target < top + height) {
        if (ppu->sprite_count < 8u) {
            uint8_t slot = ppu->sprite_count++;
            uint16_t base = (uint16_t)ppu->eval_n * 4u;
            for (uint8_t byte = 0u; byte < 4u; byte++)
                ppu->secondary_oam[(uint16_t)slot * 4u + byte] = ppu->oam[base + byte];
            ppu->sprite_zero[slot] = ppu->eval_n == 0u;
        } else {
            ppu->status |= 0x20u;
        }
    }
    ppu->eval_n++;
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
    if (ppu->dot != 257u)
        return;
    uint8_t height = (ppu->control & 0x20u) != 0u ? 16u : 8u;
    uint16_t target = (uint16_t)ppu->scanline + 1u;
    for (uint8_t i = 0u; i < ppu->sprite_count; i++) {
        uint16_t base = (uint16_t)i * 4u;
        uint8_t y = ppu->secondary_oam[base];
        uint8_t tile = ppu->secondary_oam[base + 1u];
        uint8_t attr = ppu->secondary_oam[base + 2u];
        uint8_t row = (uint8_t)(target - ((uint16_t)y + 1u));
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
            pattern += (uint16_t)tile * 16u;
        }
        ppu->sprite_lo[i] = nesturbator__ppu_read(nes, (uint16_t)(pattern + row));
        ppu->sprite_hi[i] = nesturbator__ppu_read(nes, (uint16_t)(pattern + row + 8u));
        if ((attr & 0x40u) != 0u) {
            ppu->sprite_lo[i] = reverse_bits(ppu->sprite_lo[i]);
            ppu->sprite_hi[i] = reverse_bits(ppu->sprite_hi[i]);
        }
        ppu->sprite_x[i] = ppu->secondary_oam[base + 3u];
        ppu->sprite_attr[i] = attr;
    }
}

static uint16_t compose_pixel(struct nesturbator *nes, uint32_t x, uint32_t y)
{
    uint8_t bg_opaque = 0u;
    uint16_t bg = background_pixel(nes, x, y, &bg_opaque);
    struct nesturbator__ppu *ppu = &nes->ppu;
    if ((ppu->mask & 0x10u) == 0u || (x < 8u && (ppu->mask & 0x04u) == 0u))
        return bg;
    for (uint8_t i = 0u; i < ppu->sprite_count; i++) {
        uint8_t sx = ppu->sprite_x[i];
        if (x < sx || x >= (uint32_t)sx + 8u)
            continue;
        uint8_t bit = (uint8_t)(7u - (x - sx));
        uint8_t color = (uint8_t)(((ppu->sprite_lo[i] >> bit) & 1u) |
                                  (((ppu->sprite_hi[i] >> bit) & 1u) << 1));
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
        if (nes->cart.bytes == NULL)
            return 0;
        return nes->cart.chr[addr & 0x1fffu];
    }
    if (addr < 0x3f00u) {
        uint16_t offset = (uint16_t)((addr - 0x2000u) & 0x0fffu);
        uint16_t table = (uint16_t)(offset >> 10);
        uint16_t within = (uint16_t)(offset & 0x03ffu);
        uint16_t ciram;
        /* iNES flags 6 bit 0 selects vertical (1) or horizontal (0)
           mirroring for mapper 0. Four-screen boards are rejected at load. [HWP.14] */
        if (nes->cart.bytes != NULL && (nes->cart.bytes[6] & 1u) != 0u)
            ciram = (uint16_t)((table & 1u) * 0x400u + within);
        else
            ciram = (uint16_t)((table >> 1) * 0x400u + within);
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
        if (nes->cart.chr_is_ram)
            nes->cart.chr[addr & 0x1fffu] = value;
    } else if (addr < 0x3f00u) {
        uint16_t offset = (uint16_t)((addr - 0x2000u) & 0x0fffu);
        uint16_t table = (uint16_t)(offset >> 10);
        uint16_t within = (uint16_t)(offset & 0x03ffu);
        uint16_t ciram;
        if (nes->cart.bytes != NULL && (nes->cart.bytes[6] & 1u) != 0u)
            ciram = (uint16_t)((table & 1u) * 0x400u + within);
        else
            ciram = (uint16_t)((table >> 1) * 0x400u + within);
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
        nes->ppu.dot++;
        /* NTSC vblank starts at scanline 241 dot 1 and ends at 261 dot 1.
           Enabling NMI with vblank active raises the CPU's pending edge. [HWP.03] */
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
        }
        sprite_evaluate(nes);
        sprite_fetch(nes);
        /* Rendering skips pre-render dot 340 on odd NTSC frames. [HWP.03] */
        /* Rendering skips pre-render dot 340 on odd NTSC frames. [HWP.03] */
        if (nes->ppu.scanline == 261u && nes->ppu.dot == 340u && nes->ppu.odd_frame != 0u &&
            (nes->ppu.mask & 0x18u) != 0u) {
            nes->ppu.scanline = 0u;
            nes->ppu.dot = 0u;
            nes->ppu.odd_frame = 0u;
            continue;
        }
        if (nes->ppu.dot == 341u) {
            nes->ppu.dot = 0;
            nes->ppu.scanline++;
            if (nes->ppu.scanline == 262u) {
                nes->ppu.scanline = 0;
                nes->ppu.odd_frame ^= 1u;
            }
        }
        /* Visible pixels are produced as the PPU crosses each visible dot.
           Background tile/attribute addressing follows the 2C02 scroll fields. [HWP.02][HWP.05] */
        if (nes->ppu.scanline >= 1u && nes->ppu.scanline <= NESTURBATOR_HEIGHT &&
            nes->ppu.dot >= 1u && nes->ppu.dot <= NESTURBATOR_WIDTH && nes->ppu.video_output != NULL) {
            uint32_t x = (uint32_t)nes->ppu.dot - 1u;
            uint32_t y = (uint32_t)nes->ppu.scanline - 1u;
            nes->ppu.video_output[(size_t)y * nes->ppu.video_pitch + x] = compose_pixel(nes, x, y);
        }
    }
}

uint8_t nesturbator__ppu_register_read(struct nesturbator *nes, uint16_t reg)
{
    if (reg == 0x2002u) {
        nes->bus.open_bus = (uint8_t)((nes->ppu.status & 0xe0u) | (nes->bus.open_bus & 0x1fu));
        nes->ppu.status &= 0x7fu;
        /* A read on scanline 241 before dot 1 suppresses this frame's flag
           and NMI edge. Reads at/after dot 1 observe and clear the flag. [HWP.03] */
        if (nes->ppu.scanline == 241u && nes->ppu.dot == 0u)
            nes->ppu.vblank_suppress = 1u;
        nes->ppu.address_latch = 0;
        nes->cpu.nmi_pending = 0u;
    } else if (reg == 0x2004u) {
        nes->bus.open_bus = nes->ppu.oam[nes->ppu.oam_addr];
    } else if (reg == 0x2007u) {
        uint8_t value = nesturbator__ppu_read(nes, nes->ppu.v);
        if (nes->ppu.v < 0x3f00u) {
            uint8_t old = nes->ppu.read_buffer;
            nes->ppu.read_buffer = value;
            value = old;
        }
        nes->bus.open_bus = value;
        nes->ppu.v = (uint16_t)((nes->ppu.v + ((nes->ppu.control & 4u) ? 32u : 1u)) & 0x7fffu);
    }
    return nes->bus.open_bus;
}

void nesturbator__ppu_register_write(struct nesturbator *nes, uint16_t reg, uint8_t value)
{
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
            nes->ppu.t = (uint16_t)((uint32_t)(nes->ppu.t & 0x8c1fu) | ((uint32_t)(value & 0xf8u) << 2) |
                                    ((uint32_t)(value & 7u) << 12));
        }
        nes->ppu.address_latch ^= 1u;
    } else if (reg == 0x2006u) {
        if (nes->ppu.address_latch == 0u) {
            nes->ppu.t = (uint16_t)((uint32_t)(nes->ppu.t & 0x00ffu) | ((uint32_t)(value & 0x3fu) << 8));
        } else {
            nes->ppu.t = (uint16_t)((nes->ppu.t & 0x7f00u) | value);
            nes->ppu.v = nes->ppu.t;
        }
        nes->ppu.address_latch ^= 1u;
    } else if (reg == 0x2007u) {
        nesturbator__ppu_write(nes, nes->ppu.v, value);
        nes->ppu.v = (uint16_t)((nes->ppu.v + ((nes->ppu.control & 4u) ? 32u : 1u)) & 0x7fffu);
    }
}

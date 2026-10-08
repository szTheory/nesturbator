/* PPU memory, registers, dot clock and native background output. */
#include "internal.h"

static uint16_t background_pixel(struct nesturbator *nes, uint32_t x, uint32_t y)
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

    /* Universal colour zero aliases $3F00 for every background subpalette. [HWP.07] */
    uint8_t palette_index = color == 0u ? 0u : (uint8_t)(subpalette * 4u + color);
    uint8_t palette_value = (uint8_t)(nes->ppu.palette[palette_index] & 0x3fu);
    if ((nes->ppu.mask & 0x01u) != 0u)
        palette_value &= 0x30u;
    return (uint16_t)(palette_value | (uint16_t)((nes->ppu.mask >> 5) & 7u) << 6);
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
            nes->ppu.video_output[(size_t)y * nes->ppu.video_pitch + x] = background_pixel(nes, x, y);
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

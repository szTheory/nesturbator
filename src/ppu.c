/* PPU memory, registers, dot clock and native background output. */
#include "internal.h"

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
    }
}

void nesturbator__ppu_render(struct nesturbator *nes, uint16_t *video, uint32_t pitch)
{
    uint32_t y, x;
    uint16_t base = (nes->ppu.control & 0x10u) != 0u ? 0x1000u : 0u;
    for (y = 0; y < NESTURBATOR_HEIGHT; y++) {
        for (x = 0; x < NESTURBATOR_WIDTH; x++) {
            uint16_t nt = (uint16_t)(0x2000u + ((nes->ppu.control & 3u) * 0x400u));
            uint8_t tile = nes->ppu.nametable[((nt - 0x2000u) & 0x07ffu) + (y / 8u) * 32u + x / 8u];
            uint8_t row = (uint8_t)(y & 7u), col = (uint8_t)(x & 7u);
            uint8_t plane = 0;
            if ((nes->ppu.mask & 0x08u) != 0u && (x >= 8u || (nes->ppu.mask & 0x02u) != 0u)) {
                uint16_t pat = (uint16_t)(base + (uint16_t)tile * 16u + row);
                uint8_t lo = nesturbator__ppu_read(nes, pat);
                uint8_t hi = nesturbator__ppu_read(nes, (uint16_t)(pat + 8u));
                plane = (uint8_t)(((lo >> (7u - col)) & 1u) | (((hi >> (7u - col)) & 1u) << 1));
            }
            video[(size_t)y * pitch + x] = (uint16_t)(nes->ppu.palette[plane] & 0x3fu);
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

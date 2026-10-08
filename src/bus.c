/* The CPU's bus: time, address decode and the open-bus latch (D-12).

   Each call is one CPU cycle of 24 ticks. On NTSC the cycle starts where M2
   falls and M2 is high for the last 15 of its 24 ticks, with data moving
   while M2 is high (NES-HARDWARE-CPU-APU, section 2, part 1). So a call
   advances 9 ticks to M2's rise, lets the PPU catch up, advances 15 more to
   M2's fall, lets the PPU catch up again, and then does the access. */
#include "internal.h"

static uint8_t ppu_read(struct nesturbator *nes, uint16_t addr)
{
    addr &= 0x3fffu;
    if (addr < 0x2000u) {
        if (nes->cart.bytes == NULL)
            return 0;
        return nes->cart.chr[addr & 0x1fffu];
    }
    if (addr < 0x3f00u)
        return nes->ppu.nametable[(addr - 0x2000u) & 0x07ffu];
    addr = (uint16_t)((addr - 0x3f00u) & 0x1fu);
    if ((addr & 0x13u) == 0x10u)
        addr &= 0x0fu;
    return (uint8_t)(nes->ppu.palette[addr] & 0x3fu);
}

static void ppu_write(struct nesturbator *nes, uint16_t addr, uint8_t value)
{
    addr &= 0x3fffu;
    if (addr < 0x2000u) {
        if (nes->cart.chr_is_ram)
            nes->cart.chr[addr & 0x1fffu] = value;
    } else if (addr < 0x3f00u) {
        nes->ppu.nametable[(addr - 0x2000u) & 0x07ffu] = value;
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
            nes->ppu.status |= 0x80u;
            if ((nes->ppu.control & 0x80u) != 0u)
                nes->cpu.nmi_pending = 1u;
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

static void ppu_catch_up(struct nesturbator *nes)
{
    nesturbator__ppu_run_until(nes, nes->ticks);
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
                uint8_t lo = ppu_read(nes, pat);
                uint8_t hi = ppu_read(nes, (uint16_t)(pat + 8u));
                plane = (uint8_t)(((lo >> (7u - col)) & 1u) | (((hi >> (7u - col)) & 1u) << 1));
            }
            video[(size_t)y * pitch + x] = (uint16_t)(nes->ppu.palette[plane] & 0x3fu);
        }
    }
}

/* Time for one CPU cycle, with the PPU brought up to both M2 edges. */
static void cycle(struct nesturbator *nes)
{
    nes->ticks += 9u;
    ppu_catch_up(nes);
    nes->ticks += 15u;
    ppu_catch_up(nes);
}

uint8_t nesturbator__bus_read(struct nesturbator *nes, uint16_t addr)
{
    cycle(nes);
    /* CPU RAM mirrors, PPU registers, and NROM PRG are decoded here. */
    if (addr < 0x2000u) {
        nes->bus.open_bus = nes->bus.ram[addr & 0x7FFu];
    } else if (addr < 0x4000u) {
        uint16_t reg = (uint16_t)(0x2000u | (addr & 7u));
        if (reg == 0x2002u) {
            nes->bus.open_bus = (uint8_t)((nes->ppu.status & 0xe0u) | (nes->bus.open_bus & 0x1fu));
            nes->ppu.status &= 0x7fu;
            nes->ppu.address_latch = 0;
            nes->cpu.nmi_pending = 0u;
        } else if (reg == 0x2004u) {
            nes->bus.open_bus = nes->ppu.oam[nes->ppu.oam_addr];
        } else if (reg == 0x2007u) {
            uint8_t value = ppu_read(nes, nes->ppu.v);
            if (nes->ppu.v < 0x3f00u) {
                uint8_t old = nes->ppu.read_buffer;
                nes->ppu.read_buffer = value;
                value = old;
            }
            nes->bus.open_bus = value;
            nes->ppu.v = (uint16_t)((nes->ppu.v + ((nes->ppu.control & 4u) ? 32u : 1u)) & 0x7fffu);
        }
    } else if (addr >= 0x8000u && nes->cart.bytes != NULL) {
        nes->bus.open_bus = nesturbator__cart_read(nes, addr);
    }
    return nes->bus.open_bus;
}

void nesturbator__bus_write(struct nesturbator *nes, uint16_t addr, uint8_t value)
{
    cycle(nes);
    nes->bus.open_bus = value;
    if (addr < 0x2000u) {
        nes->bus.ram[addr & 0x7FFu] = value;
    } else if (addr < 0x4000u) {
        uint16_t reg = (uint16_t)(0x2000u | (addr & 7u));
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
            }
            nes->ppu.address_latch ^= 1u;
        } else if (reg == 0x2007u) {
            ppu_write(nes, nes->ppu.v, value);
            nes->ppu.v = (uint16_t)((nes->ppu.v + ((nes->ppu.control & 4u) ? 32u : 1u)) & 0x7fffu);
        }
    }
}

uint8_t nesturbator__cart_read(struct nesturbator *nes, uint16_t addr)
{
    return nes->cart.prg[(addr - 0x8000u) & 0x3fffu];
}

/* The CPU's bus: time, address decode and the open-bus latch (D-12).

   Each call is one CPU cycle of 24 ticks. On NTSC the cycle starts where M2
   falls and M2 is high for the last 15 of its 24 ticks, with data moving
   while M2 is high (NES-HARDWARE-CPU-APU, section 2, part 1). So a call
   advances 9 ticks to M2's rise, lets the PPU catch up, advances 15 more to
   M2's fall, lets the PPU catch up again, and then does the access. */
#include "internal.h"

static void ppu_catch_up(struct nesturbator *nes)
{
    nesturbator__ppu_run_until(nes, nes->ticks);
}


/* Time for one CPU cycle, with the PPU brought up to both M2 edges. */
static void cycle(struct nesturbator *nes)
{
    nes->ticks += 9u;
    ppu_catch_up(nes);
    nes->ticks += 15u;
    ppu_catch_up(nes);
}

static void oam_dma(struct nesturbator *nes, uint8_t page)
{
    /* RDY takes effect after the $4014 write. A halt cycle is always spent;
       an extra alignment cycle is needed when the write leaves the CPU on
       the even phase. Each source byte uses the ordinary CPU bus decoder,
       and each destination byte occupies a separate put cycle. [HWC.01] */
    uint8_t align = (uint8_t)(((nes->ticks / 24u) & 1u) == 0u);
    uint16_t halted_read = nes->cpu.pc;
    (void)nesturbator__bus_read(nes, halted_read);
    if (align != 0u)
        (void)nesturbator__bus_read(nes, halted_read);
    for (uint16_t offset = 0u; offset < 256u; offset++) {
        uint8_t value = nesturbator__bus_read(nes, (uint16_t)(((uint16_t)page << 8) | offset));
        cycle(nes);
        nes->bus.open_bus = value;
        nesturbator__ppu_register_write(nes, 0x2004u, value);
    }
}

uint8_t nesturbator__bus_read(struct nesturbator *nes, uint16_t addr)
{
    cycle(nes);
    /* CPU RAM mirrors, PPU registers, and NROM PRG are decoded here. */
    if (addr < 0x2000u) {
        nes->bus.open_bus = nes->bus.ram[addr & 0x7FFu];
    } else if (addr < 0x4000u) {
        uint16_t reg = (uint16_t)(0x2000u | (addr & 7u));
        nes->bus.open_bus = nesturbator__ppu_register_read(nes, reg);
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
        nesturbator__ppu_register_write(nes, reg, value);
    } else if (addr == 0x4014u) {
        oam_dma(nes, value);
    }
}

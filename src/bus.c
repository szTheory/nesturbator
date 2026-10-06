/* The CPU's bus: time, address decode and the open-bus latch (D-12).

   Each call is one CPU cycle of 24 ticks. On NTSC the cycle starts where M2
   falls and M2 is high for the last 15 of its 24 ticks, with data moving
   while M2 is high (NES-HARDWARE-CPU-APU, section 2, part 1). So a call
   advances 9 ticks to M2's rise, lets the PPU catch up, advances 15 more to
   M2's fall, lets the PPU catch up again, and then does the access. */
#include "internal.h"

/* Runs the PPU up to nes->ticks. There is no PPU yet. */
static void ppu_catch_up(struct nesturbator *nes)
{
    (void)nes;
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
    /* 0x0000 to 0x1FFF: the 2048 bytes of RAM, mirrored four times. Nothing
       else answers yet, so any other read returns the last bus value. */
    if (addr < 0x2000u) {
        nes->bus.open_bus = nes->bus.ram[addr & 0x7FFu];
    }
    return nes->bus.open_bus;
}

void nesturbator__bus_write(struct nesturbator *nes, uint16_t addr, uint8_t value)
{
    cycle(nes);
    nes->bus.open_bus = value;
    if (addr < 0x2000u) {
        nes->bus.ram[addr & 0x7FFu] = value;
    }
}

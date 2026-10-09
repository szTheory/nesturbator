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
    /* IRQ is polled from the level present at the start of this cycle. The
       previous value becomes the instruction-end poll sample. [HWC.02] */
    nes->cpu.poll_latch = nes->cpu.irq_line;
    /* CPU cycles alternate the APU's get and put slots. Keep this phase at
       the bus seam so register races and DMA alignment use the same cycle
       state as reads and writes. [HWC.01, HWC.09] */
    nes->bus.apu_get_put_phase ^= 1u;
    nesturbator__apu_clock(nes);
    nes->ticks += 15u;
    ppu_catch_up(nes);
}

static uint8_t bus_read_data(struct nesturbator *nes, uint16_t addr)
{
    /* CPU RAM mirrors, PPU registers, and NROM PRG are decoded here. */
    if (addr == 0x4016u || addr == 0x4017u) {
        uint8_t port = (uint8_t)(addr - 0x4016u);
        uint8_t bit;
        if (nes->bus.controller_strobe != 0u) {
            bit = (uint8_t)(nes->bus.input_buttons[port] & 1u);
        } else {
            bit = (uint8_t)(nes->bus.controller_shift[port] & 1u);
            nes->bus.controller_shift[port] =
                (uint8_t)((nes->bus.controller_shift[port] >> 1) | 0x80u);
        }
        /* D0 is serial data, D6 reads high, and D5/D7 retain open bus. */
        nes->bus.open_bus = (uint8_t)((nes->bus.open_bus & 0xa0u) | 0x40u | bit);
    } else if (addr == 0x4015u) {
        nes->bus.open_bus =
            (uint8_t)((nesturbator__apu_status_read(nes) & 0xdfu) | (nes->bus.open_bus & 0x20u));
    } else if (addr < 0x2000u) {
        nes->bus.open_bus = nes->bus.ram[addr & 0x7FFu];
    } else if (addr < 0x4000u) {
        uint16_t reg = (uint16_t)(0x2000u | (addr & 7u));
        nes->bus.open_bus = nesturbator__ppu_register_read(nes, reg);
    } else if (addr >= 0x8000u && nes->cart.bytes != NULL) {
        nes->bus.open_bus = nesturbator__cart_read(nes, addr);
    }
    return nes->bus.open_bus;
}

static uint8_t dmc_dma(struct nesturbator *nes, uint16_t parked_read)
{
    struct nesturbator__dmc *dmc = &nes->apu.dmc;
    if (dmc->dma_pending == 0u || dmc->enable_delay != 0u ||
        nes->bus.apu_get_put_phase != dmc->dma_halt_phase || dmc->remaining == 0u ||
        nes->cart.prg == NULL)
        return 0u;

    /* RDY halts the CPU only on reads. The first halted cycle repeats the
       parked CPU access, including controller and PPU register side effects. */
    (void)bus_read_data(nes, parked_read);
    cycle(nes); /* DMC dummy read cycle. [HWC.01, HWC.09] */
    (void)bus_read_data(nes, parked_read);
    if (nes->bus.apu_get_put_phase != 0u) {
        cycle(nes); /* Alignment read on PUT. [HWC.01, HWC.09] */
        (void)bus_read_data(nes, parked_read);
    }
    cycle(nes); /* DMC memory get. [HWC.01, HWC.09] */
    dmc->sample_buffer = nesturbator__cart_read(nes, dmc->address);
    nes->bus.open_bus = dmc->sample_buffer;
    dmc->buffer_empty = 0u;
    dmc->dma_pending = 0u;
    dmc->address = dmc->address == 0xffffu ? 0x8000u : (uint16_t)(dmc->address + 1u);
    dmc->remaining--;
    if (dmc->remaining == 0u) {
        if ((dmc->reg[0] & 0x40u) != 0u) {
            dmc->address = (uint16_t)(0xc000u | ((uint32_t)dmc->reg[2] << 6));
            dmc->remaining = (uint16_t)(((uint32_t)dmc->reg[3] << 4) | 1u);
        } else if ((dmc->reg[0] & 0x80u) != 0u) {
            dmc->irq = 1u;
            nes->cpu.irq_line = 1u;
        }
    }
    return 1u;
}

static uint8_t bus_read_cycle(struct nesturbator *nes, uint16_t addr)
{
    cycle(nes);
    if (nes->apu.dmc.dma_pending != 0u && dmc_dma(nes, addr) != 0u) {
        /* The CPU repeats its parked read once more when RDY is released.
           This is the resumed bus cycle after the DMC memory get, and is
           distinct from the GET cycle that filled the sample buffer. [HWC.01] */
        cycle(nes);
    }
    uint8_t value = bus_read_data(nes, addr);
    return value;
}

static void oam_dma(struct nesturbator *nes, uint8_t page, uint16_t halted_read)
{
    /* RDY takes effect after the $4014 write. A halt cycle is always spent;
       an extra alignment cycle is needed when the write leaves the CPU on
       the even phase. Each source byte uses the ordinary CPU bus decoder,
       and each destination byte occupies a separate put cycle. [HWC.01] */
    uint8_t align = (uint8_t)(((nes->ticks / 24u) & 1u) == 0u);
    (void)bus_read_cycle(nes, halted_read);
    if (align != 0u)
        (void)bus_read_cycle(nes, halted_read);
    for (uint16_t offset = 0u; offset < 256u; offset++) {
        uint8_t value = bus_read_cycle(nes, (uint16_t)(((uint16_t)page << 8) | offset));
        cycle(nes);
        nes->bus.open_bus = value;
        nesturbator__ppu_register_write(nes, 0x2004u, value);
    }
}

uint8_t nesturbator__bus_read(struct nesturbator *nes, uint16_t addr)
{
    if (nes->bus.oam_dma_pending != 0u) {
        uint8_t page = nes->bus.oam_dma_page;
        nes->bus.oam_dma_pending = 0u;
        oam_dma(nes, page, addr);
    }
    return bus_read_cycle(nes, addr);
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
        nes->bus.oam_dma_page = value;
        nes->bus.oam_dma_pending = 1u;
    } else if (addr == 0x4016u) {
        uint8_t strobe = (uint8_t)(value & 1u);
        if (nes->bus.controller_strobe != 0u && strobe == 0u) {
            nes->bus.controller_latch[0] = nes->bus.input_buttons[0];
            nes->bus.controller_latch[1] = nes->bus.input_buttons[1];
            nes->bus.controller_shift[0] = nes->bus.controller_latch[0];
            nes->bus.controller_shift[1] = nes->bus.controller_latch[1];
        }
        nes->bus.controller_strobe = strobe;
    } else if (addr >= 0x4000u && addr <= 0x4017u) {
        nesturbator__apu_write(nes, addr, value);
    }
}

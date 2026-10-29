/* The CPU's bus: time, address decode and the open-bus latch (D-12).

   Each call is one CPU cycle of 24 ticks. On NTSC the cycle starts where M2
   falls and M2 is high for the last 15 of its 24 ticks, with data moving
   while M2 is high (NES-HARDWARE-CPU-APU, section 2, part 1). So a call
   advances 9 ticks to M2's rise and catches the PPU up before sampling
   PPUSTATUS.7; advances 15 more to M2's fall. At an equal PPU/CPU timestamp,
   the selected global model order is bus side effect, detector sample, then
   PPU transition. This is
   a deterministic model policy; finite-margin circuit cases support the
   phase endpoints, but do not establish physical setup/hold or paired-chip
   propagation at a tie. AccuracyCoin describes the split PPUSTATUS samples
   (TEST_2002FlagTiming, pinned at 6bee919); Visual2A03 measures internal phi2
   separately from widened M2 (Phase 20 oracle research). */
#include "internal.h"

static void ppu_catch_up(struct nesturbator *nes)
{
    nesturbator__ppu_run_until(nes, nes->ticks);
}

static void ppu_catch_up_before_close(struct nesturbator *nes)
{
    /* cycle() has advanced a complete 24-tick cycle before this call. */
    nesturbator__ppu_run_until(nes, nes->ticks - 1u);
}

/* Both interrupt inputs are sampled at internal phi2 close: NMI retains a
   recognized edge, while IRQ retains the level. Ordinary instruction polls read this
   captured history at phi2 opening; staged entry samples capture lines but
   do not create another instruction poll [HWC.02, HWC.17, Phase 21 D-08]. */
static void sample_interrupt_lines(struct nesturbator *nes)
{
    uint8_t nmi_line = nesturbator__ppu_nmi_line(nes);
    if (nmi_line != 0u && nes->cpu.nmi_prev == 0u &&
        nes->cpu.nmi_pending != NESTURBATOR_NMI_RECOGNIZED) {
        /* The vector-command pipe inhibits stage1 recognition after the C4
           choice. A newly sampled input there is only a candidate: it must
           stay asserted through the next T1, unlike an already recognized
           edge. Visual6502 Interrupt Hijacking, node1368; annex section three,
           arrival and hold expectations. */
        nes->cpu.nmi_pending =
            nes->cpu.entry_origin != NESTURBATOR_ENTRY_NONE && nes->cpu.entry_next_action >= 5u
                ? NESTURBATOR_NMI_INHIBITED
                : NESTURBATOR_NMI_RECOGNIZED;
    } else if (nmi_line == 0u && nes->cpu.nmi_pending == NESTURBATOR_NMI_INHIBITED) {
        nes->cpu.nmi_pending = NESTURBATOR_NMI_NONE;
    }
    nes->cpu.nmi_prev = nmi_line;
    nes->cpu.irq_captured = nes->cpu.irq_line;
}

static uint8_t is_ppu_status_address(uint16_t addr)
{
    return (uint8_t)(addr >= 0x2000u && addr < 0x4000u && (addr & 7u) == 2u);
}

/* Time for one CPU cycle, bringing the PPU up to just before close. Only a
   status read needs the earlier M2-rise catch-up. Return its composed status
   byte; callers apply bus side effects and capture before a tied PPU transition. */
static inline uint8_t cycle(struct nesturbator *nes, uint8_t status_read,
                            uint8_t defer_nmi_close_sample)
{
    uint8_t vblank = 0u;
    uint8_t status_value = 0u;
    if (nes->cpu.entry_origin == NESTURBATOR_ENTRY_NONE &&
        nes->cpu.nmi_pending == NESTURBATOR_NMI_INHIBITED) {
        /* First handler T1: qualify before its C1 branch poll. Only this
           unrecognized candidate consults the held input; ordinary captured
           edges keep Phase20's sticky history and close/open timing. */
        nes->cpu.nmi_pending = nesturbator__ppu_nmi_line(nes) != 0u ? NESTURBATOR_NMI_RECOGNIZED
                                                                    : NESTURBATOR_NMI_NONE;
    }
    nes->ticks += 9u;
    if (status_read != 0u) {
        ppu_catch_up(nes);
        vblank = (uint8_t)(nes->ppu.status & 0x80u);
        uint8_t suppress_vblank = (uint8_t)(nes->ppu.scanline == 241u && nes->ppu.dot == 0u);
        nesturbator__ppu_status_read_begin(nes, suppress_vblank);
    }
    /* CPU cycles alternate the APU's get and put slots. Keep this phase at
       the bus seam so register races and DMA alignment use the same cycle
       state as reads and writes. [HWC.01, HWC.09] */
    nes->bus.apu_get_put_phase ^= 1u;
    nesturbator__apu_clock(nes);
    /* M2 rises at +9 ticks, while the circuit oracle places internal phi2
       three ticks later. The CPU polls the request captured by the preceding
       close here; the physical PPU line itself is sampled at phi2 close. */
    nes->ticks += 3u;
    /* Only PPUSTATUS observes M2 rise. Other accesses and this history-only
       poll can defer PPU work until the pre-close catch-up. APU and mapper
       IRQ sources combine before the close-edge sample. */
    /* IRQ must use the preceding close sample too: a tied PPU A12 transition
       after that detector sample cannot feed this poll. [HWC.02, HWC.17] */
    if (nes->cpu.entry_origin == NESTURBATOR_ENTRY_NONE) {
        nes->cpu.poll_latch = nes->cpu.irq_captured;
        nes->cpu.nmi_polled = (uint8_t)(nes->cpu.nmi_pending == NESTURBATOR_NMI_RECOGNIZED);
    }
    nes->ticks += 12u;
    ppu_catch_up_before_close(nes);
    if (defer_nmi_close_sample == 0u) {
        /* An internal cycle has no bus side effect, so capture before the PPU
           applies a transition at this same integer timestamp. */
        sample_interrupt_lines(nes);
        ppu_catch_up(nes);
    }
    if (status_read != 0u)
        status_value = nesturbator__ppu_status_read_finish(nes, vblank);
    /* Count the cycle that just ran. A mapper write stamp is this count minus
       one: the zero-based index of the write's own cycle (D-14). */
    nes->cpu_cycle++;
    return status_value;
}

static inline uint8_t bus_read_data(struct nesturbator *nes, uint16_t addr, uint8_t status_value)
{
    /* CPU RAM mirrors, PPU registers, and NROM PRG are decoded here. */
    if (addr >= 0x4020u) {
        nes->bus.open_bus = nesturbator__map_cpu_read(nes, addr);
    } else if (addr == 0x4016u || addr == 0x4017u) {
        uint8_t port = (uint8_t)(addr - 0x4016u);
        uint8_t bit;
        if (nes->bus.controller_strobe != 0u) {
            bit = (uint8_t)(nes->bus.input_buttons[port] & 1u);
        } else {
            bit = (uint8_t)(nes->bus.controller_shift[port] & 1u);
            nes->bus.controller_shift[port] =
                (uint8_t)((nes->bus.controller_shift[port] >> 1) | 0x80u);
        }
        /* D0 is serial data; D5-D7 retain open bus. The usual D6 high comes
           from the preceding address byte, not the controller. AccuracyCoin
           6bee919 TEST_ImpliedDummyRead uses stack-driven $20/$00 operands. */
        nes->bus.open_bus = (uint8_t)((nes->bus.open_bus & 0xe0u) | bit);
    } else if (addr == 0x4015u) {
        nes->bus.open_bus =
            (uint8_t)((nesturbator__apu_status_read(nes) & 0xdfu) | (nes->bus.open_bus & 0x20u));
    } else if (addr < 0x2000u) {
        nes->bus.open_bus = nes->bus.ram[addr & 0x7FFu];
    } else if (addr < 0x4000u) {
        uint16_t reg = (uint16_t)(0x2000u | (addr & 7u));
        nes->bus.open_bus =
            reg == 0x2002u ? status_value : nesturbator__ppu_register_read(nes, reg);
    }
    return nes->bus.open_bus;
}

static uint8_t dmc_dma(struct nesturbator *nes, uint16_t parked_read, uint8_t *sample)
{
    struct nesturbator__dmc *dmc = &nes->apu.dmc;
    if (dmc->dma_pending == 0u || dmc->enable_delay != 0u ||
        nes->bus.apu_get_put_phase != dmc->dma_halt_phase || dmc->remaining == 0u ||
        nes->cart.prg == NULL)
        return 0u;

    /* RDY halts the CPU only on reads. The first halted cycle repeats the
       parked CPU access, including controller and PPU register side effects. */
    uint8_t status_read = is_ppu_status_address(parked_read);
    (void)bus_read_data(nes, parked_read, *sample);
    sample_interrupt_lines(nes);
    ppu_catch_up(nes);
    *sample = cycle(nes, status_read, 1u); /* DMC dummy read cycle. [HWC.01, HWC.09] */
    (void)bus_read_data(nes, parked_read, *sample);
    sample_interrupt_lines(nes);
    ppu_catch_up(nes);
    if (nes->bus.apu_get_put_phase != 0u) {
        *sample = cycle(nes, status_read, 1u); /* Alignment read on PUT. [HWC.01, HWC.09] */
        (void)bus_read_data(nes, parked_read, *sample);
        sample_interrupt_lines(nes);
        ppu_catch_up(nes);
    }
    (void)cycle(nes, 0u, 1u); /* DMC memory get. [HWC.01, HWC.09] */
    dmc->sample_buffer = nesturbator__map_cpu_read(nes, dmc->address);
    nes->bus.open_bus = dmc->sample_buffer;
    sample_interrupt_lines(nes);
    ppu_catch_up(nes);
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
            nesturbator__irq_update(nes);
        }
    }
    return 1u;
}

static uint8_t bus_read_cycle(struct nesturbator *nes, uint16_t addr)
{
    uint8_t status_read = is_ppu_status_address(addr);
    uint8_t sample = cycle(nes, status_read, 1u);
    if (nes->apu.dmc.dma_pending != 0u && dmc_dma(nes, addr, &sample) != 0u) {
        /* The CPU repeats its parked read once more when RDY is released.
           This is the resumed bus cycle after the DMC memory get, and is
           distinct from the GET cycle that filled the sample buffer. [HWC.01] */
        sample = cycle(nes, status_read, 1u);
    }
    uint8_t value = bus_read_data(nes, addr, sample);
    sample_interrupt_lines(nes);
    ppu_catch_up(nes);
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
        (void)cycle(nes, 0u, 1u);
        nes->bus.open_bus = value;
        nesturbator__ppu_register_write(nes, 0x2004u, value);
        sample_interrupt_lines(nes);
        ppu_catch_up(nes);
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
    (void)cycle(nes, 0u, 1u);
    if (nes->apu.dmc.dma_pending != 0u && nes->apu.dmc.enable_delay == 0u &&
        nes->bus.apu_get_put_phase == nes->apu.dmc.dma_halt_phase) {
        /* RDY cannot halt this write. Its failed attempt retries on the next
           CPU cycle, even when that cycle has the opposite GET/PUT phase.
           NESdev DMA, Cadence and DMC DMA. */
        nes->apu.dmc.dma_halt_phase ^= 1u;
    }
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
    } else if (addr >= 0x4020u) {
        uint8_t *page = nes->map.cpu_w[(addr - 0x4000u) >> 10];
        if (page != NULL) {
            page[addr & 0x3ffu] = value;
            /* D-12: count every write that reaches the battery span, changed
               byte or not. The NULL test comes first because a relational
               compare against a null pointer is undefined in C17; every
               non-NULL page here lies inside the cart.bytes allocation. */
            if (nes->cart.save_size != 0u && page >= nes->cart.save &&
                page < nes->cart.save + nes->cart.save_size)
                nes->save_generation++;
        }
        /* The hook follows the store and is not part of it, so a write that
           lands on a NULL page (NROM's ROM) still reaches the board. With bus
           conflicts the ROM drives the bus too, so the board sees the AND of
           both (NESdev "Bus conflict"). */
        if ((nes->map.watch & NESTURBATOR_WATCH_CPU_WRITE) != 0u) {
            uint8_t v = value;
            if ((nes->map.watch & NESTURBATOR_WATCH_BUS_CONFLICT) != 0u)
                v = (uint8_t)(value & nesturbator__map_cpu_read(nes, addr));
            /* cycle() has already counted this write's cycle, so the stamp is
               the zero-based index of that cycle: the first write after load
               carries 0 plus the cycles run before it (D-14, Phase 6 WR-01). */
            nes->map.ops.cpu_write(nes, addr, v, nes->cpu_cycle - 1u);
        }
    }
    /* The close-edge detector sample shares the CPU write's M2-fall
       timestamp. Apply the register side effect first under the global model
       policy documented above. */
    sample_interrupt_lines(nes);
    ppu_catch_up(nes);
}

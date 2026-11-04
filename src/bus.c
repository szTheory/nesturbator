/* The CPU's bus: time, address decode and the open-bus latch (D-12).

   Each call is one regional CPU cycle. NTSC advances 24 ticks, with M2 rising
   at +9; PAL advances 32 ticks, with M2 rising at +13. In both profiles the
   remaining ticks reach M2 fall, and the data access occurs while M2 is high
   (NES-HARDWARE-CPU-APU, section 2, part 1). A status read catches the PPU up
   at the regional M2 rise before sampling PPUSTATUS.7. At an equal PPU/CPU timestamp,
   the selected global model order is bus side effect, detector sample, then
   PPU transition. This is
   a deterministic model policy; finite-margin circuit cases support the
   phase endpoints, but do not establish physical setup/hold or paired-chip
   propagation at a tie. AccuracyCoin describes the split PPUSTATUS samples
   (TEST_2002FlagTiming, pinned at 6bee919); Visual2A03 measures internal phi2
   separately from widened M2 (Phase 20 oracle research). */
#include "internal.h"

/* Test builds may observe the source-shaped bus at compile time. Production
   translation units erase every call and never carry an observer or callback. */
#ifndef NESTURBATOR_DMA_OBSERVE
#define NESTURBATOR_DMA_OBSERVE(kind, nes, address, data, detail) ((void)0)
#endif

/* The reported 2A07 trace and its authors' SYNC interpretation support this
   bounded model: a PAL DMC transfer may halt on an opcode fetch. The observed
   address sequences and proposed mechanism are described separately in
   https://forums.nesdev.org/viewtopic.php?p=300930. Later held no-op-cycle bus
   behavior remains unresolved. [D-01, D-03, D-04] */
#ifndef NESTURBATOR_DMA_HALT_READ_ELIGIBLE
#define NESTURBATOR_DMA_HALT_READ_ELIGIBLE(nes, sync_read)                                         \
    (nesturbator__is_pal((nes)) == 0 || (sync_read) != 0u)
#endif
static void ppu_catch_up(struct nesturbator *nes)
{
    nesturbator__ppu_run_until(nes, nes->ticks);
}

static void ppu_catch_up_before_close(struct nesturbator *nes)
{
    /* cycle() has advanced a complete regional CPU cycle before this call. */
    nesturbator__ppu_run_until(nes, nes->ticks - 1u);
}

/* Both interrupt inputs are sampled at internal phi2 close: NMI retains a
   recognized edge, while IRQ retains the level. Ordinary instruction polls read this
   captured history at phi2 opening; staged entry samples capture lines but
   do not create another instruction poll [HWC.02, HWC.17, Phase 21 D-08]. */
static void sample_interrupt_lines(struct nesturbator *nes)
{
    uint8_t nmi_line = nesturbator__ppu_nmi_line(nes);
    if (nmi_line != nes->cpu.nmi_prev) {
        if (nmi_line != 0u && nes->cpu.nmi_pending != NESTURBATOR_NMI_RECOGNIZED) {
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
    }
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
                            uint8_t defer_nmi_close_sample, uint8_t dmc_sample_ready)
{
    uint8_t vblank = 0u;
    uint8_t status_value = 0u;
    if (nes->cpu.nmi_pending == NESTURBATOR_NMI_INHIBITED &&
        nes->cpu.entry_origin == NESTURBATOR_ENTRY_NONE) {
        /* First handler T1: qualify before its C1 branch poll. Only this
           unrecognized candidate consults the held input; ordinary captured
           edges keep Phase20's sticky history and close/open timing. */
        nes->cpu.nmi_pending = nesturbator__ppu_nmi_line(nes) != 0u ? NESTURBATOR_NMI_RECOGNIZED
                                                                    : NESTURBATOR_NMI_NONE;
    }
    const uint32_t m2_rise = nesturbator__m2_rise_ticks(nes);
    /* M2 is high 6 ticks longer than its rise offset in both supported
       profiles: 15 after +9 on NTSC, 19 after +13 on PAL [HWC.03, HWC.04].
       Reusing the rise offset avoids selecting the same regional profile twice. */
    const uint32_t ticks_after_m2_rise = m2_rise + 6u;
    nes->ticks += m2_rise;
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
    NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_APU_BEFORE, nes, 0u, nes->apu.dmc.dma_pending, 0u);
    nesturbator__apu_clock(nes, dmc_sample_ready);
    NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_APU_AFTER, nes, 0u, nes->apu.dmc.dma_pending, 0u);
    /* M2 rises at the regional offset, while the circuit oracle places internal phi2
       three ticks later. The CPU polls the request captured by the preceding
       close here; the physical PPU line itself is sampled at phi2 close. */
    /* Only PPUSTATUS observes M2 rise. Other accesses and this history-only
       poll can defer PPU work until the pre-close catch-up. APU and mapper
       IRQ sources combine before the close-edge sample. */
    /* IRQ must use the preceding close sample too: a tied PPU A12 transition
       after that detector sample cannot feed this poll. [HWC.02, HWC.17] */
    if (nes->cpu.entry_origin == NESTURBATOR_ENTRY_NONE) {
        nes->cpu.poll_latch = nes->cpu.irq_captured;
        nes->cpu.nmi_polled = (uint8_t)(nes->cpu.nmi_pending == NESTURBATOR_NMI_RECOGNIZED);
    }
    /* No timed helper runs between the history-only opening poll and close.
       Advance the remaining part of this CPU cycle together rather than materializing
       an otherwise unobservable intermediate timestamp on every cycle. */
    nes->ticks += ticks_after_m2_rise;
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
        /* Status drives the internal CPU bus only. Its floating D5 retains
           that bus's history, not a DMC byte driven externally. HWC.08/19. */
        nes->bus.cpu_data =
            (uint8_t)((nesturbator__apu_status_read(nes) & 0xdfu) | (nes->bus.cpu_data & 0x20u));
        return nes->bus.cpu_data;
    } else if (addr < 0x2000u) {
        nes->bus.open_bus = nes->bus.ram[addr & 0x7FFu];
    } else if (addr < 0x4000u) {
        uint16_t reg = (uint16_t)(0x2000u | (addr & 7u));
        nes->bus.open_bus =
            reg == 0x2002u ? status_value : nesturbator__ppu_register_read(nes, reg);
    }
    /* Only selected $4015 disconnects the external bus. After that select
       ends, undriven registers sample the external held value too. The pinned
       bus-conflict answer key excludes $16/$17, not $15. [HWC.08/19] */
    nes->bus.cpu_data = nes->bus.open_bus;
    return nes->bus.cpu_data;
}

/* Complete one actual GET after its clock, shared by standalone and OAM
   overlap paths. No extra CPU cycle is implied by this data-side helper. */
static void dmc_sample_get(struct nesturbator *nes, uint16_t parked_read)
{
    struct nesturbator__dmc *dmc = &nes->apu.dmc;
    dmc->sample_buffer = nesturbator__map_cpu_read(nes, dmc->address);
    NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_SAMPLE_GET, nes, dmc->address, dmc->sample_buffer, 0u);
    nes->bus.open_bus = dmc->sample_buffer;
    uint16_t internal = (uint16_t)((parked_read & 0xFFE0u) | (dmc->address & 0x001Fu));
    if (internal >= 0x4015u && internal <= 0x4017u) {
        /* Register select takes core A15:5 and external A4:0. The pinned
           AccuracyCoin test qualifies CPU-visible effects, not DMC shift
           register input; retain the existing external-byte reader model. */
        (void)bus_read_data(nes, internal, 0u);
        NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_INTERNAL_READ, nes, internal, nes->bus.cpu_data,
                                parked_read);
    } else {
        nes->bus.cpu_data = dmc->sample_buffer;
    }
    sample_interrupt_lines(nes);
    ppu_catch_up(nes);
    dmc->buffer_empty = 0u;
    dmc->dma_pending = 0u;
    dmc->address = dmc->address == 0xffffu ? 0x8000u : (uint16_t)(dmc->address + 1u);
    if (dmc->remaining != 0u)
        dmc->remaining--;
    else
        return; /* An armed GET after disable fills the buffer, not a new sample. */
    if (dmc->remaining == 0u) {
        if ((dmc->reg[0] & 0x40u) != 0u) {
            dmc->address = (uint16_t)(0xc000u | ((uint32_t)dmc->reg[2] << 6));
            dmc->remaining = (uint16_t)(((uint32_t)dmc->reg[3] << 4) | 1u);
        } else {
            nesturbator__dmc_stop_window(nes);
            if ((dmc->reg[0] & 0x80u) != 0u) {
                dmc->irq = 1u;
                nesturbator__irq_update(nes);
            }
        }
    }
}

static uint8_t dmc_dma(struct nesturbator *nes, uint16_t parked_read, uint8_t *sample,
                       uint8_t sync_read)
{
    struct nesturbator__dmc *dmc = &nes->apu.dmc;
    (void)sync_read;
    NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_HALT_ATTEMPT, nes, parked_read, 0u, sync_read);
    if (dmc->dma_pending == 0u || dmc->enable_delay != 0u ||
        nes->bus.apu_get_put_phase != dmc->dma_halt_phase || nes->cart.prg == NULL)
        return 0u;

    NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_HALT_ACCEPTED, nes, parked_read, 0u, sync_read);
    /* RDY halts the CPU only on reads. The first halted cycle repeats the
       parked CPU access, including controller and PPU register side effects. */
    uint8_t status_read = is_ppu_status_address(parked_read);
    (void)bus_read_data(nes, parked_read, *sample);
    NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_PARKED_HALT, nes, parked_read, nes->bus.open_bus, 0u);
    sample_interrupt_lines(nes);
    ppu_catch_up(nes);
    *sample = cycle(nes, status_read, 1u, 1u); /* DMC dummy read cycle. [HWC.01, HWC.09] */
    (void)bus_read_data(nes, parked_read, *sample);
    NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_PARKED_DUMMY, nes, parked_read, nes->bus.open_bus, 0u);
    sample_interrupt_lines(nes);
    ppu_catch_up(nes);
    if (nes->bus.apu_get_put_phase != 0u) {
        *sample = cycle(nes, status_read, 1u, 1u); /* Alignment read on PUT. [HWC.01, HWC.09] */
        (void)bus_read_data(nes, parked_read, *sample);
        NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_PARKED_ALIGN, nes, parked_read, nes->bus.open_bus,
                                0u);
        sample_interrupt_lines(nes);
        ppu_catch_up(nes);
    }
    (void)cycle(nes, 0u, 1u, 1u); /* DMC memory get. [HWC.01, HWC.09] */
    dmc_sample_get(nes, parked_read);
    return 1u;
}

static uint8_t bus_read_cycle(struct nesturbator *nes, uint16_t addr, uint8_t dmc_sample_ready,
                              uint8_t sync_read)
{
    uint8_t status_read = is_ppu_status_address(addr);
#ifdef NESTURBATOR_DMA_READ_KIND_OBSERVE
    uint8_t dmc_was_pending = nes->apu.dmc.dma_pending;
#endif
    /* Capture before this cycle's APU clock expires the countdown. A write
       or an already halted OAM cycle simply expires it without a retry. */
    uint8_t aborted = (uint8_t)(nes->apu.dmc.abort_delay == 1u);
    /* Give the ordinary and split-status timings constant arguments. This
       keeps PPUSTATUS-only temporaries out of every other read's hot path. */
    uint8_t sample;
    if (status_read != 0u)
        sample = cycle(nes, 1u, 1u, dmc_sample_ready);
    else
        sample = cycle(nes, 0u, 1u, dmc_sample_ready);
#ifdef NESTURBATOR_DMA_READ_KIND_OBSERVE
    NESTURBATOR_DMA_READ_KIND_OBSERVE(
        nes, addr, sync_read, (uint8_t)(dmc_was_pending != 0u || nes->apu.dmc.dma_pending != 0u));
#endif
    uint8_t value;
    if (aborted == 0u && nes->apu.dmc.dma_pending == 0u) {
        value = bus_read_data(nes, addr, sample);
        NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_CPU_READ_COMPLETE, nes, addr, value, 0u);
        sample_interrupt_lines(nes);
        ppu_catch_up(nes);
        return value;
    }
    uint8_t halted = 0u;
    if (aborted != 0u) {
        NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_HALT_ACCEPTED, nes, addr, 0u, 1u);
        (void)bus_read_data(nes, addr, sample);
        NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_PARKED_HALT, nes, addr, nes->bus.cpu_data, 1u);
        sample_interrupt_lines(nes);
        ppu_catch_up(nes);
        sample = cycle(nes, status_read, 1u, 1u);
        halted = 1u;
    }
    if (nes->apu.dmc.dma_pending != 0u && NESTURBATOR_DMA_HALT_READ_ELIGIBLE(nes, sync_read) != 0 &&
        dmc_dma(nes, addr, &sample, sync_read) != 0u) {
        /* The CPU repeats its parked read once more when RDY is released.
           This is the resumed bus cycle after the DMC memory get, and is
           distinct from the GET cycle that filled the sample buffer. The
           fresh byte is not yet output-visible on this immediate cycle
           (inferred early-G Loop3 ordering, HWC.19). This local argument
           cannot cross a public save boundary. [HWC.01] */
        sample = cycle(nes, status_read, 1u, 0u);
        halted = 1u;
    }
    value = bus_read_data(nes, addr, sample);
    if (halted != 0u) {
        NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_RESUMED_READ, nes, addr, value, 0u);
    }
    NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_CPU_READ_COMPLETE, nes, addr, value, 0u);
    sample_interrupt_lines(nes);
    ppu_catch_up(nes);
    return value;
}

static uint8_t oam_dma(struct nesturbator *nes, uint8_t page, uint16_t halted_read)
{
    /* The CPU is already halted for the whole OAM transfer. DMC halt/dummy/
       alignment may overlap OAM accesses; only its memory GET steals an OAM
       GET. A following idle PUT aligns OAM before retrying that source byte.
       DMA rev 23450, DMC DMA during OAM DMA [HWC.01]. These synchronous local
       stages cannot straddle a public save boundary and add no saved state. */
    uint16_t offset = 0u;
    uint8_t byte = 0u, have_byte = 0u, oam_halt = 1u;
    uint8_t dmc_stage = 0u; /* 0: idle, 1: dummy next, 2: GET when aligned */
    uint8_t dmc_sample_ready = 1u;
    do {
        uint8_t phase = (uint8_t)(nes->bus.apu_get_put_phase ^ 1u);
        uint8_t dmc_get = (uint8_t)(dmc_stage == 2u && phase != 0u);
        uint8_t source = (uint8_t)(oam_halt == 0u && offset < 256u && phase != 0u && dmc_get == 0u);
        uint8_t destination =
            (uint8_t)(oam_halt == 0u && offset < 256u && phase == 0u && have_byte != 0u);
        uint16_t address = source != 0u ? (uint16_t)(((uint16_t)page << 8) | offset) : halted_read;
        if (source != 0u)
            NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_OAM_SOURCE, nes, address, 0u, offset);
        uint8_t status_read =
            (uint8_t)(dmc_get == 0u && destination == 0u && is_ppu_status_address(address) != 0u);
        uint8_t status = cycle(nes, status_read, 1u, dmc_sample_ready);
        dmc_sample_ready = (uint8_t)(dmc_get == 0u);
        if (dmc_get != 0u) {
            dmc_sample_get(nes, halted_read);
        } else {
            if (source != 0u) {
                byte = bus_read_data(nes, address, status);
                have_byte = 1u;
                NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_OAM_SOURCE_COMPLETE, nes, address, byte,
                                        offset);
            } else if (destination != 0u) {
                nes->bus.open_bus = byte;
                nes->bus.cpu_data = byte;
                NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_OAM_DEST, nes, 0x2004u, byte, offset);
                nesturbator__ppu_register_write(nes, 0x2004u, byte);
                offset++;
                have_byte = 0u;
            } else {
                (void)bus_read_data(nes, halted_read, status);
            }
            sample_interrupt_lines(nes);
            ppu_catch_up(nes);
        }
        if (dmc_get != 0u) {
            dmc_stage = 0u;
        } else if (dmc_stage == 1u) {
            dmc_stage = 2u;
        } else if (dmc_stage == 0u && nes->apu.dmc.dma_pending != 0u &&
                   nes->apu.dmc.enable_delay == 0u && phase == nes->apu.dmc.dma_halt_phase &&
                   nes->cart.prg != NULL) {
            NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_HALT_ACCEPTED, nes, halted_read, 0u, offset);
            dmc_stage = 1u;
        }
        oam_halt = 0u;
    } while (offset < 256u || dmc_stage != 0u);
    return dmc_sample_ready;
}

static uint8_t bus_read_with_kind(struct nesturbator *nes, uint16_t addr, uint8_t sync_read)
{
    uint8_t dmc_sample_ready = 1u;
    if (nes->bus.oam_dma_pending != 0u) {
        uint8_t page = nes->bus.oam_dma_page;
        nes->bus.oam_dma_pending = 0u;
        dmc_sample_ready = oam_dma(nes, page, addr);
    }
    return bus_read_cycle(nes, addr, dmc_sample_ready, sync_read);
}

uint8_t nesturbator__bus_read(struct nesturbator *nes, uint16_t addr)
{
    return bus_read_with_kind(nes, addr, 0u);
}

uint8_t nesturbator__bus_read_sync(struct nesturbator *nes, uint16_t addr)
{
    return bus_read_with_kind(nes, addr, 1u);
}

void nesturbator__bus_write(struct nesturbator *nes, uint16_t addr, uint8_t value)
{
    (void)cycle(nes, 0u, 1u, 1u);
    if (nes->apu.dmc.dma_pending != 0u && nes->apu.dmc.enable_delay == 0u &&
        nes->bus.apu_get_put_phase == nes->apu.dmc.dma_halt_phase) {
        /* RDY cannot halt this write. Its failed attempt retries on the next
           CPU cycle, even when that cycle has the opposite GET/PUT phase.
           NESdev DMA, Cadence and DMC DMA. */
        nes->apu.dmc.dma_halt_phase ^= 1u;
        NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_WRITE_RETRY, nes, addr, value, 0u);
    }
    nes->bus.open_bus = value;
    nes->bus.cpu_data = value;
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
    NESTURBATOR_DMA_OBSERVE(NESTURBATOR_DMA_CPU_WRITE_COMPLETE, nes, addr, value, 0u);
}

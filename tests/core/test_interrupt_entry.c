/* INT-02 entry evidence: source-shaped NROM guests use cpu_step and the
   production PPU/APU/bus. The only bus substitution is this test executable's
   compile-time access wrapper; each wrapper call delegates to src/bus.c. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../interrupt_fixture.h"

#define ENTRY_LOG_CAPACITY 32u
#define ENTRY_NMI_MARKER 0x32u
#define ENTRY_IRQ_MARKER 0x33u
#define ENTRY_FLAG_B 0x10u
#define ENTRY_FLAG_U 0x20u
#define ENTRY_FLAG_I 0x04u
#define ENTRY_NMI_EVENTS 4u

uint8_t nesturbator__bus_read_production(struct nesturbator *nes, uint16_t addr);
void nesturbator__bus_write_production(struct nesturbator *nes, uint16_t addr, uint8_t value);

struct entry_bus_access {
    uint16_t addr;
    uint8_t value;
    uint8_t kind;
    uint64_t ticks_before;
    uint64_t ticks_after;
};

struct scheduled_nmi_event {
    uint32_t assert_access;
    uint32_t release_access;
};

struct entry_schedule {
    struct scheduled_nmi_event nmi[ENTRY_NMI_EVENTS];
    uint8_t nmi_count;
    uint32_t raise_frame_irq_access;
};

static struct entry_bus_access entry_log[ENTRY_LOG_CAPACITY];
static struct entry_schedule entry_schedule;
static uint32_t entry_access_count;
static uint8_t entry_log_overflow;

static void schedule_nmi(uint32_t assert_access, uint32_t release_access)
{
    CHECK(entry_schedule.nmi_count < ENTRY_NMI_EVENTS);
    if (entry_schedule.nmi_count >= ENTRY_NMI_EVENTS)
        return;
    entry_schedule.nmi[entry_schedule.nmi_count++] =
        (struct scheduled_nmi_event){assert_access, release_access};
}

static void apply_entry_schedule(struct nesturbator *nes, uint32_t access)
{
    for (uint8_t i = 0u; i < entry_schedule.nmi_count; i++) {
        if (entry_schedule.nmi[i].assert_access == access) {
            /* A constructed /NMI pin transition, sampled by the production
               bus. It names a core bus action, never a circuit Tn state. */
            nes->ppu.status |= 0x80u;
            nes->ppu.control |= 0x80u;
        }
    }
    if (entry_schedule.raise_frame_irq_access == access) {
        /* Raise the production APU frame-counter source; irq_update remains
           the one writer that combines it into the CPU IRQ line. */
        nes->apu.frame_irq = 1u;
        nesturbator__irq_update(nes);
    }
}

static void release_entry_schedule(struct nesturbator *nes, uint32_t access)
{
    for (uint8_t i = 0u; i < entry_schedule.nmi_count; i++) {
        if (entry_schedule.nmi[i].release_access == access &&
            entry_schedule.nmi[i].release_access != 0u)
            nes->ppu.control &= 0x7fu;
    }
}

static void record_entry_access(struct nesturbator *nes, uint16_t addr, uint8_t value, uint8_t kind,
                                uint64_t before)
{
    if (entry_access_count >= ENTRY_LOG_CAPACITY) {
        entry_log_overflow = 1u;
        return;
    }
    entry_log[entry_access_count++] =
        (struct entry_bus_access){addr, value, kind, before, nes->ticks};
}

uint8_t nesturbator__bus_read(struct nesturbator *nes, uint16_t addr)
{
    uint32_t access = entry_access_count + 1u;
    uint64_t before = nes->ticks;
    apply_entry_schedule(nes, access);
    uint8_t value = nesturbator__bus_read_production(nes, addr);
    record_entry_access(nes, addr, value, 0u, before);
    release_entry_schedule(nes, access);
    return value;
}

void nesturbator__bus_write(struct nesturbator *nes, uint16_t addr, uint8_t value)
{
    uint32_t access = entry_access_count + 1u;
    uint64_t before = nes->ticks;
    apply_entry_schedule(nes, access);
    nesturbator__bus_write_production(nes, addr, value);
    record_entry_access(nes, addr, value, 1u, before);
    release_entry_schedule(nes, access);
}

static const uint8_t entry_program[] = {
    0xeau,                            /* NOP: accepts a real APU IRQ or polls a natural PPU NMI. */
    0x00u, 0x5au,                     /* BRK; signature/padding byte. */
    0xeau, 0xeau, 0x4cu, 0x03u, 0x80u /* marker loop */
};

static const uint8_t nmi_handler[] = {
    0xe6u, ENTRY_NMI_MARKER, /* INC $32 */
    0x40u                    /* RTI */
};

static const uint8_t irq_handler[] = {
    0xe6u, ENTRY_IRQ_MARKER, /* INC $33 */
    0x40u                    /* RTI */
};

struct entry_observation {
    uint16_t pc;
    uint8_t s;
    uint8_t pch;
    uint8_t pcl;
    uint8_t pushed_p;
    uint16_t low_vector_address;
    uint16_t high_vector_address;
    uint8_t low_vector_value;
    uint8_t high_vector_value;
};

static void trace_reset(void)
{
    memset(entry_log, 0, sizeof entry_log);
    memset(&entry_schedule, 0, sizeof entry_schedule);
    entry_access_count = 0u;
    entry_log_overflow = 0u;
}

static struct nesturbator *build_entry_guest(uint8_t status)
{
    struct nesturbator *nes =
        interrupt_fixture_build(entry_program, sizeof entry_program, nmi_handler,
                                sizeof nmi_handler, irq_handler, sizeof irq_handler);
    CHECK(nes != NULL);
    if (nes != NULL)
        interrupt_fixture_start(nes, 0x8000u, status, 0u);
    return nes;
}

static struct nesturbator *prepare_frame_irq_entry(void)
{
    struct nesturbator *nes = build_entry_guest(0x20u);
    if (nes == NULL)
        return NULL;

    /* AccuracyCoin IFlagLatency source schedule: the frame counter asserts
       at 29828/29829, then its captured level is accepted at the next NOP. */
    nes->apu.frame_cycle = 29827u;
    nesturbator__cpu_step(nes);
    CHECK_EQ_U64(nes->apu.frame_irq, 1u);
    CHECK_EQ_U64(nes->cpu.irq_line, 1u);
    CHECK_EQ_U64(nes->cpu.irq_captured, 1u);
    CHECK_EQ_U64(nes->cpu.irq_accepted, 1u);
    CHECK_EQ_HEX(nes->cpu.pc, 0x8001u);
    trace_reset();
    return nes;
}

static struct nesturbator *prepare_natural_nmi_entry(void)
{
    struct nesturbator *nes = build_entry_guest(0x20u);
    if (nes == NULL)
        return NULL;

    /* The PPU, rather than a seeded CPU request, raises /NMI at VBlank dot 1. */
    nes->ticks = 0u;
    nes->cpu_cycle = 0u;
    nes->ppu.ppu_ticks = 0u;
    nes->ppu.scanline = 241u;
    nes->ppu.dot = 0u;
    nes->ppu.status = 0u;
    nes->ppu.control = 0x80u;
    nesturbator__cpu_step(nes);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 1u);
    CHECK_EQ_U64(nes->cpu.nmi_polled, 1u);
    CHECK_EQ_HEX(nes->cpu.pc, 0x8001u);
    trace_reset();
    return nes;
}

static struct entry_observation observe_entry(struct nesturbator *nes, uint16_t vector_low)
{
    struct entry_observation out = {0};
    out.pc = nes->cpu.pc;
    out.s = nes->cpu.s;
    out.pch = nes->bus.ram[0x01fdu];
    out.pcl = nes->bus.ram[0x01fcu];
    out.pushed_p = nes->bus.ram[0x01fbu];
    out.low_vector_address = vector_low;
    out.high_vector_address = (uint16_t)(vector_low + 1u);
    out.low_vector_value = nesturbator__map_cpu_read(nes, vector_low);
    out.high_vector_value = nesturbator__map_cpu_read(nes, (uint16_t)(vector_low + 1u));
    return out;
}

static unsigned check_vector_pc(const struct entry_observation *observed, uint16_t expected,
                                const char *case_name)
{
    if (observed->pc == expected)
        return 0u;
    fprintf(stderr, "%s:%d: ENTRY_ASSERTION %s vector PC expected=0x%04x actual=0x%04x\n", __FILE__,
            __LINE__, case_name, expected, observed->pc);
    return 1u;
}

static void check_entry_bus_shape(uint16_t initial_pc, uint8_t expected_pushed_p,
                                  uint16_t expected_vector)
{
    CHECK_EQ_U64(entry_access_count, 7u);
    CHECK_EQ_U64(entry_log_overflow, 0u);
    if (entry_access_count != 7u)
        return;

    CHECK_EQ_HEX(entry_log[0].addr, initial_pc);
    CHECK_EQ_HEX(entry_log[1].addr, initial_pc);
    CHECK_EQ_HEX(entry_log[2].addr, 0x01fdu);
    CHECK_EQ_HEX(entry_log[3].addr, 0x01fcu);
    CHECK_EQ_HEX(entry_log[4].addr, 0x01fbu);
    CHECK_EQ_HEX(entry_log[4].value, expected_pushed_p);
    CHECK_EQ_HEX(entry_log[5].addr, expected_vector);
    CHECK_EQ_HEX(entry_log[6].addr, (uint16_t)(expected_vector + 1u));
    for (uint32_t i = 0u; i < 7u; i++)
        CHECK_EQ_U64(entry_log[i].ticks_after - entry_log[i].ticks_before, 24u);
}

static void test_production_cpu_step_entry_origins(void)
{
    struct nesturbator *brk = build_entry_guest(0x20u);
    CHECK(brk != NULL);
    if (brk != NULL) {
        trace_reset();
        brk->cpu.pc = 0x8001u;
        nesturbator__cpu_step(brk);
        struct entry_observation observed = observe_entry(brk, 0xfffeu);
        CHECK_EQ_U64(entry_access_count, 7u);
        CHECK_EQ_U64(entry_log_overflow, 0u);
        if (entry_access_count == 7u) {
            CHECK_EQ_HEX(entry_log[0].addr, 0x8001u);
            CHECK_EQ_HEX(entry_log[1].addr, 0x8002u);
            CHECK_EQ_HEX(entry_log[2].addr, 0x01fdu);
            CHECK_EQ_HEX(entry_log[2].value, 0x80u);
            CHECK_EQ_HEX(entry_log[3].addr, 0x01fcu);
            CHECK_EQ_HEX(entry_log[3].value, 0x03u);
            CHECK_EQ_HEX(entry_log[4].addr, 0x01fbu);
            CHECK_EQ_HEX(entry_log[4].value, 0x30u);
            CHECK_EQ_HEX(entry_log[5].addr, 0xfffeu);
            CHECK_EQ_HEX(entry_log[6].addr, 0xffffu);
            for (uint32_t i = 0u; i < 7u; i++)
                CHECK_EQ_U64(entry_log[i].ticks_after - entry_log[i].ticks_before, 24u);
        }
        CHECK_EQ_HEX(observed.pc, INTERRUPT_FIXTURE_IRQ_VECTOR);
        CHECK_EQ_HEX(observed.pch, 0x80u);
        CHECK_EQ_HEX(observed.pcl, 0x03u);
        CHECK((observed.pushed_p & ENTRY_FLAG_B) != 0u);
        CHECK((observed.pushed_p & ENTRY_FLAG_U) != 0u);
        nesturbator_destroy((nesturbator *)brk);
    }

    struct nesturbator *irq = prepare_frame_irq_entry();
    CHECK(irq != NULL);
    if (irq != NULL) {
        nesturbator__cpu_step(irq);
        struct entry_observation observed = observe_entry(irq, 0xfffeu);
        check_entry_bus_shape(0x8001u, 0x20u, 0xfffeu);
        CHECK_EQ_HEX(observed.pc, INTERRUPT_FIXTURE_IRQ_VECTOR);
        CHECK_EQ_HEX(observed.pch, 0x80u);
        CHECK_EQ_HEX(observed.pcl, 0x01u);
        CHECK((observed.pushed_p & ENTRY_FLAG_B) == 0u);
        CHECK((observed.pushed_p & ENTRY_FLAG_U) != 0u);
        nesturbator_destroy((nesturbator *)irq);
    }

    struct nesturbator *nmi = prepare_natural_nmi_entry();
    CHECK(nmi != NULL);
    if (nmi != NULL) {
        nesturbator__cpu_step(nmi);
        struct entry_observation observed = observe_entry(nmi, 0xfffau);
        check_entry_bus_shape(0x8001u, 0x20u, 0xfffau);
        CHECK_EQ_HEX(observed.pc, INTERRUPT_FIXTURE_NMI_VECTOR);
        CHECK_EQ_HEX(observed.pch, 0x80u);
        CHECK_EQ_HEX(observed.pcl, 0x01u);
        CHECK((observed.pushed_p & ENTRY_FLAG_B) == 0u);
        CHECK((observed.pushed_p & ENTRY_FLAG_U) != 0u);
        nesturbator_destroy((nesturbator *)nmi);
    }
    printf("CASE guard production-bus BRK/IRQ/NMI cpu_step origins; APU and PPU sources\n");
}

static void test_dmc_reload_supplies_jsr_target_opcode(void)
{
    static const uint8_t program[] = {0x20u, 0x13u, 0x40u, 0x90u};
    static const uint8_t handler[] = {0x40u};
    struct nesturbator *nes = interrupt_fixture_build(program, sizeof program, handler,
                                                      sizeof handler, handler, sizeof handler);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    interrupt_fixture_start(nes, 0x8000u, 0x20u, 0u);
    /* AccuracyCoin 6bee919, TEST_IFlagLatency_Test_C, test E: JSR ends on
       GET and the next PUT requests the DMC reload on the target opcode.
       The bus and APU phase fields retain the equal relationship reached
       by cold execution. The separate write-retry test pins how earlier
       halted writes affect this alignment, without the guest sync routine.
       [HWC.01, HWC.10, HWC.19] */
    nes->bus.apu_get_put_phase = 1u;
    nes->apu.pulse_clock_phase = 1u;
    nes->apu.dmc.reg[0] = 0x0fu;
    nes->apu.dmc.address = 0xc003u;
    nes->apu.dmc.remaining = 1u;
    nes->apu.dmc.sample_buffer = 0u;
    nes->apu.dmc.buffer_empty = 0u;
    nes->apu.dmc.bits = 0u;
    nes->apu.dmc.counter = 4u;
    nes->apu.dmc.timer = 27u;
    nes->apu.dmc.dma_pending = 0u;
    nes->apu.dmc.dma_load_waiting = 0u;
    nes->apu.dmc.enable_delay = 0u;

    trace_reset();
    uint64_t before_jsr = nes->ticks;
    nesturbator__cpu_step(nes);
    CHECK_EQ_HEX(nes->cpu.pc, 0x4013u);
    CHECK_EQ_U64(entry_access_count, 6u);
    if (entry_access_count == 6u) {
        CHECK_EQ_HEX(entry_log[5].addr, 0x8002u);
        CHECK_EQ_U64(entry_log[5].ticks_after - entry_log[5].ticks_before, 24u);
    }
    CHECK_EQ_U64(nes->ticks - before_jsr, 6u * 24u);
    CHECK_EQ_U64(nes->apu.dmc.dma_pending, 0u);
    CHECK_EQ_U64(nes->apu.dmc.remaining, 1u);
    CHECK_EQ_HEX(nes->apu.dmc.sample_buffer, 0x00u);

    trace_reset();
    nesturbator__cpu_step(nes);
    CHECK(entry_access_count != 0u);
    if (entry_access_count != 0u) {
        CHECK_EQ_HEX(entry_log[0].addr, 0x4013u);
        CHECK_EQ_HEX(entry_log[0].value, 0x90u);
        CHECK_EQ_U64(entry_log[0].ticks_after - entry_log[0].ticks_before, 5u * 24u);
    }
    CHECK_EQ_HEX(nes->cpu.pc, 0x3fa5u);
    printf("CASE production DMC reload parks at $4013 and supplies BCC $90 after JSR $4013\n");
    nesturbator_destroy((nesturbator *)nes);
}

static void test_dmc_resumed_mapped_read_updates_open_bus(void)
{
    static const uint8_t program[] = {0xeau, 0x5au};
    static const uint8_t handler[] = {0x40u};
    struct nesturbator *nes = interrupt_fixture_build(program, sizeof program, handler,
                                                      sizeof handler, handler, sizeof handler);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    interrupt_fixture_start(nes, 0x8000u, 0x20u, 0u);
    /* Independent mapped-read control: the parked CPU address supplies $EA,
       while DMC GET supplies a deliberately different $5A. The resumed CPU
       read must drive $EA onto open bus after GET drove $5A. The circuit-level
       stall report measures that resumed read, and HWC.01 documents the
       repeated read when DMA ends. */
    nes->bus.apu_get_put_phase = 0u;
    nes->bus.open_bus = 0x11u;
    nes->apu.dmc.address = 0x8001u;
    nes->apu.dmc.remaining = 1u;
    nes->apu.dmc.sample_buffer = 0u;
    nes->apu.dmc.buffer_empty = 1u;
    nes->apu.dmc.bits = 8u;
    nes->apu.dmc.counter = 100u;
    nes->apu.dmc.dma_pending = 1u;
    nes->apu.dmc.dma_halt_phase = 1u;
    nes->apu.dmc.dma_load_waiting = 0u;
    nes->apu.dmc.enable_delay = 0u;

    uint8_t value = nesturbator__bus_read(nes, 0x8000u);
    CHECK_EQ_HEX(nes->apu.dmc.sample_buffer, 0x5au);
    CHECK_EQ_HEX(value, 0xeau);
    CHECK_EQ_HEX(nes->bus.open_bus, 0xeau);
    CHECK_EQ_U64(nes->apu.dmc.remaining, 0u);
    CHECK_EQ_U64(nes->apu.dmc.dma_pending, 0u);
    printf("CASE DMC mapped-resume control GET=$5A resumed-ROM=$EA open-bus=$%02X\n",
           nes->bus.open_bus);
    nesturbator_destroy((nesturbator *)nes);
}

static uint8_t candidate_selects_nmi(uint8_t decision_action, uint8_t arrival_action)
{
    return (uint8_t)(arrival_action <= decision_action);
}

static void run_finite_margin_case(const struct interrupt_boundary_case *boundary)
{
    struct nesturbator *nes = prepare_frame_irq_entry();
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    schedule_nmi(boundary->bus_action, 0u);
    nesturbator__cpu_step(nes);

    uint16_t actual_pc = nes->cpu.pc;
    uint16_t mapped_expected = boundary->mapped_t5_selects_nmi != 0u ? INTERRUPT_FIXTURE_NMI_VECTOR
                                                                     : INTERRUPT_FIXTURE_IRQ_VECTOR;
    struct entry_observation observed =
        observe_entry(nes, actual_pc == INTERRUPT_FIXTURE_NMI_VECTOR ? 0xfffau : 0xfffeu);
    check_entry_bus_shape(0x8001u, 0x20u,
                          actual_pc == INTERRUPT_FIXTURE_NMI_VECTOR ? 0xfffau : 0xfffeu);
    CHECK(actual_pc == INTERRUPT_FIXTURE_NMI_VECTOR || actual_pc == INTERRUPT_FIXTURE_IRQ_VECTOR);
    CHECK_EQ_HEX(observed.pch, 0x80u);
    CHECK_EQ_HEX(observed.pcl, 0x01u);
    CHECK_EQ_HEX(observed.pushed_p, 0x20u);

    CHECK_EQ_U64(check_vector_pc(&observed, mapped_expected, boundary->name), 0u);

    printf("EVIDENCE source=%s margin=%s injected-before=C%u ticks=%llu..%llu "
           "mapped-T5=%s external-C6=%s observed=0x%04x tie=unmeasured\n",
           boundary->source_pin, boundary->source_phase, boundary->bus_action,
           (unsigned long long)entry_log[boundary->bus_action - 1u].ticks_before,
           (unsigned long long)entry_log[boundary->bus_action - 1u].ticks_after,
           boundary->mapped_t5_selects_nmi != 0u ? "NMI" : "IRQ",
           boundary->external_c6_selects_nmi != 0u ? "NMI" : "IRQ", actual_pc);
    nesturbator_destroy((nesturbator *)nes);
}

static void test_source_qualified_finite_margin_mapping(void)
{
    /* The C4-arrival case has a full action of margin before end T4. The
       C6-arrival case is after the inferred T5 decision but at the delayed
       external low-vector action; it separates T5 from external-C6 models. */
    CHECK_EQ_U64(candidate_selects_nmi(INTERRUPT_ACTION_BEFORE_C1, INTERRUPT_ACTION_BEFORE_C4), 0u);
    CHECK_EQ_U64(candidate_selects_nmi(INTERRUPT_ACTION_BEFORE_C5, INTERRUPT_ACTION_BEFORE_C4), 1u);
    CHECK_EQ_U64(candidate_selects_nmi(INTERRUPT_ACTION_BEFORE_C6, INTERRUPT_ACTION_BEFORE_C4), 1u);
    CHECK_EQ_U64(candidate_selects_nmi(INTERRUPT_ACTION_BEFORE_C1, INTERRUPT_ACTION_BEFORE_C6), 0u);
    CHECK_EQ_U64(candidate_selects_nmi(INTERRUPT_ACTION_BEFORE_C5, INTERRUPT_ACTION_BEFORE_C6), 0u);
    CHECK_EQ_U64(candidate_selects_nmi(INTERRUPT_ACTION_BEFORE_C6, INTERRUPT_ACTION_BEFORE_C6), 1u);
    CHECK(candidate_selects_nmi(INTERRUPT_ACTION_BEFORE_C5, INTERRUPT_ACTION_BEFORE_C4) !=
          candidate_selects_nmi(INTERRUPT_ACTION_BEFORE_C5, INTERRUPT_ACTION_BEFORE_C6));

    CHECK_EQ_U64(sizeof interrupt_boundary_cases / sizeof interrupt_boundary_cases[0], 2u);
    CHECK(strcmp(interrupt_boundary_cases[0].source_pin,
                 "Visual6502wiki/6502_Interrupt_Hijacking revision 19268") == 0);
    CHECK(strcmp(interrupt_boundary_cases[1].source_pin,
                 "Visual6502wiki/6502_Interrupt_Hijacking revision 19268") == 0);
    run_finite_margin_case(&interrupt_boundary_cases[0]);
    run_finite_margin_case(&interrupt_boundary_cases[1]);
}

static const uint8_t overlap_program[] = {
    0xeau,        /* NOP: first NMI arrival can precede setup. */
    0xa9u, 0x00u, /* LDA #0: source setup clears N and sets Z. */
    0x18u,        /* CLC */
    0xb8u,        /* CLV */
    0x58u,        /* CLI */
    0xeau,        /* NOP */
    0x78u,        /* SEI: real APU source can be accepted under old I. */
    0x00u, 0x5au, /* BRK; skipped signature byte. */
    0xeau, 0xeau, 0x40u};

static const uint8_t overlap_nmi_handler[] = {0xeau, 0x40u};
static const uint8_t overlap_lower_handler[] = {0xeau, 0xeau, 0x40u};

static uint8_t current_entry_p(const struct nesturbator *nes)
{
    return nes->bus.ram[0x0100u | (uint8_t)(nes->cpu.s + 1u)];
}

static void guest_step(struct nesturbator *nes)
{
    trace_reset();
    nesturbator__cpu_step(nes);
}

/* Recreates the overlap source's meaningful guest sequence with owned NROM:
   flag setup, BRK or SEI-accepted APU IRQ, NMI table handler, and lower
   handler. It intentionally sweeps finite bus-action arrivals instead of
   reproducing AccuracyCoin's long PPU clockslide. Both outputs are captured
   from actual stack frames reached through cpu_step and the production bus. */
static void run_overlap_sample(uint8_t irq_origin, uint8_t sample, uint8_t *nmi_p, uint8_t *lower_p)
{
    const uint8_t pre = sample < 10u;
    const uint8_t first_late = irq_origin != 0u ? 36u : 25u;
    const uint8_t late = sample >= first_late;
    const uint8_t middle = (uint8_t)(pre == 0u && late == 0u);
    struct nesturbator *nes = interrupt_fixture_build(
        overlap_program, sizeof overlap_program, overlap_nmi_handler, sizeof overlap_nmi_handler,
        overlap_lower_handler, sizeof overlap_lower_handler);
    CHECK(nes != NULL);
    *nmi_p = 0u;
    *lower_p = 0u;
    if (nes == NULL)
        return;

    interrupt_fixture_start(nes, 0x8000u, 0xa4u, 0u);
    if (pre != 0u) {
        /* Short PPU-line pulse before the first guest NOP. The production
           bus samples its edge, then the actual NMI handler returns to setup. */
        trace_reset();
        schedule_nmi(1u, 1u);
        nesturbator__cpu_step(nes);
        CHECK_EQ_HEX(nes->cpu.pc, 0x8001u);
        guest_step(nes);
        CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_NMI_VECTOR);
        *nmi_p = current_entry_p(nes);
        guest_step(nes); /* NMI handler NOP */
        guest_step(nes); /* RTI */
        CHECK_EQ_HEX(nes->cpu.pc, 0x8001u);
    } else {
        guest_step(nes); /* NOP */
    }

    /* Finish source flag setup through real guest instructions. */
    while (nes->cpu.pc < 0x8007u)
        guest_step(nes);
    CHECK_EQ_HEX(nes->cpu.pc, 0x8007u);

    if (irq_origin != 0u) {
        /* Assert the APU frame source at SEI C1 through the owned wrapper;
           SEI accepts using old I=0, then pushes P with I set. */
        trace_reset();
        entry_schedule.raise_frame_irq_access = 1u;
        nesturbator__cpu_step(nes);
        CHECK_EQ_U64(nes->apu.frame_irq, 1u);
        CHECK_EQ_U64(nes->cpu.irq_captured, 1u);
        CHECK_EQ_U64(nes->cpu.irq_accepted, 1u);
        CHECK_EQ_HEX(nes->cpu.p, 0x26u);
    } else {
        guest_step(nes); /* SEI before the following software BRK. */
    }

    if (middle != 0u || late != 0u) {
        /* Source rows 10..24 / 10..35 exercise the mapped pre-C5 window.
           Late rows arrive after that choice but before external vector C6. */
        trace_reset();
        schedule_nmi(middle != 0u ? 4u : 6u, 0u);
    } else {
        trace_reset();
    }
    nesturbator__cpu_step(nes); /* BRK or accepted APU IRQ entry. */

    uint8_t nmi_selected = (uint8_t)(middle != 0u);
    if (nmi_selected != 0u)
        CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_NMI_VECTOR);
    else
        CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_IRQ_VECTOR);

    uint8_t lower_dispatched = (uint8_t)(nes->cpu.pc == INTERRUPT_FIXTURE_IRQ_VECTOR);
    uint8_t nmi_selected_first = (uint8_t)(nes->cpu.pc == INTERRUPT_FIXTURE_NMI_VECTOR);
    uint8_t origin_p = 0u;
    if (nmi_selected_first != 0u) {
        *nmi_p = current_entry_p(nes);
    } else if (lower_dispatched != 0u) {
        origin_p = current_entry_p(nes);
        for (uint8_t step = 0u; step < 8u; step++) {
            uint16_t old_pc = nes->cpu.pc;
            guest_step(nes);
            if (nes->cpu.pc == INTERRUPT_FIXTURE_NMI_VECTOR && *nmi_p == 0u) {
                *nmi_p = current_entry_p(nes);
            }
            if (old_pc == INTERRUPT_FIXTURE_IRQ_VECTOR &&
                nes->cpu.pc == (uint16_t)(INTERRUPT_FIXTURE_IRQ_VECTOR + 1u)) {
                *lower_p = origin_p;
            }
            if (*lower_p != 0u && *nmi_p != 0u)
                break;
        }
    }

    /* The pre-overlap row is a clean sibling: NMI A4 first, then origin
       entry with its independent B semantics. Middle-row failures are
       retained as source-table evidence for Plan 04 to remove. */
    if (pre != 0u) {
        CHECK_EQ_HEX(*nmi_p, 0xa4u);
        CHECK_EQ_HEX(*lower_p, irq_origin != 0u ? 0x26u : 0x36u);
    }
    nesturbator_destroy((nesturbator *)nes);
}

static void print_observed_status_table(const char *name, const uint8_t *table, size_t count)
{
    printf("OBSERVED_TABLE %s count=%zu values=", name, count);
    for (size_t i = 0u; i < count; i++)
        printf("%02x%s", table[i], i + 1u == count ? "\n" : ",");
}

static size_t first_table_difference(const uint8_t *observed, const uint8_t *answer, size_t count)
{
    for (size_t i = 0u; i < count; i++) {
        if (observed[i] != answer[i])
            return i;
    }
    return count;
}

static uint8_t run_brk_partial_hijack_observation(void)
{
    struct nesturbator *nes = interrupt_fixture_build(
        overlap_program, sizeof overlap_program, overlap_nmi_handler, sizeof overlap_nmi_handler,
        overlap_lower_handler, sizeof overlap_lower_handler);
    CHECK(nes != NULL);
    if (nes == NULL)
        return 0u;
    interrupt_fixture_start(nes, 0x8000u, 0xa4u, 0u);
    for (uint8_t i = 0u; i < 6u; i++)
        guest_step(nes);
    guest_step(nes); /* SEI */
    trace_reset();
    schedule_nmi(6u, 0u);
    nesturbator__cpu_step(nes); /* C6 arrives after the latched T5 choice. */
    CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_IRQ_VECTOR);
    guest_step(nes); /* The first IRQ-handler instruction is the next legal poll. */
    CHECK(nes->cpu.pc != INTERRUPT_FIXTURE_NMI_VECTOR);
    guest_step(nes); /* The captured late edge is now serviced. */
    CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_NMI_VECTOR);
    uint8_t nmi_p = current_entry_p(nes);
    /* Lower table remains the real guest's zero marker at this cutpoint:
       the lower handler has not run yet. */
    CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_NMI_VECTOR);
    nesturbator_destroy((nesturbator *)nes);
    return nmi_p;
}

static void test_source_shaped_complete_paired_overlap_tables(void)
{
    uint8_t brk_nmi[INTERRUPT_BRK_OVERLAP_SAMPLES] = {0};
    uint8_t brk_lower[INTERRUPT_BRK_OVERLAP_SAMPLES] = {0};
    uint8_t irq_nmi[INTERRUPT_IRQ_OVERLAP_SAMPLES] = {0};
    uint8_t irq_lower[INTERRUPT_IRQ_OVERLAP_SAMPLES] = {0};

    for (uint8_t i = 0u; i < INTERRUPT_BRK_OVERLAP_SAMPLES; i++)
        run_overlap_sample(0u, i, &brk_nmi[i], &brk_lower[i]);
    for (uint8_t i = 0u; i < INTERRUPT_IRQ_OVERLAP_SAMPLES; i++)
        run_overlap_sample(1u, i, &irq_nmi[i], &irq_lower[i]);

    for (uint8_t i = 0u; i < 10u; i++) {
        CHECK_EQ_HEX(brk_nmi[i], 0xa4u);
        CHECK_EQ_HEX(brk_lower[i], 0x36u);
        CHECK_EQ_HEX(irq_nmi[i], 0xa4u);
        CHECK_EQ_HEX(irq_lower[i], 0x26u);
    }
    for (uint8_t i = 36u; i < INTERRUPT_IRQ_OVERLAP_SAMPLES; i++) {
        CHECK_EQ_HEX(irq_nmi[i], 0x26u);
        CHECK_EQ_HEX(irq_lower[i], 0x26u);
    }
    for (uint8_t i = 25u; i < INTERRUPT_BRK_OVERLAP_SAMPLES; i++) {
        CHECK_EQ_HEX(brk_nmi[i], 0x26u);
        CHECK_EQ_HEX(brk_lower[i], 0x36u);
    }
    print_observed_status_table("NmiAndBrk.NMI", brk_nmi, INTERRUPT_BRK_OVERLAP_SAMPLES);
    print_observed_status_table("NmiAndBrk.BRK", brk_lower, INTERRUPT_BRK_OVERLAP_SAMPLES);
    print_observed_status_table("NmiAndIrq.NMI", irq_nmi, INTERRUPT_IRQ_OVERLAP_SAMPLES);
    print_observed_status_table("NmiAndIrq.IRQ", irq_lower, INTERRUPT_IRQ_OVERLAP_SAMPLES);

    /* Actual CPU stack observations are compared with both complete pinned
       source tables. One alignment must explain both handlers in each pair. */
    enum interrupt_table_alignment brk_alignment =
        interrupt_paired_table_alignment(brk_nmi, brk_lower, interrupt_brk_answer_nmi,
                                         interrupt_brk_answer_lower, INTERRUPT_BRK_OVERLAP_SAMPLES);
    enum interrupt_table_alignment irq_alignment =
        interrupt_paired_table_alignment(irq_nmi, irq_lower, interrupt_irq_answer_nmi,
                                         interrupt_irq_answer_lower, INTERRUPT_IRQ_OVERLAP_SAMPLES);
    const uint32_t valid_alignment_mask =
        (1u << INTERRUPT_TABLE_EXACT) | (1u << INTERRUPT_TABLE_SHIFTED);
    for (uint8_t table = 0u; table < 2u; table++) {
        enum interrupt_table_alignment alignment = table == 0u ? brk_alignment : irq_alignment;
        CHECK((valid_alignment_mask & (1u << alignment)) != 0u);
        const uint8_t *observed_nmi = table == 0u ? brk_nmi : irq_nmi;
        const uint8_t *observed_lower = table == 0u ? brk_lower : irq_lower;
        const uint8_t *answer_nmi =
            table == 0u ? interrupt_brk_answer_nmi : interrupt_irq_answer_nmi;
        const uint8_t *answer_lower =
            table == 0u ? interrupt_brk_answer_lower : interrupt_irq_answer_lower;
        size_t count = table == 0u ? INTERRUPT_BRK_OVERLAP_SAMPLES : INTERRUPT_IRQ_OVERLAP_SAMPLES;
        if (alignment != INTERRUPT_TABLE_EXACT && alignment != INTERRUPT_TABLE_SHIFTED) {
            fprintf(stderr,
                    "source-paired-table-%s alignment=%u first-exact-diffs=nmi:%zu lower:%zu\n",
                    table == 0u ? "BRK" : "IRQ", (unsigned)alignment,
                    first_table_difference(observed_nmi, answer_nmi, count),
                    first_table_difference(observed_lower, answer_lower, count));
        } else {
            printf("CASE source-paired-table-%s alignment=%s first-exact-diffs=nmi:%zu lower:%zu\n",
                   table == 0u ? "BRK" : "IRQ",
                   alignment == INTERRUPT_TABLE_EXACT ? "exact" : "common-shift",
                   first_table_difference(observed_nmi, answer_nmi, count),
                   first_table_difference(observed_lower, answer_lower, count));
        }
    }

    /* This wrong model independently shifts only the BRK-handler result.
       NMI rows 0..9 and lower rows 1..9 are actual clean guest captures; the
       final lower value is a second real guest stopped before its lower
       handler starts, where the handler-visible marker is still zero. */
    uint8_t mixed_lower[10];
    memcpy(mixed_lower, brk_lower + 1u, 9u);
    mixed_lower[9] = run_brk_partial_hijack_observation() == 0x26u ? 0u : 0xffu;
    CHECK_EQ_U64(interrupt_table_alignment_of(brk_nmi, interrupt_brk_answer_nmi, 10u),
                 INTERRUPT_TABLE_EXACT);
    CHECK_EQ_U64(interrupt_table_alignment_of(mixed_lower, interrupt_brk_answer_lower, 10u),
                 INTERRUPT_TABLE_SHIFTED);
    CHECK_EQ_U64(interrupt_paired_table_alignment(brk_nmi, mixed_lower, interrupt_brk_answer_nmi,
                                                  interrupt_brk_answer_lower, 10u),
                 INTERRUPT_TABLE_MIXED);
    printf("EXPECTED_RED control=independent-handler-shift clean-NMI=exact "
           "shifted-lower=shifted paired-verdict=mixed\n");

    uint8_t per_byte_nmi[INTERRUPT_BRK_OVERLAP_SAMPLES];
    uint8_t per_byte_lower[INTERRUPT_BRK_OVERLAP_SAMPLES];
    memcpy(per_byte_nmi, brk_nmi, sizeof per_byte_nmi);
    memcpy(per_byte_lower, brk_lower, sizeof per_byte_lower);
    for (uint8_t i = 1u; i + 1u < INTERRUPT_BRK_OVERLAP_SAMPLES; i += 2u) {
        per_byte_nmi[i] = brk_nmi[i + 1u];
        per_byte_lower[i] = brk_lower[i + 1u];
    }
    enum interrupt_table_alignment per_byte_alignment =
        interrupt_paired_table_alignment(per_byte_nmi, per_byte_lower, interrupt_brk_answer_nmi,
                                         interrupt_brk_answer_lower, INTERRUPT_BRK_OVERLAP_SAMPLES);
    CHECK(per_byte_alignment != INTERRUPT_TABLE_EXACT &&
          per_byte_alignment != INTERRUPT_TABLE_SHIFTED);
    printf("EXPECTED_RED control=per-byte-index-choice actual-verdict=%u\n",
           (unsigned)per_byte_alignment);
    printf("CASE guard source-shaped guest sweeps captured both pushed-P tables rows=32/64\n");
}

static void test_every_entry_arrival_action(void)
{
    for (uint8_t origin = 0u; origin < 2u; origin++) {
        for (uint8_t action = INTERRUPT_ACTION_BEFORE_C1; action <= INTERRUPT_ACTION_BEFORE_C7;
             action++) {
            struct nesturbator *nes =
                origin == 0u ? build_entry_guest(0x20u) : prepare_frame_irq_entry();
            CHECK(nes != NULL);
            if (nes == NULL)
                continue;
            if (origin == 0u)
                nes->cpu.pc = 0x8001u;
            trace_reset();
            schedule_nmi(action, 0u);
            nesturbator__cpu_step(nes);
            struct entry_observation observed =
                observe_entry(nes, nes->cpu.pc == INTERRUPT_FIXTURE_NMI_VECTOR ? 0xfffau : 0xfffeu);
            uint16_t expected = action <= INTERRUPT_ACTION_BEFORE_C4 ? INTERRUPT_FIXTURE_NMI_VECTOR
                                                                     : INTERRUPT_FIXTURE_IRQ_VECTOR;
            uint16_t expected_vector = action <= INTERRUPT_ACTION_BEFORE_C4 ? 0xfffau : 0xfffeu;
            CHECK_EQ_U64(
                check_vector_pc(&observed, expected, origin == 0u ? "BRK-arrival" : "IRQ-arrival"),
                0u);
            CHECK_EQ_U64(entry_access_count, 7u);
            CHECK_EQ_HEX(observed.pch, 0x80u);
            CHECK_EQ_HEX(observed.pcl, origin == 0u ? 0x03u : 0x01u);
            CHECK_EQ_HEX(observed.pushed_p, origin == 0u ? 0x30u : 0x20u);
            CHECK_EQ_HEX(observed.low_vector_address, expected_vector);
            CHECK_EQ_HEX(observed.high_vector_address, (uint16_t)(expected_vector + 1u));
            if (origin != 0u) {
                check_entry_bus_shape(0x8001u, 0x20u, expected_vector);
            } else if (entry_access_count == 7u) {
                const uint16_t brk_addresses[7] = {0x8001u,
                                                   0x8002u,
                                                   0x01fdu,
                                                   0x01fcu,
                                                   0x01fbu,
                                                   expected_vector,
                                                   (uint16_t)(expected_vector + 1u)};
                const uint8_t brk_values[7] = {0x00u,
                                               0x5au,
                                               0x80u,
                                               0x03u,
                                               0x30u,
                                               observed.low_vector_value,
                                               observed.high_vector_value};
                for (uint32_t i = 0u; i < 7u; i++) {
                    CHECK_EQ_HEX(entry_log[i].addr, brk_addresses[i]);
                    CHECK_EQ_HEX(entry_log[i].value, brk_values[i]);
                    CHECK_EQ_U64(entry_log[i].ticks_after - entry_log[i].ticks_before, 24u);
                }
            }
            printf("CASE %s arrival-before-C%u observed=0x%04x low=%04x high=%04x\n",
                   origin == 0u ? "BRK" : "IRQ", action, observed.pc, observed.low_vector_address,
                   observed.high_vector_address);
            nesturbator_destroy((nesturbator *)nes);
        }
    }
}

enum entry_contract_error {
    ENTRY_CONTRACT_VECTOR = 1u << 0,
    ENTRY_CONTRACT_STATUS = 1u << 1,
    ENTRY_CONTRACT_RETURN_PC = 1u << 2,
    ENTRY_CONTRACT_VECTOR_PAIR = 1u << 3
};

struct entry_contract {
    uint16_t pc;
    uint8_t pushed_p;
    uint16_t return_pc;
    uint16_t vector_low;
    uint16_t vector_high;
};

static uint32_t entry_contract_errors(const struct entry_observation *observed,
                                      const struct entry_contract *expected)
{
    uint32_t errors = 0u;
    if (observed->pc != expected->pc)
        errors |= ENTRY_CONTRACT_VECTOR;
    if (observed->pushed_p != expected->pushed_p)
        errors |= ENTRY_CONTRACT_STATUS;
    if ((((uint16_t)observed->pch << 8) | observed->pcl) != expected->return_pc)
        errors |= ENTRY_CONTRACT_RETURN_PC;
    if (observed->low_vector_address != expected->vector_low ||
        observed->high_vector_address != expected->vector_high ||
        (((uint16_t)observed->high_vector_value << 8) | observed->low_vector_value) != expected->pc)
        errors |= ENTRY_CONTRACT_VECTOR_PAIR;
    return errors;
}

static void expected_red_contract(const char *name, const struct entry_observation *observed,
                                  const struct entry_contract *expected, uint32_t expected_error)
{
    uint32_t errors = entry_contract_errors(observed, expected);
    int before = check_failures;
    CHECK_EQ_U64(errors, 0u);
    unsigned rejected = (unsigned)(check_failures - before);
    check_failures = before;
    CHECK_EQ_U64(rejected, 1u);
    CHECK_EQ_U64(errors, expected_error);
    printf("EXPECTED_RED control=%s intended-error=0x%x observed-errors=0x%x\n", name,
           expected_error, errors);
}

static void test_wrong_model_controls_use_clean_entry_siblings(void)
{
    struct nesturbator *irq = prepare_frame_irq_entry();
    CHECK(irq != NULL);
    if (irq != NULL) {
        trace_reset();
        nesturbator__cpu_step(irq);
        struct entry_observation actual = observe_entry(irq, 0xfffeu);
        const struct entry_contract expected_irq = {INTERRUPT_FIXTURE_IRQ_VECTOR, 0x20u, 0x8001u,
                                                    0xfffeu, 0xffffu};
        CHECK_EQ_U64(entry_contract_errors(&actual, &expected_irq), 0u);
        struct entry_observation wrong_b = actual;
        wrong_b.pushed_p |= ENTRY_FLAG_B;
        expected_red_contract("wrong-B-bit", &wrong_b, &expected_irq, ENTRY_CONTRACT_STATUS);
        struct entry_observation mixed_vector = actual;
        mixed_vector.high_vector_address = 0xfffbu;
        mixed_vector.high_vector_value = 0x81u;
        expected_red_contract("mixed-vector-byte-pair", &mixed_vector, &expected_irq,
                              ENTRY_CONTRACT_VECTOR_PAIR);
        nesturbator_destroy((nesturbator *)irq);
    }

    struct nesturbator *brk = build_entry_guest(0x20u);
    CHECK(brk != NULL);
    if (brk != NULL) {
        trace_reset();
        brk->cpu.pc = 0x8001u;
        nesturbator__cpu_step(brk);
        struct entry_observation actual = observe_entry(brk, 0xfffeu);
        const struct entry_contract expected_brk = {INTERRUPT_FIXTURE_IRQ_VECTOR, 0x30u, 0x8003u,
                                                    0xfffeu, 0xffffu};
        CHECK_EQ_U64(entry_contract_errors(&actual, &expected_brk), 0u);
        struct entry_observation wrong_return = actual;
        wrong_return.pcl = 0x02u;
        expected_red_contract("wrong-BRK-return-PC", &wrong_return, &expected_brk,
                              ENTRY_CONTRACT_RETURN_PC);
        nesturbator_destroy((nesturbator *)brk);
    }

    /* The real PPU line and APU frame source are captured by the same
       production bus instruction poll. NMI must win the simultaneous poll. */
    struct nesturbator *both = build_entry_guest(0x20u);
    CHECK(both != NULL);
    if (both != NULL) {
        trace_reset();
        schedule_nmi(1u, 0u);
        entry_schedule.raise_frame_irq_access = 1u;
        nesturbator__cpu_step(both);
        CHECK_EQ_U64(both->cpu.nmi_polled, 1u);
        CHECK_EQ_U64(both->cpu.irq_accepted, 1u);
        trace_reset();
        nesturbator__cpu_step(both);
        struct entry_observation actual = observe_entry(both, 0xfffau);
        const struct entry_contract expected_nmi = {INTERRUPT_FIXTURE_NMI_VECTOR, 0x20u, 0x8001u,
                                                    0xfffau, 0xfffbu};
        CHECK_EQ_U64(entry_contract_errors(&actual, &expected_nmi), 0u);
        struct entry_observation wrong_priority = actual;
        wrong_priority.pc = INTERRUPT_FIXTURE_IRQ_VECTOR;
        expected_red_contract("wrong-simultaneous-priority", &wrong_priority, &expected_nmi,
                              ENTRY_CONTRACT_VECTOR);
        nesturbator_destroy((nesturbator *)both);
    }
}

static void test_simultaneous_nmi_consumes_arbitration_not_queued_irq(void)
{
    static const uint8_t program[] = {0xeau, 0xeau, 0x4cu, 0x01u, 0x80u};
    static const uint8_t ack_handler[] = {0xe6u, ENTRY_NMI_MARKER, 0xadu, 0x15u, 0x40u,
                                          0x40u}; /* INC marker; LDA $4015; RTI */
    for (uint8_t acknowledge = 0u; acknowledge < 2u; acknowledge++) {
        struct nesturbator *nes = interrupt_fixture_build(
            program, sizeof program, acknowledge != 0u ? ack_handler : nmi_handler,
            acknowledge != 0u ? sizeof ack_handler : sizeof nmi_handler, irq_handler,
            sizeof irq_handler);
        CHECK(nes != NULL);
        if (nes == NULL)
            continue;
        interrupt_fixture_start(nes, 0x8000u, 0x20u, 0u);
        trace_reset();
        schedule_nmi(1u, 2u);
        entry_schedule.raise_frame_irq_access = 1u;
        nesturbator__cpu_step(nes); /* Both inputs accepted by the same NOP. */
        CHECK_EQ_U64(nes->cpu.nmi_polled, 1u);
        CHECK_EQ_U64(nes->cpu.irq_accepted, 1u);
        guest_step(nes);
        CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_NMI_VECTOR);
        CHECK_EQ_U64(nes->cpu.irq_accepted, 0u);
        CHECK((nes->cpu.p & ENTRY_FLAG_I) != 0u);
        guest_step(nes); /* The NMI handler must run before any IRQ entry. */
        CHECK_EQ_HEX(nes->bus.ram[ENTRY_NMI_MARKER], 1u);
        CHECK_EQ_HEX(nes->bus.ram[ENTRY_IRQ_MARKER], 0u);
        if (acknowledge != 0u)
            guest_step(nes); /* Read the real APU status register. */
        guest_step(nes);     /* RTI restores I=0, then polls the current IRQ. */
        CHECK_EQ_HEX(nes->cpu.pc, 0x8001u);
        CHECK_EQ_U64(nes->cpu.irq_accepted, acknowledge != 0u ? 0u : 1u);
        guest_step(nes);
        CHECK_EQ_HEX(nes->cpu.pc, acknowledge != 0u ? 0x8002u : INTERRUPT_FIXTURE_IRQ_VECTOR);
        CHECK_EQ_HEX(nes->bus.ram[ENTRY_IRQ_MARKER], 0u);
        printf("CASE simultaneous NMI/IRQ arbitration consumed; source=%s after-RTI=%s\n",
               acknowledge != 0u ? "acknowledged" : "held",
               acknowledge != 0u ? "guest-opcode" : "fresh-IRQ-entry");
        nesturbator_destroy((nesturbator *)nes);
    }
}

static void test_sampled_inhibited_pulses_do_not_queue_service(void)
{
    for (uint8_t origin = 0u; origin < 3u; origin++) {
        for (uint32_t action = 5u; action <= 7u; action++) {
            struct nesturbator *nes = origin == 0u   ? build_entry_guest(0x20u)
                                      : origin == 1u ? prepare_frame_irq_entry()
                                                     : prepare_natural_nmi_entry();
            CHECK(nes != NULL);
            if (nes == NULL)
                continue;
            if (origin == 0u)
                nes->cpu.pc = 0x8001u;
            /* Release the old NMI before C1 so this is a distinct raw edge.
               C5/C6 pulses have a complete later entry cycle before T1.
               C7 release is the deterministic boundary sibling, not a claim
               about the physical setup/hold winner at that seam. */
            nes->ppu.control &= 0x7fu;
            trace_reset();
            schedule_nmi(action, action);
            nesturbator__cpu_step(nes);
            CHECK_EQ_U64(nesturbator__ppu_nmi_line(nes), 0u);
            if (action == 7u)
                CHECK_EQ_U64(nes->cpu.nmi_prev, 1u); /* It really was sampled. */
            CHECK_EQ_HEX(nes->cpu.pc, origin == 2u ? INTERRUPT_FIXTURE_NMI_VECTOR
                                                   : INTERRUPT_FIXTURE_IRQ_VECTOR);
            /* Save the real post-entry history, including a C7 candidate
               whose raw sample was high but whose current input is low. */
            size_t size = nesturbator_state_size((nesturbator *)nes);
            uint8_t *state = (uint8_t *)malloc(size);
            struct nesturbator *loaded = build_entry_guest(0x20u);
            CHECK(state != NULL && loaded != NULL);
            if (state != NULL && loaded != NULL) {
                CHECK_EQ_U64(nesturbator_save_state((nesturbator *)nes, state, size),
                             NESTURBATOR_OK);
                CHECK_EQ_U64(nesturbator_load_state((nesturbator *)loaded, state, size),
                             NESTURBATOR_OK);
                nesturbator_destroy((nesturbator *)nes);
                nes = loaded;
                loaded = NULL;
            }
            free(state);
            nesturbator_destroy((nesturbator *)loaded);
            guest_step(nes); /* First handler instruction executes. */
            CHECK_EQ_U64(nes->cpu.nmi_polled, 0u);
            guest_step(nes); /* Handler RTI, not an extra NMI. */
            CHECK_EQ_HEX(nes->cpu.pc, origin == 0u ? 0x8003u : 0x8001u);
            CHECK_EQ_HEX(nes->bus.ram[ENTRY_NMI_MARKER], origin == 2u ? 1u : 0u);
            CHECK_EQ_HEX(nes->bus.ram[ENTRY_IRQ_MARKER], origin == 2u ? 0u : 1u);
            printf("CASE sampled inhibited pulse origin=%u C%u release-before-handler no-service\n",
                   origin, (unsigned)action);
            nesturbator_destroy((nesturbator *)nes);
        }
    }
}

static void test_held_inhibited_inputs_and_consumed_level(void)
{
    for (uint8_t origin = 0u; origin < 3u; origin++) {
        for (uint32_t action = 5u; action <= 7u; action++) {
            struct nesturbator *nes = origin == 0u   ? build_entry_guest(0x20u)
                                      : origin == 1u ? prepare_frame_irq_entry()
                                                     : prepare_natural_nmi_entry();
            CHECK(nes != NULL);
            if (nes == NULL)
                continue;
            if (origin == 0u)
                nes->cpu.pc = 0x8001u;
            nes->ppu.control &= 0x7fu;
            trace_reset();
            schedule_nmi(action, 0u);
            nesturbator__cpu_step(nes);
            CHECK_EQ_U64(nes->cpu.nmi_pending, NESTURBATOR_NMI_INHIBITED);
            guest_step(nes);
            CHECK_EQ_U64(nes->cpu.nmi_pending, NESTURBATOR_NMI_RECOGNIZED);
            CHECK_EQ_U64(nes->cpu.nmi_polled, 1u);
            nes->ppu.control &= 0x7fu;
            guest_step(nes);
            CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_NMI_VECTOR);
            guest_step(nes);
            CHECK_EQ_HEX(nes->bus.ram[ENTRY_NMI_MARKER], origin == 2u ? 2u : 1u);
            guest_step(nes); /* NMI RTI. */
            CHECK_EQ_U64(nes->cpu.nmi_pending, NESTURBATOR_NMI_NONE);
            nesturbator_destroy((nesturbator *)nes);
        }
    }
    struct nesturbator *consumed = prepare_natural_nmi_entry();
    CHECK(consumed != NULL);
    if (consumed != NULL) {
        guest_step(consumed); /* Original NMI input remains asserted. */
        guest_step(consumed); /* Handler marker. */
        guest_step(consumed); /* RTI. */
        CHECK_EQ_HEX(consumed->bus.ram[ENTRY_NMI_MARKER], 1u);
        CHECK_EQ_U64(nesturbator__ppu_nmi_line(consumed), 1u);
        CHECK_EQ_U64(consumed->cpu.nmi_pending, NESTURBATOR_NMI_NONE);
        CHECK_EQ_U64(consumed->cpu.nmi_polled, 0u);
        nesturbator_destroy((nesturbator *)consumed);
    }
}

static void test_first_handler_branch_recognizes_only_held_late_input(void)
{
    static const uint8_t handler[] = {
        0x90u, 0x00u, /* BCC to next instruction, taken without page crossing. */
        0xe6u, ENTRY_IRQ_MARKER, 0x40u};
    for (uint8_t held = 0u; held < 2u; held++) {
        struct nesturbator *nes =
            interrupt_fixture_build(entry_program, sizeof entry_program, nmi_handler,
                                    sizeof nmi_handler, handler, sizeof handler);
        CHECK(nes != NULL);
        if (nes == NULL)
            continue;
        interrupt_fixture_start(nes, 0x8000u, 0x20u, 29827u);
        trace_reset();
        nesturbator__cpu_step(nes); /* Accept real frame IRQ. */
        trace_reset();
        schedule_nmi(6u, held != 0u ? 0u : 6u);
        nesturbator__cpu_step(nes);
        CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_IRQ_VECTOR);
        guest_step(nes); /* The first handler branch's C1 is its legal poll. */
        CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_IRQ_VECTOR + 2u);
        CHECK_EQ_U64(entry_access_count, 3u);
        CHECK_EQ_U64(nes->cpu.nmi_polled, held);
        nes->ppu.control &= 0x7fu; /* A recognized input survives release. */
        guest_step(nes);
        CHECK_EQ_HEX(nes->cpu.pc,
                     held != 0u ? INTERRUPT_FIXTURE_NMI_VECTOR : INTERRUPT_FIXTURE_IRQ_VECTOR + 4u);
        CHECK_EQ_HEX(nes->bus.ram[ENTRY_IRQ_MARKER], held != 0u ? 0u : 1u);
        nesturbator_destroy((nesturbator *)nes);
    }
}

static void test_late_sustained_request_and_inhibited_short_pulse(void)
{
    struct nesturbator *held = prepare_frame_irq_entry();
    CHECK(held != NULL);
    if (held != NULL) {
        trace_reset();
        schedule_nmi(6u, 0u); /* Held from late entry through lower handler. */
        nesturbator__cpu_step(held);
        CHECK_EQ_HEX(held->cpu.pc, INTERRUPT_FIXTURE_IRQ_VECTOR);
        CHECK_EQ_HEX(held->bus.ram[ENTRY_IRQ_MARKER], 0u);
        guest_step(held); /* One lower-priority instruction runs first. */
        CHECK_EQ_HEX(held->cpu.pc, INTERRUPT_FIXTURE_IRQ_VECTOR + 2u);
        CHECK_EQ_HEX(held->bus.ram[ENTRY_IRQ_MARKER], 1u);
        CHECK_EQ_HEX(held->cpu.pc, INTERRUPT_FIXTURE_IRQ_VECTOR + 2u);
        guest_step(held);
        CHECK_EQ_HEX(held->cpu.pc, INTERRUPT_FIXTURE_NMI_VECTOR);
        CHECK_EQ_HEX(current_entry_p(held), 0x24u);
        guest_step(held); /* NMI handler marker */
        CHECK_EQ_HEX(held->bus.ram[ENTRY_NMI_MARKER], 1u);
        guest_step(held); /* RTI */
        guest_step(held); /* Remaining lower RTI; held level has no new edge. */
        guest_step(held); /* Return to guest and poll held level again. */
        CHECK_EQ_HEX(held->bus.ram[ENTRY_NMI_MARKER], 1u);
        CHECK_EQ_U64(held->cpu.nmi_pending, 0u);
        printf(
            "CASE guard late-sustained edge lower-instruction-before-NMI no-duplicate marker=1\n");

        /* A wrong lost-request model must fail the same marker-count check;
           the actual held-line sibling above passes exactly once. */
        uint8_t observed_count = held->bus.ram[ENTRY_NMI_MARKER];
        CHECK_EQ_U64(observed_count, 1u);
        for (uint8_t wrong_count = 0u; wrong_count <= 2u; wrong_count += 2u) {
            int before = check_failures;
            CHECK_EQ_U64(wrong_count, observed_count);
            unsigned rejected = (unsigned)(check_failures - before);
            check_failures = before;
            CHECK_EQ_U64(rejected, 1u);
            printf("EXPECTED_RED control=%s-marker intended-count=%u actual-count=%u rejected=%u\n",
                   wrong_count == 0u ? "lost-request" : "duplicate-held-level", wrong_count,
                   observed_count, rejected);
        }
        nesturbator_destroy((nesturbator *)held);
    }

    struct nesturbator *missed = prepare_frame_irq_entry();
    CHECK(missed != NULL);
    if (missed != NULL) {
        trace_reset();
        nesturbator__cpu_step(missed); /* Entry completes with no sampled NMI. */
        /* Construct a pulse entirely between the final entry close sample and
           the next production bus access. This is deliberately below the
           model's sampling aperture, so it demonstrates a miss rather than
           claiming a physical circuit pulse is always retained. */
        missed->ppu.status |= 0x80u;
        missed->ppu.control |= 0x80u;
        CHECK_EQ_U64(nesturbator__ppu_nmi_line(missed), 1u);
        missed->ppu.control &= 0x7fu;
        CHECK_EQ_U64(nesturbator__ppu_nmi_line(missed), 0u);
        CHECK_EQ_U64(missed->cpu.nmi_pending, 0u);
        guest_step(missed); /* Lower handler first instruction. */
        CHECK_EQ_HEX(missed->bus.ram[ENTRY_IRQ_MARKER], 1u);
        CHECK_EQ_U64(missed->cpu.nmi_polled, 0u);
        printf("CASE constructed-inhibited-short-pulse released-between-close-samples missed\n");
        nesturbator_destroy((nesturbator *)missed);
    }
}

static void test_new_edge_during_older_nmi_entry(void)
{
    struct nesturbator *nes = prepare_natural_nmi_entry();
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    /* Release the consumed input before C1 samples low. C3 then supplies a
       genuinely new eligible edge; C5's inhibited pulse must not erase it. */
    nes->ppu.control &= 0x7fu;
    trace_reset();
    schedule_nmi(3u, 3u);
    schedule_nmi(5u, 5u);
    nesturbator__cpu_step(nes); /* Services old edge; captures a new edge in-entry. */
    CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_NMI_VECTOR);
    CHECK_EQ_U64(nes->cpu.nmi_pending, 1u);
    CHECK_EQ_U64(nesturbator__ppu_nmi_line(nes), 0u);
    guest_step(nes); /* First handler instruction is the next poll. */
    CHECK_EQ_HEX(nes->bus.ram[ENTRY_NMI_MARKER], 1u);
    guest_step(nes); /* New retained edge dispatches independently. */
    CHECK_EQ_HEX(nes->cpu.pc, INTERRUPT_FIXTURE_NMI_VECTOR);
    guest_step(nes);
    CHECK_EQ_HEX(nes->bus.ram[ENTRY_NMI_MARKER], 2u);
    printf(
        "CASE guard new NMI edge during older entry survives line release and is consumed once\n");
    nesturbator_destroy((nesturbator *)nes);
}

static void test_irq_acknowledged_and_unacknowledged_lifetime(void)
{
    struct nesturbator *live = prepare_frame_irq_entry();
    CHECK(live != NULL);
    if (live != NULL) {
        trace_reset();
        nesturbator__cpu_step(live);
        guest_step(live); /* IRQ handler INC, no source acknowledgement. */
        guest_step(live); /* RTI restores I=0 and accepts the still-live IRQ. */
        CHECK_EQ_U64(live->apu.frame_irq, 1u);
        CHECK_EQ_U64(live->cpu.irq_accepted, 1u);
        guest_step(live);
        CHECK_EQ_HEX(live->cpu.pc, INTERRUPT_FIXTURE_IRQ_VECTOR);
        printf("CASE guard unacknowledged APU frame IRQ retriggers after RTI\n");
        nesturbator_destroy((nesturbator *)live);
    }

    static const uint8_t ack_handler[] = {0xadu, 0x15u, 0x40u, 0x40u};
    static const uint8_t ack_program[] = {0xeau, 0xeau, 0xeau};
    struct nesturbator *acked =
        interrupt_fixture_build(ack_program, sizeof ack_program, nmi_handler, sizeof nmi_handler,
                                ack_handler, sizeof ack_handler);
    CHECK(acked != NULL);
    if (acked != NULL) {
        interrupt_fixture_start(acked, 0x8000u, 0x20u, 29827u);
        guest_step(acked); /* Real APU frame IRQ source reaches acceptance. */
        CHECK_EQ_U64(acked->cpu.irq_accepted, 1u);
        guest_step(acked); /* IRQ entry. */
        CHECK_EQ_HEX(acked->cpu.pc, INTERRUPT_FIXTURE_IRQ_VECTOR);
        guest_step(acked); /* LDA $4015 acknowledges frame IRQ. */
        guest_step(acked); /* RTI; no live source remains to retrigger. */
        CHECK_EQ_U64(acked->apu.frame_irq, 0u);
        CHECK_EQ_U64(acked->cpu.irq_accepted, 0u);
        guest_step(acked);
        CHECK(acked->cpu.pc != INTERRUPT_FIXTURE_IRQ_VECTOR);
        printf("CASE guard acknowledged APU frame IRQ stays cleared after RTI\n");
        nesturbator_destroy((nesturbator *)acked);
    }
}

static void check_entry_cutpoint_roundtrip(struct nesturbator *source, struct nesturbator *restored,
                                           uint8_t expected_origin, uint8_t expected_action,
                                           uint8_t expected_vector, size_t state_size,
                                           uint8_t *state, uint8_t *roundtrip)
{
    CHECK_EQ_U64(source->cpu.entry_origin, expected_origin);
    CHECK_EQ_U64(source->cpu.entry_next_action, expected_action);
    CHECK_EQ_U64(source->cpu.entry_vector, expected_vector);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)source, state, state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_state((nesturbator *)restored, state, state_size),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)restored, roundtrip, state_size),
                 NESTURBATOR_OK);
    CHECK(memcmp(state, roundtrip, state_size) == 0);
}

static void test_native_state_roundtrips_entry_stage_matrix(void)
{
    struct nesturbator *probe = build_entry_guest(0x20u);
    const size_t state_size = nesturbator_state_size((nesturbator *)probe);
    uint8_t *state = (uint8_t *)malloc(state_size);
    uint8_t *roundtrip = (uint8_t *)malloc(state_size);
    nesturbator_destroy((nesturbator *)probe);
    CHECK(state_size != 0u && state != NULL && roundtrip != NULL);
    if (state_size == 0u || state == NULL || roundtrip == NULL) {
        free(state);
        free(roundtrip);
        return;
    }

    for (enum nesturbator_entry_origin origin = NESTURBATOR_ENTRY_BRK;
         origin <= NESTURBATOR_ENTRY_NMI; ++origin) {
        struct nesturbator *source = build_entry_guest(0x20u);
        struct nesturbator *restored = build_entry_guest(0x20u);
        CHECK(source != NULL && restored != NULL);
        if (source == NULL || restored == NULL) {
            nesturbator_destroy((nesturbator *)source);
            nesturbator_destroy((nesturbator *)restored);
            continue;
        }

        if (origin == NESTURBATOR_ENTRY_BRK) {
            source->cpu.pc = 0x8001u;
            (void)nesturbator__bus_read(source, source->cpu.pc); /* BRK C1 opcode fetch. */
            source->cpu.pc++;
        }
        nesturbator__cpu_entry_begin(source, origin);
        while (source->cpu.entry_origin != NESTURBATOR_ENTRY_NONE) {
            uint8_t expected_action = source->cpu.entry_next_action;
            uint8_t expected_vector = origin == NESTURBATOR_ENTRY_NMI ? NESTURBATOR_ENTRY_VECTOR_NMI
                                      : source->cpu.entry_next_action >= 5u
                                          ? NESTURBATOR_ENTRY_VECTOR_IRQ
                                          : NESTURBATOR_ENTRY_VECTOR_NONE;
            check_entry_cutpoint_roundtrip(source, restored, (uint8_t)origin,
                                           source->cpu.entry_next_action, expected_vector,
                                           state_size, state, roundtrip);
            trace_reset();
            nesturbator__cpu_step(restored); /* Ordinary execution drains the loaded entry. */
            CHECK_EQ_U64(restored->cpu.entry_origin, NESTURBATOR_ENTRY_NONE);
            CHECK_EQ_HEX(restored->cpu.pc, expected_vector == NESTURBATOR_ENTRY_VECTOR_NMI
                                               ? INTERRUPT_FIXTURE_NMI_VECTOR
                                               : INTERRUPT_FIXTURE_IRQ_VECTOR);
            CHECK_EQ_U64(entry_access_count, 8u - expected_action);
            if (nesturbator__cpu_entry_action(source) == 0u)
                break;
        }
        nesturbator_destroy((nesturbator *)source);
        nesturbator_destroy((nesturbator *)restored);
    }
    printf("CASE guard every reachable BRK/IRQ/NMI completed-action state loads and drains\n");
    free(state);
    free(roundtrip);
}

static void test_native_state_resumes_hijacked_vector_cutpoint(void)
{
    struct nesturbator *source = prepare_frame_irq_entry();
    struct nesturbator *restored = build_entry_guest(0x20u);
    uint8_t *state = NULL;
    uint8_t *source_after = NULL;
    uint8_t *restored_after = NULL;
    size_t state_size;
    CHECK(source != NULL && restored != NULL);
    if (source == NULL || restored == NULL) {
        nesturbator_destroy((nesturbator *)source);
        nesturbator_destroy((nesturbator *)restored);
        return;
    }

    trace_reset();
    schedule_nmi(4u, 4u); /* A brief eligible edge at the mapped C4/T5 boundary. */
    nesturbator__cpu_entry_begin(source, NESTURBATOR_ENTRY_IRQ);
    for (uint8_t action = 0u; action < 6u; ++action)
        CHECK_EQ_U64(nesturbator__cpu_entry_action(source), 1u);
    CHECK_EQ_U64(source->cpu.entry_next_action, 7u);
    CHECK_EQ_U64(source->cpu.entry_vector, NESTURBATOR_ENTRY_VECTOR_NMI);
    CHECK_EQ_U64(source->cpu.entry_vector_low, INTERRUPT_FIXTURE_NMI_VECTOR & 0xffu);
    CHECK_EQ_U64(source->cpu.nmi_pending, 0u); /* The old edge was consumed once at C4. */

    state_size = nesturbator_state_size((nesturbator *)source);
    state = (uint8_t *)malloc(state_size);
    source_after = (uint8_t *)malloc(state_size);
    restored_after = (uint8_t *)malloc(state_size);
    CHECK(state != NULL && source_after != NULL && restored_after != NULL);
    if (state == NULL || source_after == NULL || restored_after == NULL) {
        free(state);
        free(source_after);
        free(restored_after);
        nesturbator_destroy((nesturbator *)source);
        nesturbator_destroy((nesturbator *)restored);
        return;
    }

    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)source, state, state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_state((nesturbator *)restored, state, state_size),
                 NESTURBATOR_OK);
    trace_reset();
    nesturbator__cpu_step(source); /* C7 is the only remaining action. */
    CHECK_EQ_U64(entry_access_count, 1u);
    CHECK_EQ_HEX(entry_log[0].addr, 0xfffbu);
    CHECK_EQ_HEX(source->cpu.pc, INTERRUPT_FIXTURE_NMI_VECTOR);
    CHECK_EQ_U64(source->cpu.entry_origin, NESTURBATOR_ENTRY_NONE);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)source, source_after, state_size),
                 NESTURBATOR_OK);

    trace_reset();
    nesturbator__cpu_step(restored);
    CHECK_EQ_U64(entry_access_count, 1u);
    CHECK_EQ_HEX(entry_log[0].addr, 0xfffbu);
    CHECK_EQ_HEX(restored->cpu.pc, INTERRUPT_FIXTURE_NMI_VECTOR);
    CHECK_EQ_U64(restored->cpu.entry_origin, NESTURBATOR_ENTRY_NONE);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)restored, restored_after, state_size),
                 NESTURBATOR_OK);
    CHECK(memcmp(source_after, restored_after, state_size) == 0);
    printf("CASE guard public native state resumes selected nonzero NMI low byte at ordinary "
           "cpu_step\n");

    free(state);
    free(source_after);
    free(restored_after);
    nesturbator_destroy((nesturbator *)source);
    nesturbator_destroy((nesturbator *)restored);
}

int main(void)
{
    test_production_cpu_step_entry_origins();
    test_dmc_reload_supplies_jsr_target_opcode();
    test_dmc_resumed_mapped_read_updates_open_bus();
    test_source_qualified_finite_margin_mapping();
    test_every_entry_arrival_action();
    test_source_shaped_complete_paired_overlap_tables();
    test_wrong_model_controls_use_clean_entry_siblings();
    test_simultaneous_nmi_consumes_arbitration_not_queued_irq();
    test_sampled_inhibited_pulses_do_not_queue_service();
    test_held_inhibited_inputs_and_consumed_level();
    test_first_handler_branch_recognizes_only_held_late_input();
    test_late_sustained_request_and_inhibited_short_pulse();
    test_new_edge_during_older_nmi_entry();
    test_irq_acknowledged_and_unacknowledged_lifetime();
    test_native_state_roundtrips_entry_stage_matrix();
    test_native_state_resumes_hijacked_vector_cutpoint();
    CHECK_DONE();
}

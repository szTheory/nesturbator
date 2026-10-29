/* Owned NROM guests for IRQ poll and entry tests. */
#ifndef NESTURBATOR_TESTS_INTERRUPT_FIXTURE_H
#define NESTURBATOR_TESTS_INTERRUPT_FIXTURE_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "check.h"
#include "ines.h"
#include "internal.h"

#define INTERRUPT_FIXTURE_IMAGE_CAP 40000u
#define INTERRUPT_FIXTURE_NMI_VECTOR 0x8134u
#define INTERRUPT_FIXTURE_IRQ_VECTOR 0x8200u
#define INTERRUPT_FIXTURE_IRQ_COUNT 0x30u
#define INTERRUPT_FIXTURE_NMI_COUNT 0x31u
#define INTERRUPT_FIXTURE_ENTRY_PC 0x8020u
#define INTERRUPT_FIXTURE_ENTRY_RETURN_PC 0x8022u

int interrupt_state_run(void);

/* Cn names the core's external bus action, not Visual6502's Tn. The two
   finite-margin rows bracket the inferred boundary: D-06 maps T5 phi1 after
   the C4 PCL action and before C5 P; an external-C6 policy waits until the
   low-vector read. No exact-timestamp hardware tie is encoded here. */
enum interrupt_fixture_action {
    INTERRUPT_ACTION_BEFORE_C1 = 1,
    INTERRUPT_ACTION_BEFORE_C2,
    INTERRUPT_ACTION_BEFORE_C3,
    INTERRUPT_ACTION_BEFORE_C4,
    INTERRUPT_ACTION_BEFORE_C5,
    INTERRUPT_ACTION_BEFORE_C6,
    INTERRUPT_ACTION_BEFORE_C7
};

struct interrupt_boundary_case {
    const char *name;
    uint8_t bus_action;
    uint8_t entry_start_selects_nmi;
    uint8_t mapped_t5_selects_nmi;
    uint8_t external_c6_selects_nmi;
    const char *source_pin;
    const char *source_phase;
};

static const struct interrupt_boundary_case interrupt_boundary_cases[] = {
    {"stable-through-C4", INTERRUPT_ACTION_BEFORE_C4, 0u, 1u, 1u,
     "Visual6502wiki/6502_Interrupt_Hijacking revision 19268",
     "NMI low by end T4; stage 1 available at T5 phase 1"},
    {"stable-before-C6", INTERRUPT_ACTION_BEFORE_C6, 0u, 0u, 1u,
     "Visual6502wiki/6502_Interrupt_Hijacking revision 19268",
     "after T5 choice but before external C6 vector-low consequence"}};

#define INTERRUPT_BRK_OVERLAP_SAMPLES 32u
#define INTERRUPT_IRQ_OVERLAP_SAMPLES 64u

/* AccuracyCoin 6bee919529289c555803b612b8c9e4f7eaaeeabb, TEST_NmiAndBrk:
   32 answer entries plus source-order byte 32 for the source's +1 compare. */
static const uint8_t interrupt_brk_answer_nmi[INTERRUPT_BRK_OVERLAP_SAMPLES + 1u] = {
    0xa4u, 0xa4u, 0xa4u, 0xa4u, 0xa4u, 0xa4u, 0xa4u, 0xa4u, 0xa4u, 0xa4u, 0x36u,
    0x36u, 0x36u, 0x36u, 0x36u, 0x36u, 0x36u, 0x36u, 0x36u, 0x36u, 0x36u, 0x36u,
    0x36u, 0x36u, 0x36u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x36u};

static const uint8_t interrupt_brk_answer_lower[INTERRUPT_BRK_OVERLAP_SAMPLES + 1u] = {
    0x36u, 0x36u, 0x36u, 0x36u, 0x36u, 0x36u, 0x36u, 0x36u, 0x36u, 0x36u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x36u, 0x36u, 0x36u, 0x36u, 0x36u, 0x36u, 0x36u, 0xa9u};

/* AccuracyCoin TEST_NmiAndIrq: 64 answer entries plus source-order byte 64
   for the source's paired +1 compare. */
static const uint8_t interrupt_irq_answer_nmi[INTERRUPT_IRQ_OVERLAP_SAMPLES + 1u] = {
    0xa4u, 0xa4u, 0xa4u, 0xa4u, 0xa4u, 0xa4u, 0xa4u, 0xa4u, 0xa4u, 0xa4u, 0x26u, 0x26u, 0x26u,
    0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u,
    0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u,
    0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u,
    0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u};

static const uint8_t interrupt_irq_answer_lower[INTERRUPT_IRQ_OVERLAP_SAMPLES + 1u] = {
    0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
    0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x26u, 0x26u, 0x26u,
    0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u,
    0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x26u, 0x78u};

enum interrupt_table_alignment {
    INTERRUPT_TABLE_INVALID = 0,
    INTERRUPT_TABLE_EXACT,
    INTERRUPT_TABLE_SHIFTED,
    INTERRUPT_TABLE_MIXED
};

static inline enum interrupt_table_alignment
interrupt_table_alignment_of(const uint8_t *observed, const uint8_t *answer, size_t count)
{
    if (memcmp(observed, answer, count) == 0)
        return INTERRUPT_TABLE_EXACT;
    if (memcmp(observed, answer + 1u, count) == 0)
        return INTERRUPT_TABLE_SHIFTED;
    return INTERRUPT_TABLE_INVALID;
}

/* AccuracyCoin permits one whole-table choice shared by both handlers. */
static inline enum interrupt_table_alignment
interrupt_paired_table_alignment(const uint8_t *observed_nmi, const uint8_t *observed_lower,
                                 const uint8_t *answer_nmi, const uint8_t *answer_lower,
                                 size_t count)
{
    enum interrupt_table_alignment nmi =
        interrupt_table_alignment_of(observed_nmi, answer_nmi, count);
    enum interrupt_table_alignment lower =
        interrupt_table_alignment_of(observed_lower, answer_lower, count);
    if (nmi == INTERRUPT_TABLE_EXACT && lower == INTERRUPT_TABLE_EXACT)
        return INTERRUPT_TABLE_EXACT;
    if (nmi == INTERRUPT_TABLE_SHIFTED && lower == INTERRUPT_TABLE_SHIFTED)
        return INTERRUPT_TABLE_SHIFTED;
    if ((nmi == INTERRUPT_TABLE_EXACT && lower == INTERRUPT_TABLE_SHIFTED) ||
        (nmi == INTERRUPT_TABLE_SHIFTED && lower == INTERRUPT_TABLE_EXACT))
        return INTERRUPT_TABLE_MIXED;
    return INTERRUPT_TABLE_INVALID;
}

/* Build an owned NROM with independently visible NMI and IRQ handlers.
   The handlers live at fixed addresses so test expectations can name the
   vector pair and marker locations without relying on observed output. */
static inline struct nesturbator *
interrupt_fixture_build(const uint8_t *program, size_t program_len, const uint8_t *nmi_handler,
                        size_t nmi_handler_len, const uint8_t *irq_handler, size_t irq_handler_len)
{
    static uint8_t image[INTERRUPT_FIXTURE_IMAGE_CAP];
    static const uint8_t idle_program[] = {0x4cu, 0x00u, 0x80u}; /* JMP $8000 */
    struct ines_segment segments[2];
    struct ines_spec spec;
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    size_t size;

    CHECK(nmi_handler != NULL && nmi_handler_len != 0u);
    CHECK(irq_handler != NULL && irq_handler_len != 0u);
    if (nmi_handler == NULL || nmi_handler_len == 0u || irq_handler == NULL ||
        irq_handler_len == 0u)
        return NULL;

    memset(&spec, 0, sizeof spec);
    segments[0].prg_offset = INTERRUPT_FIXTURE_NMI_VECTOR - 0x8000u;
    segments[0].bytes = nmi_handler;
    segments[0].len = nmi_handler_len;
    segments[1].prg_offset = INTERRUPT_FIXTURE_IRQ_VECTOR - 0x8000u;
    segments[1].bytes = irq_handler;
    segments[1].len = irq_handler_len;
    spec.prg_16k = 1u;
    spec.chr_8k = 1u;
    spec.prg_code = program != NULL ? program : idle_program;
    spec.prg_code_len = program != NULL ? program_len : sizeof idle_program;
    spec.segments = segments;
    spec.segment_count = 2u;
    spec.nmi_vector = INTERRUPT_FIXTURE_NMI_VECTOR;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = INTERRUPT_FIXTURE_IRQ_VECTOR;
    size = ines_build(image, sizeof image, &spec);
    CHECK(size != 0u);
    if (size == 0u)
        return NULL;

    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    CHECK(inst != NULL);
    if (inst != NULL)
        CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, size), NESTURBATOR_OK);
    return (struct nesturbator *)inst;
}

static inline void interrupt_fixture_start(struct nesturbator *nes, uint16_t pc, uint8_t status,
                                           uint32_t frame_cycle)
{
    nes->cpu.pc = pc;
    nes->cpu.s = 0xfdu;
    nes->cpu.p = status;
    nes->cpu.a = 0u;
    nes->cpu.x = 0u;
    nes->cpu.y = 0u;
    nes->cpu.nmi_prev = 0u;
    nes->cpu.nmi_pending = 0u;
    nes->cpu.nmi_polled = 0u;
    nes->cpu.irq_line = 0u;
    nes->cpu.irq_captured = 0u;
    nes->cpu.poll_latch = 0u;
    nes->apu.frame_mode = 0u;
    nes->apu.frame_cycle = frame_cycle;
    nes->apu.frame_reset_delay = 0u;
    nes->apu.frame_irq = 0u;
    nes->apu.frame_irq_inhibit = 0u;
    nes->apu.frame_irq_clear_pending = 0u;
    nes->apu.dmc.irq = 0u;
    nesturbator__irq_update(nes);
}

/* A reachable audio warm-up program followed by a quiet loop, a BRK site,
   and a return loop. Tests execute the setup as ordinary instructions so
   bus/APU phases and synthesizer history stay aligned. */
static const uint8_t interrupt_fixture_entry_program[0x26] = {
    [0x00] = 0x78u,                                 /* SEI */
    [0x01] = 0xa9u, [0x02] = 0x01u,                 /* LDA #$01 */
    [0x03] = 0x8du, [0x04] = 0x15u, [0x05] = 0x40u, /* STA $4015 */
    [0x06] = 0xa9u, [0x07] = 0xdfu,                 /* LDA #$DF: audible pulse 1 */
    [0x08] = 0x8du, [0x09] = 0x00u, [0x0a] = 0x40u, /* STA $4000 */
    [0x0b] = 0xa9u, [0x0c] = 0xfdu,                 /* LDA #$FD */
    [0x0d] = 0x8du, [0x0e] = 0x02u, [0x0f] = 0x40u, /* STA $4002 */
    [0x10] = 0xa9u, [0x11] = 0x08u,                 /* LDA #$08 */
    [0x12] = 0x8du, [0x13] = 0x03u, [0x14] = 0x40u, /* STA $4003 */
    [0x15] = 0xeau,                                 /* NOP */
    [0x16] = 0x4cu, [0x17] = 0x15u, [0x18] = 0x80u, /* JMP $8015 */
    [0x20] = 0x00u, [0x21] = 0xeau,                 /* BRK, padding */
    [0x22] = 0xeau,                                 /* NOP return target */
    [0x23] = 0x4cu, [0x24] = 0x22u, [0x25] = 0x80u  /* JMP $8022 */
};

static const uint8_t interrupt_fixture_entry_nmi_handler[] = {
    0xe6u, INTERRUPT_FIXTURE_NMI_COUNT, 0x40u /* INC marker; RTI */
};

static const uint8_t interrupt_fixture_entry_irq_handler[] = {
    0xe6u, INTERRUPT_FIXTURE_IRQ_COUNT, 0x40u /* INC marker; RTI */
};

static inline struct nesturbator *interrupt_fixture_entry_build(void)
{
    return interrupt_fixture_build(
        interrupt_fixture_entry_program, sizeof interrupt_fixture_entry_program,
        interrupt_fixture_entry_nmi_handler, sizeof interrupt_fixture_entry_nmi_handler,
        interrupt_fixture_entry_irq_handler, sizeof interrupt_fixture_entry_irq_handler);
}

static inline void interrupt_fixture_entry_warm(struct nesturbator *nes, unsigned instructions)
{
    interrupt_fixture_start(nes, 0x8000u, 0x24u, 0u);
    nes->bus.ram[INTERRUPT_FIXTURE_IRQ_COUNT] = 0u;
    nes->bus.ram[INTERRUPT_FIXTURE_NMI_COUNT] = 0u;
    for (unsigned i = 0u; i < instructions; ++i)
        nesturbator__cpu_step(nes);
    /* Keep the long future-frame replay free of a second level IRQ. This is
       a real production register write at the naturally reached bus/APU
       phase; the normal-initiation case reenables the frame source explicitly. */
    nesturbator__bus_write(nes, 0x4017u, 0x40u);
}

/* Construct a labeled entry origin for test sweeps. BRK's opcode is read
   from the production bus before the shared entry helper starts at C2;
   IRQ/NMI starts name the accepted history that the helper consumes. No
   stack write or entry stage is manufactured. Ordinary cpu_step initiation
   is separately checked by core.interrupt_state. */
static inline int interrupt_fixture_entry_construct(struct nesturbator *nes,
                                                    enum nesturbator_entry_origin origin)
{
    if (nes == NULL)
        return 0;
    if (origin == NESTURBATOR_ENTRY_BRK) {
        uint8_t opcode;
        nes->cpu.pc = INTERRUPT_FIXTURE_ENTRY_PC;
        opcode = nesturbator__bus_read(nes, nes->cpu.pc);
        if (opcode != 0x00u)
            return 0;
        nes->cpu.pc = (uint16_t)(nes->cpu.pc + 1u);
    } else if (origin == NESTURBATOR_ENTRY_IRQ) {
        nes->cpu.pc = INTERRUPT_FIXTURE_ENTRY_RETURN_PC;
        nes->cpu.irq_accepted = 1u;
    } else if (origin == NESTURBATOR_ENTRY_NMI) {
        nes->cpu.pc = INTERRUPT_FIXTURE_ENTRY_RETURN_PC;
        nes->cpu.nmi_pending = 1u;
    } else {
        return 0;
    }
    nesturbator__cpu_entry_begin(nes, origin);
    return nes->cpu.entry_origin == (uint8_t)origin;
}

/* Advance with the production one-action entry helper. The returned state
   is a compatible cutpoint serialized only through the public native API;
   Plan 21-07 host tests can reuse the same ROM and buffer contract. */
static inline int interrupt_fixture_entry_reach(struct nesturbator *nes, uint8_t next_action)
{
    while (nes != NULL && nes->cpu.entry_origin != NESTURBATOR_ENTRY_NONE &&
           nes->cpu.entry_next_action < next_action) {
        if (nesturbator__cpu_entry_action(nes) == 0u)
            return 0;
    }
    return nes != NULL && nes->cpu.entry_origin != NESTURBATOR_ENTRY_NONE &&
           nes->cpu.entry_next_action == next_action;
}

static inline int interrupt_fixture_entry_save_cutpoint(struct nesturbator *nes,
                                                        uint8_t next_action, void *buffer,
                                                        size_t capacity)
{
    return nesturbator_save_state((nesturbator *)nes, buffer, capacity) == NESTURBATOR_OK &&
           nes->cpu.entry_origin != NESTURBATOR_ENTRY_NONE &&
           nes->cpu.entry_next_action == next_action;
}

#endif /* NESTURBATOR_TESTS_INTERRUPT_FIXTURE_H */

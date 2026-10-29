/* INT-02 native entry continuation: constructed-origin cutpoints are kept
   distinct from ordinary cpu_step initiation, and each resumed bus action
   delegates to the production bus implementation. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../interrupt_fixture.h"

#define ENTRY_TRACE_CAPACITY 16u
#define ENTRY_TRANSITION_CAPACITY 8192u

struct entry_bus_access {
    uint16_t address;
    uint8_t value;
    uint8_t kind;
    uint64_t ticks_before;
    uint64_t ticks_after;
    uint64_t cpu_cycle_before;
    uint64_t cpu_cycle_after;
};

struct entry_trace {
    struct entry_bus_access access[ENTRY_TRACE_CAPACITY];
    size_t count;
    uint8_t overflow;
};

struct transition_event {
    uint64_t cpu_cycle;
    int32_t level;
};

struct transition_trace {
    struct transition_event event[ENTRY_TRANSITION_CAPACITY];
    size_t count;
    uint8_t overflow;
};

struct frame_observation {
    uint16_t video[256u * 240u];
    int16_t audio[1024u];
    nesturbator_frame frame;
    struct transition_trace transitions;
};

static struct entry_trace observed_trace;
static uint8_t observe_bus;
static struct frame_observation frame_expected;
static struct frame_observation frame_replay;
static struct frame_observation frame_control;

uint8_t nesturbator__bus_read_production(struct nesturbator *nes, uint16_t address);
void nesturbator__bus_write_production(struct nesturbator *nes, uint16_t address, uint8_t value);

static void record_bus_access(struct nesturbator *nes, uint16_t address, uint8_t value,
                              uint8_t kind, uint64_t ticks_before, uint64_t cycle_before)
{
    if (observe_bus == 0u)
        return;
    if (observed_trace.count >= ENTRY_TRACE_CAPACITY) {
        observed_trace.overflow = 1u;
        return;
    }
    observed_trace.access[observed_trace.count++] = (struct entry_bus_access){
        address, value, kind, ticks_before, nes->ticks, cycle_before, nes->cpu_cycle};
}

uint8_t nesturbator__bus_read(struct nesturbator *nes, uint16_t address)
{
    uint64_t ticks_before = nes->ticks;
    uint64_t cycle_before = nes->cpu_cycle;
    uint8_t value = nesturbator__bus_read_production(nes, address);
    record_bus_access(nes, address, value, 0u, ticks_before, cycle_before);
    return value;
}

void nesturbator__bus_write(struct nesturbator *nes, uint16_t address, uint8_t value)
{
    uint64_t ticks_before = nes->ticks;
    uint64_t cycle_before = nes->cpu_cycle;
    nesturbator__bus_write_production(nes, address, value);
    record_bus_access(nes, address, value, 1u, ticks_before, cycle_before);
}

static void trace_reset(void)
{
    memset(&observed_trace, 0, sizeof observed_trace);
    observe_bus = 0u;
}

static struct entry_trace trace_cpu_step(struct nesturbator *nes)
{
    trace_reset();
    observe_bus = 1u;
    nesturbator__cpu_step(nes);
    observe_bus = 0u;
    return observed_trace;
}

static void record_transition(void *context, uint64_t cpu_cycle, int32_t level)
{
    struct transition_trace *trace = (struct transition_trace *)context;
    if (trace->count >= ENTRY_TRANSITION_CAPACITY) {
        trace->overflow = 1u;
        return;
    }
    trace->event[trace->count++] = (struct transition_event){cpu_cycle, level};
}

static void run_public_frame(struct nesturbator *nes, struct frame_observation *out)
{
    nesturbator_input input;
    memset(&input, 0, sizeof input);
    input.size = (uint32_t)sizeof input;
    memset(out, 0, sizeof *out);
    out->frame.size = (uint32_t)sizeof out->frame;
    out->frame.video = out->video;
    out->frame.video_pitch = 256u;
    out->frame.audio = out->audio;
    out->frame.audio_capacity = (uint32_t)(sizeof out->audio / sizeof out->audio[0]);
    nesturbator__apu_set_transition_sink(nes, record_transition, &out->transitions);
    CHECK_EQ_U64(nesturbator_set_input((nesturbator *)nes, &input), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_run_frame((nesturbator *)nes, &out->frame), NESTURBATOR_OK);
    nesturbator__apu_set_transition_sink(nes, NULL, NULL);
}

static int transitions_equal(const struct transition_trace *a, const struct transition_trace *b)
{
    if (a->count != b->count || a->overflow != 0u || b->overflow != 0u)
        return 0;
    for (size_t i = 0u; i < a->count; ++i)
        if (a->event[i].cpu_cycle != b->event[i].cpu_cycle ||
            a->event[i].level != b->event[i].level)
            return 0;
    return 1;
}

static void compare_future_frames(const struct nesturbator *expected_nes,
                                  const struct nesturbator *replay_nes,
                                  const struct frame_observation *expected,
                                  const struct frame_observation *replay, size_t state_size,
                                  uint8_t *expected_state, uint8_t *replay_state, const char *label)
{
    int nonzero_pcm = 0;
    CHECK_EQ_U64(expected->frame.audio_count, replay->frame.audio_count);
    CHECK_EQ_U64(expected->frame.frame_number, replay->frame.frame_number);
    CHECK_EQ_U64(expected->frame.ticks, replay->frame.ticks);
    CHECK(memcmp(expected->video, replay->video, sizeof expected->video) == 0);
    CHECK(memcmp(expected->audio, replay->audio,
                 (size_t)expected->frame.audio_count * sizeof expected->audio[0]) == 0);
    CHECK(transitions_equal(&expected->transitions, &replay->transitions));
    CHECK(expected->transitions.count != 0u);
    CHECK_EQ_U64(expected->transitions.overflow, 0u);
    CHECK_EQ_U64(replay->transitions.overflow, 0u);
    CHECK(expected->frame.audio_count != 0u);
    for (uint32_t i = 0u; i < expected->frame.audio_count; ++i)
        nonzero_pcm |= expected->audio[i] != 0;
    CHECK(nonzero_pcm != 0);
    CHECK_EQ_U64(expected_nes->bus.ram[INTERRUPT_FIXTURE_IRQ_COUNT],
                 replay_nes->bus.ram[INTERRUPT_FIXTURE_IRQ_COUNT]);
    CHECK_EQ_U64(expected_nes->bus.ram[INTERRUPT_FIXTURE_NMI_COUNT],
                 replay_nes->bus.ram[INTERRUPT_FIXTURE_NMI_COUNT]);
    CHECK((uint8_t)(expected_nes->bus.ram[INTERRUPT_FIXTURE_IRQ_COUNT] +
                    expected_nes->bus.ram[INTERRUPT_FIXTURE_NMI_COUNT]) != 0u);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)expected_nes, expected_state, state_size),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)replay_nes, replay_state, state_size),
                 NESTURBATOR_OK);
    CHECK(memcmp(expected_state, replay_state, state_size) == 0);
    printf("future frame %s: frame=%llu ticks=%llu pixels=equal PCM=%u/nonzero "
           "transitions=%lu/equal guest-markers=equal canonical=equal\n",
           label, (unsigned long long)expected->frame.frame_number,
           (unsigned long long)expected->frame.ticks, expected->frame.audio_count,
           (unsigned long)expected->transitions.count);
}

static int trace_equal(const struct entry_trace *a, const struct entry_trace *b)
{
    if (a->count != b->count || a->overflow != 0u || b->overflow != 0u)
        return 0;
    for (size_t i = 0u; i < a->count; ++i) {
        const struct entry_bus_access *left = &a->access[i];
        const struct entry_bus_access *right = &b->access[i];
        if (left->address != right->address || left->value != right->value ||
            left->kind != right->kind || left->ticks_before != right->ticks_before ||
            left->ticks_after != right->ticks_after ||
            left->cpu_cycle_before != right->cpu_cycle_before ||
            left->cpu_cycle_after != right->cpu_cycle_after)
            return 0;
    }
    return 1;
}

static void check_trace_equal(const struct entry_trace *a, const struct entry_trace *b,
                              const char *label)
{
    if (!trace_equal(a, b)) {
        size_t first = 0u;
        while (first < a->count && first < b->count &&
               memcmp(&a->access[first], &b->access[first], sizeof a->access[first]) == 0)
            first++;
        fprintf(stderr, "%s bus trace differs at action %lu (counts %lu/%lu)\n", label,
                (unsigned long)first, (unsigned long)a->count, (unsigned long)b->count);
        check_failures++;
    }
}

static int has_synthesis_history(const struct nesturbator *nes)
{
    int impulse_nonzero = 0;
    for (size_t i = 0u; i < NESTURBATOR_SYNTH_TAPS; ++i)
        impulse_nonzero |= nes->synth.impulse[i] != 0;
    if (!(nes->apu.sample_phase != 0u && nes->synth.mixed_level != 0 && impulse_nonzero &&
          nes->apu.sample_output == NULL && nes->synth.pcm_count == 0u))
        fprintf(stderr,
                "audio history phase=%u mixed=%d impulse=%u head=%u pulse-length=%u "
                "pulse-phase=%u sample-out=%p pcm=%u\n",
                nes->apu.sample_phase, nes->synth.mixed_level, (unsigned)impulse_nonzero,
                nes->synth.impulse_head, nes->apu.pulse[0].length, nes->apu.pulse[0].phase,
                (void *)nes->apu.sample_output, nes->synth.pcm_count);
    return nes->apu.sample_phase != 0u && nes->synth.mixed_level != 0 && impulse_nonzero &&
           nes->apu.sample_output == NULL && nes->synth.pcm_count == 0u;
}

static struct nesturbator *build_warm_fixture(unsigned warm_instructions)
{
    struct nesturbator *nes = interrupt_fixture_entry_build();
    CHECK(nes != NULL);
    if (nes == NULL)
        return NULL;
    interrupt_fixture_entry_warm(nes, warm_instructions);
    nes->cpu.p = 0x20u; /* architectural test input; timing remains naturally advanced */
    return nes;
}

static uint8_t first_action(enum nesturbator_entry_origin origin)
{
    return origin == NESTURBATOR_ENTRY_BRK ? 2u : 1u;
}

static uint8_t expected_vector(enum nesturbator_entry_origin origin)
{
    return origin == NESTURBATOR_ENTRY_NMI ? NESTURBATOR_ENTRY_VECTOR_NMI
                                           : NESTURBATOR_ENTRY_VECTOR_IRQ;
}

static void check_completed_stack(const struct nesturbator *nes,
                                  enum nesturbator_entry_origin origin,
                                  enum nesturbator_entry_vector vector, uint16_t return_pc)
{
    CHECK_EQ_HEX(nes->cpu.pc, vector == NESTURBATOR_ENTRY_VECTOR_NMI
                                  ? INTERRUPT_FIXTURE_NMI_VECTOR
                                  : INTERRUPT_FIXTURE_IRQ_VECTOR);
    CHECK_EQ_U64(nes->cpu.s, 0xfau);
    CHECK_EQ_HEX(nes->bus.ram[0x01fdu], (uint8_t)(return_pc >> 8));
    CHECK_EQ_HEX(nes->bus.ram[0x01fcu], (uint8_t)return_pc);
    CHECK_EQ_HEX(nes->bus.ram[0x01fbu], origin == NESTURBATOR_ENTRY_BRK ? 0x30u : 0x20u);
    CHECK_EQ_HEX(nes->cpu.p, 0x24u);
    CHECK_EQ_U64(nes->cpu.entry_origin, NESTURBATOR_ENTRY_NONE);
    CHECK_EQ_U64(nes->cpu.entry_next_action, 0u);
    CHECK_EQ_U64(nes->cpu.entry_vector, NESTURBATOR_ENTRY_VECTOR_NONE);
    CHECK_EQ_U64(nes->cpu.entry_vector_low, 0u);
}

static void check_full_entry_trace(const struct entry_trace *trace,
                                   enum nesturbator_entry_origin origin, uint16_t return_pc,
                                   enum nesturbator_entry_vector vector, uint64_t ticks_before,
                                   uint64_t cycles_before)
{
    uint8_t stacked_p = origin == NESTURBATOR_ENTRY_BRK ? 0x30u : 0x20u;
    uint16_t vector_address = vector == NESTURBATOR_ENTRY_VECTOR_NMI ? 0xfffau : 0xfffeu;
    uint8_t vector_low = vector == NESTURBATOR_ENTRY_VECTOR_NMI ? 0x34u : 0x00u;
    uint8_t vector_high = vector == NESTURBATOR_ENTRY_VECTOR_NMI ? 0x81u : 0x82u;
    CHECK_EQ_U64(trace->count, 7u);
    CHECK_EQ_U64(trace->overflow, 0u);
    if (trace->count != 7u)
        return;
    CHECK_EQ_U64(trace->access[0].kind, 0u);
    CHECK_EQ_HEX(trace->access[0].address,
                 origin == NESTURBATOR_ENTRY_BRK ? INTERRUPT_FIXTURE_ENTRY_PC : return_pc);
    CHECK_EQ_U64(trace->access[1].kind, 0u);
    CHECK_EQ_HEX(trace->access[1].address,
                 origin == NESTURBATOR_ENTRY_BRK ? INTERRUPT_FIXTURE_ENTRY_PC + 1u : return_pc);
    CHECK_EQ_U64(trace->access[2].kind, 1u);
    CHECK_EQ_HEX(trace->access[2].address, 0x01fdu);
    CHECK_EQ_HEX(trace->access[2].value, (uint8_t)(return_pc >> 8));
    CHECK_EQ_U64(trace->access[3].kind, 1u);
    CHECK_EQ_HEX(trace->access[3].address, 0x01fcu);
    CHECK_EQ_HEX(trace->access[3].value, (uint8_t)return_pc);
    CHECK_EQ_U64(trace->access[4].kind, 1u);
    CHECK_EQ_HEX(trace->access[4].address, 0x01fbu);
    CHECK_EQ_HEX(trace->access[4].value, stacked_p);
    CHECK_EQ_U64(trace->access[5].kind, 0u);
    CHECK_EQ_HEX(trace->access[5].address, vector_address);
    CHECK_EQ_HEX(trace->access[5].value, vector_low);
    CHECK_EQ_U64(trace->access[6].kind, 0u);
    CHECK_EQ_HEX(trace->access[6].address, (uint16_t)(vector_address + 1u));
    CHECK_EQ_HEX(trace->access[6].value, vector_high);
    CHECK_EQ_U64(trace->access[6].ticks_after - ticks_before, 7u * 24u);
    CHECK_EQ_U64(trace->access[6].cpu_cycle_after - cycles_before, 7u);
}

static void test_ordinary_cpu_step_initiates_each_origin(void)
{
    struct nesturbator *brk = build_warm_fixture(80u);
    struct nesturbator *irq = build_warm_fixture(80u);
    struct nesturbator *nmi = build_warm_fixture(80u);
    struct entry_trace trace;
    uint64_t ticks_before;
    uint64_t cycles_before;
    uint16_t return_pc;

    CHECK(brk != NULL && irq != NULL && nmi != NULL);
    if (brk == NULL || irq == NULL || nmi == NULL)
        goto done;

    brk->cpu.pc = INTERRUPT_FIXTURE_ENTRY_PC;
    ticks_before = brk->ticks;
    cycles_before = brk->cpu_cycle;
    trace = trace_cpu_step(brk);
    check_full_entry_trace(&trace, NESTURBATOR_ENTRY_BRK, INTERRUPT_FIXTURE_ENTRY_RETURN_PC,
                           NESTURBATOR_ENTRY_VECTOR_IRQ, ticks_before, cycles_before);
    CHECK_EQ_U64(brk->ticks - ticks_before, 7u * 24u);
    CHECK_EQ_U64(brk->cpu_cycle - cycles_before, 7u);
    check_completed_stack(brk, NESTURBATOR_ENTRY_BRK, NESTURBATOR_ENTRY_VECTOR_IRQ,
                          INTERRUPT_FIXTURE_ENTRY_RETURN_PC);
    printf("normal initiation origin=BRK mechanism=cpu_step bus-actions=%lu vector=$%04X\n",
           (unsigned long)trace.count, brk->cpu.pc);

    /* Let the production frame counter and ordinary instruction poll create
       accepted IRQ history; do not seed the frame-cycle phase. */
    irq->cpu.p = 0x20u;
    nesturbator__bus_write(irq, 0x4017u, 0x00u); /* production write reenables frame IRQ */
    unsigned steps = 0u;
    while (irq->cpu.irq_accepted == 0u && steps < 50000u) {
        nesturbator__cpu_step(irq);
        steps++;
    }
    CHECK(steps < 50000u);
    CHECK_EQ_U64(irq->apu.frame_irq, 1u);
    CHECK_EQ_U64(irq->cpu.irq_accepted, 1u);
    return_pc = irq->cpu.pc;
    ticks_before = irq->ticks;
    cycles_before = irq->cpu_cycle;
    trace = trace_cpu_step(irq);
    check_full_entry_trace(&trace, NESTURBATOR_ENTRY_IRQ, return_pc, NESTURBATOR_ENTRY_VECTOR_IRQ,
                           ticks_before, cycles_before);
    CHECK_EQ_U64(irq->cpu.irq_accepted, 0u);
    check_completed_stack(irq, NESTURBATOR_ENTRY_IRQ, NESTURBATOR_ENTRY_VECTOR_IRQ, return_pc);
    printf("normal initiation origin=IRQ mechanism=cpu_step frame-counter=production-poll "
           "bus-actions=%lu vector=$%04X\n",
           (unsigned long)trace.count, irq->cpu.pc);

    /* The fixture supplies a constructed /NMI pin transition. The CPU latch
       and poll result are both generated by an ordinary guest instruction. */
    nmi->ppu.status |= 0x80u;
    nmi->ppu.control |= 0x80u;
    nesturbator__cpu_step(nmi);
    CHECK_EQ_U64(nmi->cpu.nmi_pending, 1u);
    CHECK_EQ_U64(nmi->cpu.nmi_polled, 1u);
    return_pc = nmi->cpu.pc;
    ticks_before = nmi->ticks;
    cycles_before = nmi->cpu_cycle;
    trace = trace_cpu_step(nmi);
    check_full_entry_trace(&trace, NESTURBATOR_ENTRY_NMI, return_pc, NESTURBATOR_ENTRY_VECTOR_NMI,
                           ticks_before, cycles_before);
    CHECK_EQ_U64(nmi->cpu.nmi_pending, 0u);
    CHECK_EQ_U64(nmi->cpu.nmi_polled, 0u);
    check_completed_stack(nmi, NESTURBATOR_ENTRY_NMI, NESTURBATOR_ENTRY_VECTOR_NMI, return_pc);
    printf("normal initiation origin=NMI mechanism=cpu_step pin-capture-poll "
           "bus-actions=%lu vector=$%04X\n",
           (unsigned long)trace.count, nmi->cpu.pc);

done:
    nesturbator_destroy((nesturbator *)brk);
    nesturbator_destroy((nesturbator *)irq);
    nesturbator_destroy((nesturbator *)nmi);
}

static void verify_saved_cutpoint(struct nesturbator *source, enum nesturbator_entry_origin origin,
                                  uint8_t next_action, uint8_t vector, const char *label)
{
    struct nesturbator *restored = NULL;
    uint8_t *state = NULL;
    uint8_t *roundtrip = NULL;
    size_t state_size = 0u;
    uint8_t stage_vector = next_action < 5u && origin != NESTURBATOR_ENTRY_NMI
                               ? NESTURBATOR_ENTRY_VECTOR_NONE
                               : vector;
    struct entry_trace expected;
    struct entry_trace replay;
    uint64_t ticks_before;
    uint64_t cycles_before;
    uint8_t pending_before;

    CHECK(has_synthesis_history(source));
    CHECK_EQ_U64(source->apu.sample_output == NULL, 1u);
    CHECK_EQ_U64(source->synth.pcm_count, 0u);
    CHECK_EQ_U64(source->cpu.entry_origin, origin);
    CHECK_EQ_U64(source->cpu.entry_next_action, next_action);
    pending_before = source->cpu.nmi_pending;
    /* Recognized requests are sticky. An inhibited candidate released with
       an entry cycle still ahead is canceled before handler recognition. */
    if (pending_before == NESTURBATOR_NMI_INHIBITED && nesturbator__ppu_nmi_line(source) == 0u)
        pending_before = NESTURBATOR_NMI_NONE;
    if (next_action < 5u && origin != NESTURBATOR_ENTRY_NMI)
        CHECK_EQ_U64(source->cpu.entry_vector, NESTURBATOR_ENTRY_VECTOR_NONE);
    if (next_action >= 5u || origin == NESTURBATOR_ENTRY_NMI)
        CHECK_EQ_U64(source->cpu.entry_vector, vector);
    if (next_action == 7u)
        CHECK_EQ_U64(source->cpu.entry_vector_low,
                     vector == NESTURBATOR_ENTRY_VECTOR_NMI ? 0x34u : 0x00u);
    CHECK(has_synthesis_history(source));

    state_size = nesturbator_state_size((nesturbator *)source);
    state = (uint8_t *)malloc(state_size);
    roundtrip = (uint8_t *)malloc(state_size);
    CHECK(state_size != 0u && state != NULL && roundtrip != NULL);
    if (state_size == 0u || state == NULL || roundtrip == NULL)
        goto done;
    CHECK(interrupt_fixture_entry_save_cutpoint(source, next_action, state, state_size));

    /* A fresh instance loads the same synthetic ROM bytes, then only the
       public native loader restores machine state. */
    restored = interrupt_fixture_entry_build();
    CHECK(restored != NULL);
    if (restored == NULL)
        goto done;
    CHECK_EQ_U64(nesturbator_state_size((nesturbator *)restored), state_size);
    CHECK_EQ_U64(nesturbator_load_state((nesturbator *)restored, state, state_size),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)restored, roundtrip, state_size),
                 NESTURBATOR_OK);
    CHECK(memcmp(state, roundtrip, state_size) == 0);
    CHECK_EQ_U64(restored->cpu.entry_origin, origin);
    CHECK_EQ_U64(restored->cpu.entry_next_action, next_action);
    CHECK_EQ_U64(restored->cpu.entry_vector, stage_vector);
    CHECK_EQ_HEX(restored->cpu.pc, source->cpu.pc);
    CHECK_EQ_HEX(restored->cpu.p, source->cpu.p);
    CHECK_EQ_HEX(restored->cpu.s, source->cpu.s);
    CHECK_EQ_HEX(restored->bus.ram[0x01fdu], source->bus.ram[0x01fdu]);
    CHECK_EQ_HEX(restored->bus.ram[0x01fcu], source->bus.ram[0x01fcu]);
    CHECK_EQ_HEX(restored->bus.ram[0x01fbu], source->bus.ram[0x01fbu]);
    CHECK(has_synthesis_history(restored));

    ticks_before = source->ticks;
    cycles_before = source->cpu_cycle;
    expected = trace_cpu_step(source); /* uninterrupted origin-to-completion path */
    CHECK_EQ_U64(source->ticks - ticks_before, expected.count * 24u);
    CHECK_EQ_U64(source->cpu_cycle - cycles_before, expected.count);
    replay = trace_cpu_step(restored); /* ordinary cpu_step resumes loaded history */
    check_trace_equal(&expected, &replay, "native interrupt continuation");
    CHECK_EQ_U64(expected.count, (size_t)(8u - next_action));
    CHECK_EQ_U64(restored->ticks - ticks_before, expected.count * 24u);
    CHECK_EQ_U64(restored->cpu_cycle - cycles_before, expected.count);
    CHECK_EQ_U64(restored->cpu.pc, source->cpu.pc);
    CHECK_EQ_U64(restored->cpu.p, source->cpu.p);
    CHECK_EQ_U64(restored->cpu.s, source->cpu.s);
    CHECK_EQ_U64(source->cpu.nmi_pending, pending_before);
    CHECK_EQ_U64(restored->cpu.nmi_pending, source->cpu.nmi_pending);
    CHECK_EQ_U64(restored->bus.ram[0x01fdu], source->bus.ram[0x01fdu]);
    CHECK_EQ_U64(restored->bus.ram[0x01fcu], source->bus.ram[0x01fcu]);
    CHECK_EQ_U64(restored->bus.ram[0x01fbu], source->bus.ram[0x01fbu]);
    CHECK_EQ_U64(source->cpu.nmi_pending, pending_before);
    CHECK_EQ_U64(restored->cpu.nmi_pending, pending_before);
    check_completed_stack(source, origin, (enum nesturbator_entry_vector)vector,
                          INTERRUPT_FIXTURE_ENTRY_RETURN_PC);
    check_completed_stack(restored, origin, (enum nesturbator_entry_vector)vector,
                          INTERRUPT_FIXTURE_ENTRY_RETURN_PC);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)source, state, state_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)restored, roundtrip, state_size),
                 NESTURBATOR_OK);
    CHECK(memcmp(state, roundtrip, state_size) == 0);

    run_public_frame(source, &frame_expected);
    run_public_frame(restored, &frame_replay);
    compare_future_frames(source, restored, &frame_expected, &frame_replay, state_size, state,
                          roundtrip, label);

    printf("native cut %s stage=next-C%u resumed-actions=%lu save-load-save=equal "
           "exact-trace=equal PC=$%04X P=$%02X S=$%02X ticks=%llu\n",
           label, (unsigned)next_action, (unsigned long)expected.count, source->cpu.pc,
           source->cpu.p, source->cpu.s, (unsigned long long)source->ticks);

done:
    free(state);
    free(roundtrip);
    nesturbator_destroy((nesturbator *)restored);
}

static void test_native_cutpoint(enum nesturbator_entry_origin origin, uint8_t next_action)
{
    struct nesturbator *source = build_warm_fixture(600u);
    uint8_t first = first_action(origin);
    uint8_t vector = expected_vector(origin);

    CHECK(source != NULL);
    if (source == NULL)
        return;
    trace_reset();
    observe_bus = 1u;
    CHECK(interrupt_fixture_entry_construct(source, origin));
    observe_bus = 0u;
    size_t origin_accesses = observed_trace.count;
    CHECK_EQ_U64(origin_accesses, origin == NESTURBATOR_ENTRY_BRK ? 1u : 0u);
    if (origin == NESTURBATOR_ENTRY_BRK) {
        CHECK_EQ_U64(observed_trace.access[0].kind, 0u);
        CHECK_EQ_U64(observed_trace.access[0].address, INTERRUPT_FIXTURE_ENTRY_PC);
    }
    if (origin == NESTURBATOR_ENTRY_IRQ)
        CHECK_EQ_U64(source->cpu.irq_accepted, 0u);
    if (origin == NESTURBATOR_ENTRY_NMI)
        CHECK_EQ_U64(source->cpu.nmi_pending, 0u);
    CHECK_EQ_U64(source->cpu.entry_next_action, first);

    trace_reset();
    observe_bus = 1u;
    CHECK(interrupt_fixture_entry_reach(source, next_action));
    observe_bus = 0u;
    CHECK_EQ_U64(observed_trace.count, (size_t)(next_action - first));
    uint8_t pushes = next_action <= 3u ? 0u : (uint8_t)(next_action - 3u);
    if (pushes > 3u)
        pushes = 3u;
    CHECK_EQ_U64(source->cpu.s, (uint8_t)(0xfdu - pushes));
    verify_saved_cutpoint(source, origin, next_action, vector,
                          origin == NESTURBATOR_ENTRY_BRK   ? "constructed-BRK"
                          : origin == NESTURBATOR_ENTRY_IRQ ? "constructed-IRQ"
                                                            : "constructed-NMI");
    nesturbator_destroy((nesturbator *)source);
}

static void test_all_native_entry_cutpoints(void)
{
    static const enum nesturbator_entry_origin origins[] = {
        NESTURBATOR_ENTRY_BRK, NESTURBATOR_ENTRY_IRQ, NESTURBATOR_ENTRY_NMI};
    for (size_t i = 0u; i < sizeof origins / sizeof origins[0]; ++i) {
        uint8_t first = first_action(origins[i]);
        for (uint8_t next_action = first; next_action <= 7u; ++next_action)
            test_native_cutpoint(origins[i], next_action);
    }
}

static struct nesturbator *build_constructed_cut(enum nesturbator_entry_origin origin,
                                                 uint8_t next_action, unsigned warm_instructions)
{
    struct nesturbator *nes = build_warm_fixture(warm_instructions);
    if (nes == NULL)
        return NULL;
    if (!interrupt_fixture_entry_construct(nes, origin) ||
        !interrupt_fixture_entry_reach(nes, next_action)) {
        CHECK(0);
        nesturbator_destroy((nesturbator *)nes);
        return NULL;
    }
    return nes;
}

static void assert_nmi_pin(struct nesturbator *nes)
{
    nes->ppu.status |= 0x80u;
    nes->ppu.control |= 0x80u;
}

static void test_coherent_selection_cutpoints(void)
{
    struct nesturbator *before = build_constructed_cut(NESTURBATOR_ENTRY_BRK, 4u, 600u);
    struct nesturbator *after = build_constructed_cut(NESTURBATOR_ENTRY_IRQ, 5u, 713u);
    struct nesturbator *between_reads = build_constructed_cut(NESTURBATOR_ENTRY_IRQ, 6u, 827u);
    struct nesturbator *released = build_constructed_cut(NESTURBATOR_ENTRY_IRQ, 5u, 900u);

    CHECK(before != NULL && after != NULL && between_reads != NULL && released != NULL);
    if (before == NULL || after == NULL || between_reads == NULL || released == NULL)
        goto done;

    /* The input edge is sampled by the completed production C4 stack write;
       it selects one coherent NMI pair before C5. */
    assert_nmi_pin(before);
    trace_reset();
    observe_bus = 1u;
    CHECK_EQ_U64(nesturbator__cpu_entry_action(before), 1u);
    observe_bus = 0u;
    CHECK_EQ_U64(before->cpu.entry_next_action, 5u);
    CHECK_EQ_U64(before->cpu.entry_vector, NESTURBATOR_ENTRY_VECTOR_NMI);
    CHECK_EQ_U64(before->cpu.nmi_pending, 0u);
    CHECK_EQ_U64(observed_trace.count, 1u);
    CHECK_EQ_U64(observed_trace.access[0].kind, 1u);
    CHECK_EQ_HEX(observed_trace.access[0].address, 0x01fcu);
    verify_saved_cutpoint(before, NESTURBATOR_ENTRY_BRK, 5u, NESTURBATOR_ENTRY_VECTOR_NMI,
                          "BRK pre-selection edge");

    /* Once the IRQ pair was selected after C4, an edge sampled on C5 stays
       an inhibited candidate for later recognition and cannot replace either
       vector byte. It is not yet a recognized sticky request. */
    CHECK_EQ_U64(after->cpu.entry_vector, NESTURBATOR_ENTRY_VECTOR_IRQ);
    assert_nmi_pin(after);
    trace_reset();
    observe_bus = 1u;
    CHECK_EQ_U64(nesturbator__cpu_entry_action(after), 1u);
    observe_bus = 0u;
    CHECK_EQ_U64(after->cpu.entry_next_action, 6u);
    CHECK_EQ_U64(after->cpu.entry_vector, NESTURBATOR_ENTRY_VECTOR_IRQ);
    CHECK_EQ_U64(after->cpu.nmi_pending, NESTURBATOR_NMI_INHIBITED);
    CHECK_EQ_HEX(observed_trace.access[0].address, 0x01fbu);
    verify_saved_cutpoint(after, NESTURBATOR_ENTRY_IRQ, 6u, NESTURBATOR_ENTRY_VECTOR_IRQ,
                          "IRQ post-selection edge");

    /* A later edge between vector reads is retained, while the high byte still
       comes from the pair whose low byte was already read. */
    CHECK_EQ_U64(between_reads->cpu.entry_vector, NESTURBATOR_ENTRY_VECTOR_IRQ);
    assert_nmi_pin(between_reads);
    trace_reset();
    observe_bus = 1u;
    CHECK_EQ_U64(nesturbator__cpu_entry_action(between_reads), 1u);
    observe_bus = 0u;
    CHECK_EQ_U64(between_reads->cpu.entry_next_action, 7u);
    CHECK_EQ_U64(between_reads->cpu.entry_vector, NESTURBATOR_ENTRY_VECTOR_IRQ);
    CHECK_EQ_U64(between_reads->cpu.entry_vector_low, 0u);
    CHECK_EQ_U64(between_reads->cpu.nmi_pending, NESTURBATOR_NMI_INHIBITED);
    CHECK_EQ_HEX(observed_trace.access[0].address, 0xfffeu);
    verify_saved_cutpoint(between_reads, NESTURBATOR_ENTRY_IRQ, 7u, NESTURBATOR_ENTRY_VECTOR_IRQ,
                          "IRQ between vector reads");

    assert_nmi_pin(released);
    CHECK_EQ_U64(nesturbator__cpu_entry_action(released), 1u); /* C5 samples high. */
    CHECK_EQ_U64(released->cpu.nmi_pending, NESTURBATOR_NMI_INHIBITED);
    released->ppu.control &= 0x7fu;
    verify_saved_cutpoint(released, NESTURBATOR_ENTRY_IRQ, 6u, NESTURBATOR_ENTRY_VECTOR_IRQ,
                          "IRQ inhibited input released");

done:
    nesturbator_destroy((nesturbator *)before);
    nesturbator_destroy((nesturbator *)after);
    nesturbator_destroy((nesturbator *)between_reads);
    nesturbator_destroy((nesturbator *)released);
}

static void test_restore_controls(void)
{
    struct nesturbator *irq_source = build_constructed_cut(NESTURBATOR_ENTRY_IRQ, 7u, 600u);
    struct nesturbator *nmi_source = build_constructed_cut(NESTURBATOR_ENTRY_NMI, 7u, 733u);
    struct nesturbator *good = NULL;
    struct nesturbator *wrong_origin = NULL;
    struct nesturbator *skipped_load = NULL;
    struct nesturbator *omitted_history = NULL;
    uint8_t *irq_state = NULL;
    uint8_t *nmi_state = NULL;
    uint8_t *scratch = NULL;
    size_t state_size = 0u;
    struct entry_trace expected;
    struct entry_trace observed;

    CHECK(irq_source != NULL && nmi_source != NULL);
    if (irq_source == NULL || nmi_source == NULL)
        goto done;
    state_size = nesturbator_state_size((nesturbator *)irq_source);
    irq_state = (uint8_t *)malloc(state_size);
    nmi_state = (uint8_t *)malloc(state_size);
    scratch = (uint8_t *)malloc(state_size);
    CHECK(state_size != 0u && irq_state != NULL && nmi_state != NULL && scratch != NULL);
    if (state_size == 0u || irq_state == NULL || nmi_state == NULL || scratch == NULL)
        goto done;

    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)irq_source, irq_state, state_size),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)nmi_source, nmi_state, state_size),
                 NESTURBATOR_OK);
    expected = trace_cpu_step(irq_source);
    CHECK_EQ_U64(expected.count, 1u);
    CHECK_EQ_HEX(expected.access[0].address, 0xffffu);

    good = interrupt_fixture_entry_build();
    CHECK(good != NULL);
    if (good == NULL)
        goto done;
    CHECK_EQ_U64(nesturbator_load_state((nesturbator *)good, irq_state, state_size),
                 NESTURBATOR_OK);
    observed = trace_cpu_step(good);
    check_trace_equal(&expected, &observed, "clean IRQ sibling");

    /* A valid NMI-origin C7 state is a wrong-origin substitute for this IRQ
       continuation; its first read names the NMI high-vector byte. */
    wrong_origin = interrupt_fixture_entry_build();
    CHECK(wrong_origin != NULL);
    if (wrong_origin == NULL)
        goto done;
    CHECK_EQ_U64(nesturbator_load_state((nesturbator *)wrong_origin, nmi_state, state_size),
                 NESTURBATOR_OK);
    observed = trace_cpu_step(wrong_origin);
    CHECK(!trace_equal(&expected, &observed));
    CHECK_EQ_HEX(observed.access[0].address, 0xfffbu);
    printf("restore control wrong-origin: expected-high=$%04X observed=$%04X clean-sibling=PASS\n",
           expected.access[0].address, observed.access[0].address);

    /* Skipping the load fetches the clean guest's first opcode instead of the
       pending vector byte. */
    skipped_load = interrupt_fixture_entry_build();
    CHECK(skipped_load != NULL);
    if (skipped_load == NULL)
        goto done;
    observed = trace_cpu_step(skipped_load);
    CHECK(!trace_equal(&expected, &observed));
    CHECK_EQ_HEX(observed.access[0].address, 0x8000u);
    printf("restore control skipped-load: expected-high=$%04X observed-opcode=$%04X "
           "clean-sibling=PASS\n",
           expected.access[0].address, observed.access[0].address);

    /* Load correctly, then deliberately omit only the serialized entry
       history. The clean state now fetches the return-site NOP at $8022. */
    omitted_history = interrupt_fixture_entry_build();
    CHECK(omitted_history != NULL);
    if (omitted_history == NULL)
        goto done;
    CHECK_EQ_U64(nesturbator_load_state((nesturbator *)omitted_history, irq_state, state_size),
                 NESTURBATOR_OK);
    omitted_history->cpu.entry_origin = NESTURBATOR_ENTRY_NONE;
    omitted_history->cpu.entry_next_action = 0u;
    omitted_history->cpu.entry_vector = NESTURBATOR_ENTRY_VECTOR_NONE;
    omitted_history->cpu.entry_vector_low = 0u;
    observed = trace_cpu_step(omitted_history);
    CHECK(!trace_equal(&expected, &observed));
    CHECK_EQ_HEX(observed.access[0].address, INTERRUPT_FIXTURE_ENTRY_RETURN_PC);
    printf("restore control omitted-entry-history: expected-high=$%04X observed-opcode=$%04X "
           "clean-sibling=PASS\n",
           expected.access[0].address, observed.access[0].address);

done:
    free(irq_state);
    free(nmi_state);
    free(scratch);
    nesturbator_destroy((nesturbator *)irq_source);
    nesturbator_destroy((nesturbator *)nmi_source);
    nesturbator_destroy((nesturbator *)good);
    nesturbator_destroy((nesturbator *)wrong_origin);
    nesturbator_destroy((nesturbator *)skipped_load);
    nesturbator_destroy((nesturbator *)omitted_history);
}

static void test_interleaved_instances(void)
{
    struct nesturbator *left = build_constructed_cut(NESTURBATOR_ENTRY_IRQ, 4u, 600u);
    struct nesturbator *right = build_constructed_cut(NESTURBATOR_ENTRY_NMI, 6u, 947u);
    struct nesturbator *left_restored = NULL;
    struct nesturbator *right_restored = NULL;
    uint8_t *left_state = NULL;
    uint8_t *right_state = NULL;
    uint8_t *scratch_a = NULL;
    uint8_t *scratch_b = NULL;
    size_t state_size = 0u;
    struct entry_trace left_expected;
    struct entry_trace right_expected;
    struct entry_trace left_replay;
    struct entry_trace right_replay;

    CHECK(left != NULL && right != NULL);
    if (left == NULL || right == NULL)
        goto done;
    CHECK(left->cpu.entry_origin != right->cpu.entry_origin);
    CHECK(left->cpu.entry_next_action != right->cpu.entry_next_action);
    CHECK(left->apu.sample_phase != right->apu.sample_phase);
    CHECK(has_synthesis_history(left));
    CHECK(has_synthesis_history(right));
    state_size = nesturbator_state_size((nesturbator *)left);
    left_state = (uint8_t *)malloc(state_size);
    right_state = (uint8_t *)malloc(state_size);
    scratch_a = (uint8_t *)malloc(state_size);
    scratch_b = (uint8_t *)malloc(state_size);
    CHECK(state_size != 0u && left_state != NULL && right_state != NULL && scratch_a != NULL &&
          scratch_b != NULL);
    if (state_size == 0u || left_state == NULL || right_state == NULL || scratch_a == NULL ||
        scratch_b == NULL)
        goto done;
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)left, left_state, state_size),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)right, right_state, state_size),
                 NESTURBATOR_OK);
    left_restored = interrupt_fixture_entry_build();
    right_restored = interrupt_fixture_entry_build();
    CHECK(left_restored != NULL && right_restored != NULL);
    if (left_restored == NULL || right_restored == NULL)
        goto done;
    CHECK_EQ_U64(nesturbator_load_state((nesturbator *)left_restored, left_state, state_size),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_state((nesturbator *)right_restored, right_state, state_size),
                 NESTURBATOR_OK);

    /* Complete two different histories in alternating instance order. */
    left_expected = trace_cpu_step(left);
    right_expected = trace_cpu_step(right);
    right_replay = trace_cpu_step(right_restored);
    left_replay = trace_cpu_step(left_restored);
    check_trace_equal(&right_expected, &right_replay, "interleaved NMI instance");
    check_trace_equal(&left_expected, &left_replay, "interleaved IRQ instance");
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)left, scratch_a, state_size),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)left_restored, scratch_b, state_size),
                 NESTURBATOR_OK);
    CHECK(memcmp(scratch_a, scratch_b, state_size) == 0);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)right, scratch_a, state_size),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)right_restored, scratch_b, state_size),
                 NESTURBATOR_OK);
    CHECK(memcmp(scratch_a, scratch_b, state_size) == 0);

    run_public_frame(left, &frame_expected);
    run_public_frame(right, &frame_replay);
    run_public_frame(right_restored, &frame_control);
    compare_future_frames(right, right_restored, &frame_replay, &frame_control, state_size,
                          scratch_a, scratch_b, "interleaved NMI instance");
    run_public_frame(left_restored, &frame_replay);
    compare_future_frames(left, left_restored, &frame_expected, &frame_replay, state_size,
                          scratch_a, scratch_b, "interleaved IRQ instance");
    printf("interleaved instances: origins=IRQ/NMI stages=C4/C6 sample-phases=%u/%u "
           "isolated-and-interleaved=equal\n",
           left->apu.sample_phase, right->apu.sample_phase);

done:
    free(left_state);
    free(right_state);
    free(scratch_a);
    free(scratch_b);
    nesturbator_destroy((nesturbator *)left);
    nesturbator_destroy((nesturbator *)right);
    nesturbator_destroy((nesturbator *)left_restored);
    nesturbator_destroy((nesturbator *)right_restored);
}

int interrupt_state_run(void)
{
    trace_reset();
    test_ordinary_cpu_step_initiates_each_origin();
    test_all_native_entry_cutpoints();
    test_coherent_selection_cutpoints();
    test_restore_controls();
    test_interleaved_instances();
    CHECK_DONE();
}

#ifndef NESTURBATOR_INTERRUPT_STATE_NO_MAIN
int main(void)
{
    return interrupt_state_run();
}
#endif

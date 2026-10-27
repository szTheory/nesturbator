/* KEEP-03: exact enabled-NMI set/clear and captured-before-poll replay. The
   fixture origins are explicitly constructed; every named endpoint is reached
   only through complete production cpu_step calls. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../nmi_fixture.h"
#include "../state_test.h"

#define NMI_COUNT 0x30u

static const uint8_t jmp_loop[] = {0x4cu, 0x00u, 0x80u};
static const uint8_t nmi_handler[] = {0xe6u, NMI_COUNT, 0x40u};

struct frame_result {
    uint16_t video[256u * 240u];
    int16_t audio[1024u];
    nesturbator_frame frame;
};

static void seed_audio(struct nesturbator *nes)
{
    nesturbator__apu_write(nes, 0x4011u, 64u);
    nesturbator__apu_write(nes, 0x4000u, 0x1fu);
    nesturbator__apu_write(nes, 0x4002u, 0x40u);
    nesturbator__apu_write(nes, 0x4003u, 0x08u);
    nesturbator__apu_write(nes, 0x4015u, 0x01u);
}

static struct nesturbator *build_fixture(uint16_t scanline, uint16_t dot, uint8_t status,
                                         uint8_t control, uint8_t nmi_prev)
{
    struct nesturbator *nes =
        nmi_fixture_build(jmp_loop, sizeof jmp_loop, nmi_handler, sizeof nmi_handler, 0x8200u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return NULL;

    nmi_fixture_at(nes, scanline, dot, status, control, 0u);
    nes->ppu.mask = 0u;
    nes->ppu.odd_frame = 0u;
    nes->cpu.pc = 0x8000u;
    nes->cpu.s = 0xfdu;
    nes->cpu.p = 0x24u;
    nes->cpu.a = 0u;
    nes->cpu.x = 0u;
    nes->cpu.y = 0u;
    nes->cpu.nmi_prev = nmi_prev;
    nes->cpu.nmi_pending = 0u;
    nes->cpu.nmi_polled = 0u;
    nes->bus.ram[NMI_COUNT] = 0u;
    seed_audio(nes);
    return nes;
}

static int has_synth_history(const struct nesturbator *nes)
{
    int64_t impulse_sum = 0;
    for (size_t i = 0u; i < NESTURBATOR_SYNTH_TAPS; ++i)
        impulse_sum += nes->synth.impulse[i];
    return nes->apu.sample_phase != 0u && nes->synth.mixed_level > 0 && impulse_sum != 0 &&
           nes->apu.sample_output == NULL && nes->synth.pcm_count == 0u;
}

static void save_pair_equal(struct nesturbator *a, struct nesturbator *b, size_t size,
                            uint8_t *a_state, uint8_t *b_state, const char *where)
{
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)a, a_state, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)b, b_state, size), NESTURBATOR_OK);
    if (memcmp(a_state, b_state, size) != 0) {
        fprintf(stderr, "NMI state mismatch at %s\n", where);
        check_failures++;
    }
}

static void run_frame(struct nesturbator *nes, struct frame_result *out)
{
    nesturbator_input input;
    memset(&input, 0, sizeof input);
    input.size = (uint32_t)sizeof input;
    CHECK_EQ_U64(nesturbator_set_input((nesturbator *)nes, &input), NESTURBATOR_OK);

    memset(out, 0, sizeof *out);
    out->frame.size = (uint32_t)sizeof out->frame;
    out->frame.video = out->video;
    out->frame.video_pitch = 256u;
    out->frame.audio = out->audio;
    out->frame.audio_capacity = (uint32_t)(sizeof out->audio / sizeof out->audio[0]);
    CHECK_EQ_U64(nesturbator_run_frame((nesturbator *)nes, &out->frame), NESTURBATOR_OK);
}

static void compare_frames(const struct frame_result *a, const struct frame_result *b)
{
    CHECK_EQ_U64(a->frame.audio_count, b->frame.audio_count);
    CHECK_EQ_U64(a->frame.frame_number, b->frame.frame_number);
    CHECK_EQ_U64(a->frame.ticks, b->frame.ticks);
    CHECK(memcmp(a->video, b->video, sizeof a->video) == 0);
    CHECK(memcmp(a->audio, b->audio, (size_t)a->frame.audio_count * sizeof a->audio[0]) == 0);
}

static int same_synth_history(const struct nesturbator *a, const struct nesturbator *b)
{
    return a->apu.sample_phase == b->apu.sample_phase &&
           a->synth.mixed_level == b->synth.mixed_level &&
           a->synth.impulse_head == b->synth.impulse_head &&
           memcmp(a->synth.impulse, b->synth.impulse, sizeof a->synth.impulse) == 0 &&
           a->synth.integrator_q15 == b->synth.integrator_q15 &&
           memcmp(a->synth.hp_input_q30, b->synth.hp_input_q30, sizeof a->synth.hp_input_q30) ==
               0 &&
           memcmp(a->synth.hp_output_q30, b->synth.hp_output_q30, sizeof a->synth.hp_output_q30) ==
               0 &&
           a->synth.lp_output_q30 == b->synth.lp_output_q30;
}

static void empty_drain_is_noop(struct nesturbator *nes, const uint8_t *canonical,
                                uint8_t *after_drain, size_t size, const char *label)
{
    const int16_t sentinel_value = (int16_t)0x5a5a;
    int16_t sentinel = sentinel_value;
    uint16_t pcm_read = nes->synth.pcm_read;
    uint16_t pcm_write = nes->synth.pcm_write;
    uint32_t drained;

    CHECK(nes->apu.sample_output == NULL);
    CHECK_EQ_U64(nes->synth.pcm_count, 0u);
    drained = nesturbator__synth_drain(nes, &sentinel, 1u);
    CHECK_EQ_U64(drained, 0u);
    CHECK(sentinel == sentinel_value);
    CHECK_EQ_U64(nes->synth.pcm_read, pcm_read);
    CHECK_EQ_U64(nes->synth.pcm_write, pcm_write);
    CHECK_EQ_U64(nes->synth.pcm_count, 0u);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)nes, after_drain, size), NESTURBATOR_OK);
    CHECK(memcmp(canonical, after_drain, size) == 0);
    printf("empty PCM drain %s: returned=0 sentinel=unchanged ring=empty canonical=unchanged\n",
           label);
}

static int pcm_diff_index(const struct frame_result *a, const struct frame_result *b,
                          uint32_t *first_difference)
{
    uint32_t common =
        a->frame.audio_count < b->frame.audio_count ? a->frame.audio_count : b->frame.audio_count;
    for (uint32_t i = 0u; i < common; ++i) {
        if (a->audio[i] != b->audio[i]) {
            *first_difference = i;
            return 1;
        }
    }
    if (a->frame.audio_count != b->frame.audio_count) {
        *first_difference = common;
        return 1;
    }
    return 0;
}

static void test_frame_zero_empty_pcm_preserves_history(void)
{
    struct nesturbator *source = build_fixture(240u, 333u, 0u, 0x80u, 0u);
    struct nesturbator *restored = build_fixture(240u, 333u, 0u, 0x80u, 0u);
    struct nesturbator *lost_history = build_fixture(240u, 333u, 0u, 0x80u, 0u);
    struct frame_result poisoned_frame;
    struct frame_result source_frame;
    struct frame_result restored_frame;
    struct frame_result lost_frame;
    size_t size = 0u;
    uint8_t *state = NULL;
    uint8_t *roundtrip = NULL;
    uint8_t *source_now = NULL;
    uint8_t *restored_now = NULL;
    uint8_t *lost_now = NULL;
    unsigned instructions = 0u;
    int pcm_nonzero = 0;
    int future_state_diff = 0;
    int future_pcm_diff = 0;
    uint32_t pcm_difference_frame = 0u;
    uint32_t pcm_difference_sample = 0u;

    CHECK(source != NULL && restored != NULL && lost_history != NULL);
    if (source == NULL || restored == NULL || lost_history == NULL)
        goto done;

    /* The origin is a valid constructed PPU point; one complete production JMP reaches 241:1. */
    CHECK_EQ_U64(source->frame_number, 0u);
    CHECK_EQ_U64(source->ticks, 0u);
    CHECK(source->apu.sample_output == NULL);
    CHECK_EQ_U64(source->synth.pcm_count, 0u);
    nesturbator__cpu_step(source);
    CHECK_EQ_U64(source->frame_number, 0u);
    CHECK_EQ_U64(source->ticks, 72u);
    CHECK_EQ_U64(source->ppu.ppu_ticks, 72u);
    CHECK_EQ_U64(source->cpu_cycle, 3u);
    CHECK_EQ_U64(source->ppu.scanline, 241u);
    CHECK_EQ_U64(source->ppu.dot, 1u);
    CHECK_EQ_U64(source->cpu.pc, 0x8000u);
    CHECK_EQ_U64(source->synth.pcm_count, 0u);
    CHECK(source->apu.sample_output == NULL);
    CHECK(has_synth_history(source));
    printf("frame-zero empty-output origin=constructed(240,333) endpoint=executed(241,1) "
           "frame=%llu ticks=%llu sample-phase=%u mixed-level=%d impulse-head=%u "
           "integrator=%lld PCM-count=%u\n",
           (unsigned long long)source->frame_number, (unsigned long long)source->ticks,
           source->apu.sample_phase, source->synth.mixed_level, source->synth.impulse_head,
           (long long)source->synth.integrator_q15, source->synth.pcm_count);

    size = nesturbator_state_size((nesturbator *)source);
    state = (uint8_t *)malloc(size);
    roundtrip = (uint8_t *)malloc(size);
    source_now = (uint8_t *)malloc(size);
    restored_now = (uint8_t *)malloc(size);
    lost_now = (uint8_t *)malloc(size);
    CHECK(size != 0u && state != NULL && roundtrip != NULL && source_now != NULL &&
          restored_now != NULL && lost_now != NULL);
    if (size == 0u || state == NULL || roundtrip == NULL || source_now == NULL ||
        restored_now == NULL || lost_now == NULL)
        goto done;

    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)source, state, size), NESTURBATOR_OK);
    /* Poison the destination with a different valid public-frame state before loading. */
    run_frame(restored, &poisoned_frame);
    CHECK(restored->frame_number != source->frame_number);
    CHECK(restored->ticks != source->ticks);
    CHECK_EQ_U64(nesturbator_load_state((nesturbator *)restored, state, size), NESTURBATOR_OK);
    CHECK_EQ_U64(restored->frame_number, 0u);
    CHECK_EQ_U64(restored->ticks, 72u);
    CHECK(has_synth_history(restored));
    CHECK(same_synth_history(source, restored));
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)restored, roundtrip, size), NESTURBATOR_OK);
    CHECK(memcmp(state, roundtrip, size) == 0);

    CHECK_EQ_U64(nesturbator_load_state((nesturbator *)lost_history, state, size), NESTURBATOR_OK);
    CHECK_EQ_U64(lost_history->frame_number, 0u);
    CHECK_EQ_U64(lost_history->ticks, 72u);
    nesturbator__synth_reset(lost_history);
    CHECK(!same_synth_history(source, lost_history));
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)lost_history, lost_now, size),
                 NESTURBATOR_OK);
    CHECK(memcmp(state, lost_now, size) != 0);
    printf("lost-history control: loaded valid frame-zero state, reset synth history, "
           "history-oracle=detected saved-state-difference=detected\n");

    empty_drain_is_noop(source, state, roundtrip, size, "source");
    empty_drain_is_noop(restored, state, roundtrip, size, "restored");

    /* Resume complete production instructions until the guest's NMI handler marks entry. */
    while (source->bus.ram[NMI_COUNT] == 0u && instructions < 40000u) {
        nesturbator__cpu_step(source);
        nesturbator__cpu_step(restored);
        nesturbator__cpu_step(lost_history);
        instructions++;
    }
    CHECK(instructions < 40000u);
    CHECK_EQ_U64(source->bus.ram[NMI_COUNT], 1u);
    CHECK_EQ_U64(source->bus.ram[NMI_COUNT], restored->bus.ram[NMI_COUNT]);
    CHECK_EQ_U64(source->bus.ram[NMI_COUNT], lost_history->bus.ram[NMI_COUNT]);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)source, source_now, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)restored, restored_now, size),
                 NESTURBATOR_OK);
    CHECK(memcmp(source_now, restored_now, size) == 0);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)lost_history, lost_now, size),
                 NESTURBATOR_OK);
    future_state_diff = memcmp(source_now, lost_now, size) != 0;

    for (uint32_t frame = 1u; frame <= 2u; ++frame) {
        run_frame(source, &source_frame);
        run_frame(restored, &restored_frame);
        run_frame(lost_history, &lost_frame);
        compare_frames(&source_frame, &restored_frame);
        CHECK_EQ_U64(source_frame.frame.frame_number, frame);
        CHECK_EQ_U64(source_frame.frame.ticks, source->ticks);
        CHECK_EQ_U64(source_frame.frame.audio_count, restored_frame.frame.audio_count);
        CHECK_EQ_U64(source_frame.frame.audio_count, lost_frame.frame.audio_count);
        CHECK(source_frame.frame.audio_count != 0u);
        for (uint32_t i = 0u; i < source_frame.frame.audio_count; ++i)
            if (source_frame.audio[i] != 0)
                pcm_nonzero = 1;

        CHECK_EQ_U64(nesturbator_save_state((nesturbator *)source, source_now, size),
                     NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_save_state((nesturbator *)restored, restored_now, size),
                     NESTURBATOR_OK);
        CHECK(memcmp(source_now, restored_now, size) == 0);
        CHECK_EQ_U64(nesturbator_save_state((nesturbator *)lost_history, lost_now, size),
                     NESTURBATOR_OK);
        if (memcmp(source_now, lost_now, size) != 0)
            future_state_diff = 1;
        if (!future_pcm_diff &&
            pcm_diff_index(&source_frame, &lost_frame, &pcm_difference_sample)) {
            future_pcm_diff = 1;
            pcm_difference_frame = frame;
        }
        if (future_pcm_diff)
            break;
    }
    CHECK(pcm_nonzero != 0);
    CHECK(future_state_diff || future_pcm_diff);
    CHECK_EQ_U64(source->bus.ram[NMI_COUNT], restored->bus.ram[NMI_COUNT]);
    CHECK_EQ_U64(source->bus.ram[NMI_COUNT], lost_history->bus.ram[NMI_COUNT]);
    if (future_pcm_diff) {
        printf(
            "lost-history continuation control: state-oracle=%s pcm-difference=frame %u sample %u "
            "guest-count=%u\n",
            future_state_diff ? "different" : "equal", pcm_difference_frame, pcm_difference_sample,
            source->bus.ram[NMI_COUNT]);
    } else {
        printf("lost-history continuation control: state-difference=%s PCM-equal-through=2 frames "
               "guest-count=%u\n",
               future_state_diff ? "detected" : "not-detected", source->bus.ram[NMI_COUNT]);
    }

done:
    free(state);
    free(roundtrip);
    free(source_now);
    free(restored_now);
    free(lost_now);
    if (source != NULL)
        nesturbator_destroy((nesturbator *)source);
    if (restored != NULL)
        nesturbator_destroy((nesturbator *)restored);
    if (lost_history != NULL)
        nesturbator_destroy((nesturbator *)lost_history);
}

static void test_exact_transition_cutpoint(const char *name, uint16_t origin_line,
                                           uint16_t origin_dot, uint8_t status, uint8_t nmi_prev,
                                           uint64_t target_ppu_ticks, uint16_t line, uint16_t dot,
                                           uint8_t expected_before, uint8_t expected_after)
{
    struct nesturbator *a = build_fixture(origin_line, origin_dot, status, 0x80u, nmi_prev);
    struct nesturbator *b = build_fixture(origin_line, origin_dot, status, 0x80u, nmi_prev);
    struct nesturbator *isolated = build_fixture(origin_line, origin_dot, 0u, 0u, 0u);
    size_t size;
    uint8_t *state = NULL;
    uint8_t *roundtrip = NULL;
    uint8_t *a_state = NULL;
    uint8_t *b_state = NULL;
    unsigned steps = 0u;
    int16_t pcm_nonzero = 0;
    struct frame_result frame_a;
    struct frame_result frame_b;

    CHECK(a != NULL && b != NULL && isolated != NULL);
    if (a == NULL || b == NULL || isolated == NULL)
        goto done;

    while (a->ppu.ppu_ticks < target_ppu_ticks && steps < 100000u) {
        nesturbator__cpu_step(a);
        steps++;
    }
    CHECK(steps < 100000u);
    CHECK_EQ_U64(a->ppu.ppu_ticks, target_ppu_ticks);
    CHECK_EQ_U64(a->ticks, target_ppu_ticks);
    CHECK_EQ_U64(a->cpu_cycle, target_ppu_ticks / 24u);
    CHECK_EQ_U64(a->ppu.scanline, line);
    CHECK_EQ_U64(a->ppu.dot, dot);
    CHECK_EQ_U64(a->cpu.pc, 0x8000u);
    CHECK_EQ_U64(a->cpu.jammed, 0u);
    CHECK_EQ_U64(a->ppu.control, 0x80u);
    CHECK_EQ_U64(a->bus.ram[NMI_COUNT], expected_before);
    CHECK(has_synth_history(a));

    printf("cutpoint %s origin=constructed(%u,%u) endpoint=executed(%u,%u) "
           "cycles=%llu instructions=%u PC=$%04X opcode=$4C status=%02X control=%02X "
           "prev/pending/polled=%u/%u/%u guest-count=%u audio-phase=%u synth-level=%d\n",
           name, (unsigned)origin_line, (unsigned)origin_dot, (unsigned)line, (unsigned)dot,
           (unsigned long long)a->cpu_cycle, steps, a->cpu.pc, a->ppu.status, a->ppu.control,
           a->cpu.nmi_prev, a->cpu.nmi_pending, a->cpu.nmi_polled, a->bus.ram[NMI_COUNT],
           a->apu.sample_phase, a->synth.mixed_level);

    size = nesturbator_state_size((nesturbator *)a);
    state = (uint8_t *)malloc(size);
    roundtrip = (uint8_t *)malloc(size);
    a_state = (uint8_t *)malloc(size);
    b_state = (uint8_t *)malloc(size);
    CHECK(state != NULL && roundtrip != NULL && a_state != NULL && b_state != NULL);
    if (state == NULL || roundtrip == NULL || a_state == NULL || b_state == NULL)
        goto done;

    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)a, state, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_state((nesturbator *)b, state, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)b, roundtrip, size), NESTURBATOR_OK);
    CHECK(memcmp(state, roundtrip, size) == 0);
    CHECK(has_synth_history(b));
    CHECK(memcmp(state, roundtrip, size) == 0);

    /* The third instance has a distinct disabled-line history. Interleave its
       execution with the restored pair to guard against shared latches. */
    for (unsigned i = 0u; i < 40000u && a->bus.ram[NMI_COUNT] != expected_after; ++i) {
        nesturbator__cpu_step(a);
        nesturbator__cpu_step(isolated);
        nesturbator__cpu_step(b);
        save_pair_equal(a, b, size, a_state, b_state, "interleaved CPU steps");
        CHECK_EQ_U64(isolated->bus.ram[NMI_COUNT], 0u);
        CHECK_EQ_U64(isolated->cpu.nmi_pending, 0u);
        CHECK_EQ_U64(isolated->cpu.nmi_polled, 0u);
    }
    CHECK_EQ_U64(a->bus.ram[NMI_COUNT], expected_after);
    CHECK_EQ_U64(b->bus.ram[NMI_COUNT], expected_after);

    run_frame(a, &frame_a);
    run_frame(b, &frame_b);
    compare_frames(&frame_a, &frame_b);
    for (uint32_t i = 0u; i < frame_a.frame.audio_count; ++i)
        if (frame_a.audio[i] != 0)
            pcm_nonzero = 1;
    CHECK(pcm_nonzero != 0);
    save_pair_equal(a, b, size, a_state, b_state, "frame continuation");
    printf("cutpoint %s replay=PASS canonical=equal frames=1 ticks=%llu audio=%u pcm=nonzero "
           "video=equal guest-count=%u\n",
           name, (unsigned long long)frame_a.frame.ticks, frame_a.frame.audio_count,
           a->bus.ram[NMI_COUNT]);

done:
    free(state);
    free(roundtrip);
    free(a_state);
    free(b_state);
    if (a != NULL)
        nesturbator_destroy((nesturbator *)a);
    if (b != NULL)
        nesturbator_destroy((nesturbator *)b);
    if (isolated != NULL)
        nesturbator_destroy((nesturbator *)isolated);
}

static size_t state_offset(struct nesturbator *nes, size_t member)
{
    struct nesturbator__state_bound bounds[512];
    size_t count = nesturbator__state_bounds(nes, bounds, 512u);
    size_t size = nesturbator_state_size((nesturbator *)nes);
    uint8_t *before = (uint8_t *)malloc(size);
    uint8_t *after = (uint8_t *)malloc(size);
    uint8_t saved[8];
    uint8_t *field = (uint8_t *)nes + member;
    size_t changed = 0u;
    CHECK(size != 0u && before != NULL && after != NULL);
    if (size == 0u || before == NULL || after == NULL) {
        free(before);
        free(after);
        return 0u;
    }
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)nes, before, size), NESTURBATOR_OK);
    memcpy(saved, field, sizeof(uint8_t));
    *field ^= 0xffu;
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)nes, after, size), NESTURBATOR_OK);
    memcpy(field, saved, sizeof(uint8_t));
    for (size_t i = 64u; i < size; ++i) {
        if (before[i] != after[i]) {
            changed = i;
            break;
        }
    }
    int recorded = 0;
    for (size_t i = 0u; i < count && i < 512u; ++i)
        if (bounds[i].offset == changed)
            recorded = 1;
    CHECK(recorded);
    free(before);
    free(after);
    return recorded ? changed : 0u;
}

static void test_captured_edge_survives_vblank_clear_load(void)
{
    static const uint8_t program[] = {
        0x8du, 0x00u, 0x20u, /* STA $2000: enable NMI just before VBlank clear */
        0xa2u, 0x10u,        /* LDX #$10 */
        0x8eu, 0x40u, 0x00u, /* STX $0040 */
        0x4cu, 0x08u, 0x80u  /* JMP $8008 */
    };
    static const uint8_t handler[] = {
        0xe6u, NMI_COUNT,        /* INC $30: independent guest entry marker */
        0x8eu, 0x31u,     0x00u, /* STX $0031: ordering marker */
        0xe8u,                   /* INX */
        0x40u                    /* RTI */
    };
    struct nesturbator *a =
        nmi_fixture_build(program, sizeof program, handler, sizeof handler, 0x8200u);
    struct nesturbator *b =
        nmi_fixture_build(program, sizeof program, handler, sizeof handler, 0x8200u);
    size_t size;
    uint8_t *state = NULL;
    uint8_t *roundtrip = NULL;
    uint8_t *a_state = NULL;
    uint8_t *b_state = NULL;
    struct frame_result frame_a;
    struct frame_result frame_b;
    int16_t pcm_nonzero = 0;

    CHECK(a != NULL && b != NULL);
    if (a == NULL || b == NULL)
        goto done;

    nmi_fixture_at(a, 260u, 330u, 0x80u, 0u, 0u);
    nmi_fixture_at(b, 260u, 330u, 0x80u, 0u, 0u);
    a->cpu.pc = b->cpu.pc = 0x8000u;
    a->cpu.s = b->cpu.s = 0xfdu;
    a->cpu.p = b->cpu.p = 0x24u;
    a->cpu.a = b->cpu.a = 0x80u;
    a->cpu.x = b->cpu.x = 0u;
    a->bus.ram[0x31u] = b->bus.ram[0x31u] = 0xffu;
    seed_audio(a);
    seed_audio(b);
    nesturbator__cpu_step(a);

    CHECK_EQ_U64(a->cpu_cycle, 4u);
    CHECK_EQ_U64(a->ppu.ppu_ticks, 96u);
    CHECK_EQ_U64(a->ticks, 96u);
    CHECK_EQ_U64(a->ppu.scanline, 261u);
    CHECK_EQ_U64(a->ppu.dot, 1u);
    CHECK_EQ_U64(a->cpu.pc, 0x8003u);
    CHECK_EQ_U64(a->ppu.control, 0x80u);
    CHECK_EQ_U64(a->ppu.status & 0x80u, 0u);
    CHECK_EQ_U64(a->cpu.nmi_prev, 1u);
    CHECK_EQ_U64(a->cpu.nmi_pending, 1u);
    CHECK_EQ_U64(a->cpu.nmi_polled, 0u);
    CHECK(has_synth_history(a));
    printf("cutpoint edge-before-poll-after-clear origin=constructed(260,330) "
           "endpoint=executed(%u,%u) PC=$%04X opcode=$8D bus-cycle=0..%llu "
           "status=%02X control=%02X prev/captured/polled=%u/%u/%u "
           "audio-phase=%u synth-level=%d\n",
           (unsigned)a->ppu.scanline, (unsigned)a->ppu.dot, a->cpu.pc,
           (unsigned long long)a->cpu_cycle, a->ppu.status, a->ppu.control, a->cpu.nmi_prev,
           a->cpu.nmi_pending, a->cpu.nmi_polled, a->apu.sample_phase, a->synth.mixed_level);

    size = nesturbator_state_size((nesturbator *)a);
    state = (uint8_t *)malloc(size);
    roundtrip = (uint8_t *)malloc(size);
    a_state = (uint8_t *)malloc(size);
    b_state = (uint8_t *)malloc(size);
    CHECK(state != NULL && roundtrip != NULL && a_state != NULL && b_state != NULL);
    if (state == NULL || roundtrip == NULL || a_state == NULL || b_state == NULL)
        goto done;
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)a, state, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_state((nesturbator *)b, state, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)b, roundtrip, size), NESTURBATOR_OK);
    CHECK(memcmp(state, roundtrip, size) == 0);
    CHECK(has_synth_history(b));

    for (unsigned i = 0u; i < 16u && a->bus.ram[NMI_COUNT] == 0u; ++i) {
        nesturbator__cpu_step(a);
        nesturbator__cpu_step(b);
        save_pair_equal(a, b, size, a_state, b_state, "post-clear captured-edge steps");
    }
    CHECK_EQ_U64(a->bus.ram[NMI_COUNT], 1u);
    CHECK_EQ_U64(b->bus.ram[NMI_COUNT], 1u);

    run_frame(a, &frame_a);
    run_frame(b, &frame_b);
    compare_frames(&frame_a, &frame_b);
    for (uint32_t i = 0u; i < frame_a.frame.audio_count; ++i)
        if (frame_a.audio[i] != 0)
            pcm_nonzero = 1;
    CHECK(pcm_nonzero != 0);
    save_pair_equal(a, b, size, a_state, b_state, "post-clear captured-edge frame continuation");
    printf("cutpoint edge-before-poll-after-clear replay=PASS canonical=equal guest-count=%u "
           "frames=1 ticks=%llu audio=%u pcm=nonzero video=equal\n",
           a->bus.ram[NMI_COUNT], (unsigned long long)frame_a.frame.ticks,
           frame_a.frame.audio_count);

done:
    free(state);
    free(roundtrip);
    free(a_state);
    free(b_state);
    if (a != NULL)
        nesturbator_destroy((nesturbator *)a);
    if (b != NULL)
        nesturbator_destroy((nesturbator *)b);
}

static void test_edge_before_poll_replay_and_omitted_history(void)
{
    static const uint8_t program[] = {
        0x8du, 0x00u, 0x20u, /* STA $2000: capture before the next legal poll */
        0xa2u, 0x10u,        /* LDX #$10 */
        0x8eu, 0x40u, 0x00u, /* STX $0040 */
        0x4cu, 0x08u, 0x80u  /* JMP $8008 */
    };
    static const uint8_t handler[] = {
        0xe6u, NMI_COUNT,        /* INC $30: independent guest entry marker */
        0x8eu, 0x31u,     0x00u, /* STX $0031: ordering marker */
        0xe8u,                   /* INX */
        0x40u                    /* RTI */
    };
    struct nesturbator *a =
        nmi_fixture_build(program, sizeof program, handler, sizeof handler, 0x8200u);
    struct nesturbator *b =
        nmi_fixture_build(program, sizeof program, handler, sizeof handler, 0x8200u);
    struct nesturbator *omitted_instance =
        nmi_fixture_build(program, sizeof program, handler, sizeof handler, 0x8200u);
    struct nesturbator *isolated =
        nmi_fixture_build(program, sizeof program, handler, sizeof handler, 0x8200u);
    size_t size;
    size_t pending_offset;
    uint8_t *state = NULL;
    uint8_t *omitted = NULL;
    uint8_t *roundtrip = NULL;
    uint8_t *a_state = NULL;
    uint8_t *b_state = NULL;
    int16_t pcm_nonzero = 0;
    struct frame_result frame_a;
    struct frame_result frame_b;
    CHECK(a != NULL && b != NULL && omitted_instance != NULL && isolated != NULL);
    if (a == NULL || b == NULL || omitted_instance == NULL || isolated == NULL)
        goto done;

    nmi_fixture_at(a, 240u, 331u, 0u, 0u, 0u);
    nmi_fixture_at(b, 240u, 331u, 0u, 0u, 0u);
    nmi_fixture_at(omitted_instance, 240u, 331u, 0u, 0u, 0u);
    nmi_fixture_at(isolated, 240u, 331u, 0u, 0u, 0u);
    a->cpu.pc = b->cpu.pc = omitted_instance->cpu.pc = isolated->cpu.pc = 0x8000u;
    a->cpu.s = b->cpu.s = omitted_instance->cpu.s = isolated->cpu.s = 0xfdu;
    a->cpu.p = b->cpu.p = omitted_instance->cpu.p = isolated->cpu.p = 0x24u;
    a->cpu.x = b->cpu.x = omitted_instance->cpu.x = isolated->cpu.x = 0u;
    a->cpu.a = b->cpu.a = omitted_instance->cpu.a = 0x80u;
    isolated->cpu.a = 0u;
    a->bus.ram[0x31u] = b->bus.ram[0x31u] = omitted_instance->bus.ram[0x31u] =
        isolated->bus.ram[0x31u] = 0xffu;
    seed_audio(a);
    seed_audio(b);
    seed_audio(omitted_instance);
    isolated->ppu.control = 0u;
    isolated->cpu.nmi_prev = 0u;
    seed_audio(isolated);
    nesturbator__cpu_step(a);

    CHECK_EQ_U64(a->cpu_cycle, 4u);
    CHECK_EQ_U64(a->ppu.ppu_ticks, 96u);
    CHECK_EQ_U64(a->ticks, 96u);
    CHECK_EQ_U64(a->ppu.scanline, 241u);
    CHECK_EQ_U64(a->ppu.dot, 2u);
    CHECK_EQ_U64(a->cpu.pc, 0x8003u);
    CHECK_EQ_U64(a->ppu.control, 0x80u);
    CHECK((a->ppu.status & 0x80u) != 0u);
    CHECK_EQ_U64(a->cpu.nmi_prev, 1u);
    CHECK_EQ_U64(a->cpu.nmi_pending, 1u);
    CHECK_EQ_U64(a->cpu.nmi_polled, 0u);
    CHECK(has_synth_history(a));
    printf("cutpoint edge-before-poll origin=constructed(240,331) endpoint=executed(%u,%u) "
           "PC=$%04X opcode=$8D bus-cycle=0..%llu prev/captured/polled=%u/%u/%u "
           "audio-phase=%u synth-level=%d next=LDX\n",
           (unsigned)a->ppu.scanline, (unsigned)a->ppu.dot, a->cpu.pc,
           (unsigned long long)a->cpu_cycle, a->cpu.nmi_prev, a->cpu.nmi_pending, a->cpu.nmi_polled,
           a->apu.sample_phase, a->synth.mixed_level);

    size = nesturbator_state_size((nesturbator *)a);
    pending_offset = state_offset(a, offsetof(struct nesturbator, cpu.nmi_pending));
    CHECK(pending_offset >= 64u);
    state = (uint8_t *)malloc(size);
    omitted = (uint8_t *)malloc(size);
    roundtrip = (uint8_t *)malloc(size);
    a_state = (uint8_t *)malloc(size);
    b_state = (uint8_t *)malloc(size);
    CHECK(state != NULL && omitted != NULL && roundtrip != NULL && a_state != NULL &&
          b_state != NULL);
    if (state == NULL || omitted == NULL || roundtrip == NULL || a_state == NULL ||
        b_state == NULL || pending_offset < 64u)
        goto done;

    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)a, state, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_state((nesturbator *)b, state, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)b, roundtrip, size), NESTURBATOR_OK);
    CHECK(memcmp(state, roundtrip, size) == 0);
    CHECK(has_synth_history(b));

    memcpy(omitted, state, size);
    CHECK_EQ_U64(omitted[pending_offset], 1u);
    omitted[pending_offset] = 0u;
    st_recrc(omitted, size);
    nesturbator__cpu_step(omitted_instance);
    CHECK_EQ_U64(omitted_instance->cpu.nmi_pending, 1u);
    CHECK_EQ_U64(nesturbator_load_state((nesturbator *)omitted_instance, omitted, size),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state((nesturbator *)omitted_instance, roundtrip, size),
                 NESTURBATOR_OK);
    CHECK(memcmp(omitted, roundtrip, size) == 0);
    CHECK_EQ_U64(omitted_instance->cpu.nmi_prev, 1u);
    CHECK_EQ_U64(omitted_instance->cpu.nmi_pending, 0u);
    CHECK_EQ_U64(omitted_instance->cpu.nmi_polled, 0u);
    CHECK_EQ_U64(omitted_instance->ppu.control, a->ppu.control);
    CHECK_EQ_U64(omitted_instance->ppu.status, a->ppu.status);

    for (unsigned i = 0u; i < 16u && a->bus.ram[NMI_COUNT] == 0u; ++i) {
        nesturbator__cpu_step(a);
        nesturbator__cpu_step(isolated);
        nesturbator__cpu_step(b);
        nesturbator__cpu_step(omitted_instance);
        save_pair_equal(a, b, size, a_state, b_state, "edge-before-poll CPU steps");
        CHECK_EQ_U64(isolated->bus.ram[NMI_COUNT], 0u);
        CHECK_EQ_U64(isolated->cpu.nmi_pending, 0u);
        CHECK_EQ_U64(isolated->cpu.nmi_polled, 0u);
    }
    CHECK_EQ_U64(a->bus.ram[NMI_COUNT], 1u);
    CHECK_EQ_U64(b->bus.ram[NMI_COUNT], 1u);
    CHECK_EQ_U64(omitted_instance->bus.ram[NMI_COUNT], 0u);
    printf("cutpoint edge-before-poll replay=PASS canonical=equal expected guest count=1 "
           "restored=%u next-poll=NMI-entry\n",
           b->bus.ram[NMI_COUNT]);
    printf("control omitted-history field=cpu.nmi_pending CRC=correct live-line=high "
           "stale-pending=1->loaded-0 expected guest count=1 actual=%u "
           "omitted-history count=%u result=detected\n",
           a->bus.ram[NMI_COUNT], omitted_instance->bus.ram[NMI_COUNT]);

    run_frame(a, &frame_a);
    run_frame(b, &frame_b);
    compare_frames(&frame_a, &frame_b);
    for (uint32_t i = 0u; i < frame_a.frame.audio_count; ++i)
        if (frame_a.audio[i] != 0)
            pcm_nonzero = 1;
    CHECK(pcm_nonzero != 0);
    save_pair_equal(a, b, size, a_state, b_state, "edge-before-poll frame continuation");
    printf("cutpoint edge-before-poll frames=1 ticks=%llu audio=%u pcm=nonzero video=equal\n",
           (unsigned long long)frame_a.frame.ticks, frame_a.frame.audio_count);

done:
    free(state);
    free(omitted);
    free(roundtrip);
    free(a_state);
    free(b_state);
    if (a != NULL)
        nesturbator_destroy((nesturbator *)a);
    if (b != NULL)
        nesturbator_destroy((nesturbator *)b);
    if (omitted_instance != NULL)
        nesturbator_destroy((nesturbator *)omitted_instance);
    if (isolated != NULL)
        nesturbator_destroy((nesturbator *)isolated);
}

static void test_disabled_probe_endpoints_are_limited_guard(void)
{
    static const struct {
        const char *name;
        uint64_t ppu_dots;
        uint16_t line;
        uint16_t dot;
        uint8_t vblank;
    } cases[] = {
        {"set", UINT64_C(261207), 241u, 1u, 1u},
        {"clear", UINT64_C(89343), 261u, 1u, 0u},
    };

    for (size_t c = 0u; c < sizeof cases / sizeof cases[0]; ++c) {
        struct nesturbator *nes = build_fixture(261u, 0u, 0u, 0u, 0u);
        uint64_t target = cases[c].ppu_dots * 8u;
        unsigned steps = 0u;
        CHECK(nes != NULL);
        if (nes == NULL)
            continue;
        while (nes->ppu.ppu_ticks < target && steps < 100000u) {
            nesturbator__cpu_step(nes);
            steps++;
        }
        CHECK(steps < 100000u);
        CHECK_EQ_U64(nes->ppu.ppu_ticks, target);
        CHECK_EQ_U64(nes->ppu.scanline, cases[c].line);
        CHECK_EQ_U64(nes->ppu.dot, cases[c].dot);
        CHECK_EQ_U64((nes->ppu.status >> 7) & 1u, cases[c].vblank);
        CHECK_EQ_U64(nes->ppu.control, 0u);
        CHECK_EQ_U64(nes->cpu.nmi_pending, 0u);
        printf("limited guard disabled-%s origin=constructed(261,0) endpoint=executed(%u,%u) "
               "cycles=%llu instructions=%u NMI=disabled\n",
               cases[c].name, (unsigned)nes->ppu.scanline, (unsigned)nes->ppu.dot,
               (unsigned long long)nes->cpu_cycle, steps);
        nesturbator_destroy((nesturbator *)nes);
    }
}

static void trace_control_candidate(const char *control_case, const char *edge,
                                    uint16_t origin_line, uint16_t base_dot, int offset,
                                    const uint8_t *program, size_t program_len, uint8_t a_value,
                                    uint8_t ppu_status, uint8_t ppu_control, uint8_t io_bus)
{
    static const uint8_t handler[] = {
        0xe6u, NMI_COUNT,        /* INC $30: guest handler-entry marker */
        0x8eu, 0x31u,     0x00u, /* STX $0031: instruction-order marker */
        0xe8u,                   /* INX */
        0x40u                    /* RTI */
    };
    struct nesturbator *nes =
        nmi_fixture_build(program, program_len, handler, sizeof handler, 0x8200u);
    CHECK(nes != NULL);
    if (nes == NULL)
        return;

    uint16_t start_dot = (uint16_t)((int)base_dot + offset);
    nmi_fixture_at(nes, origin_line, start_dot, ppu_status, ppu_control, io_bus);
    nes->cpu.pc = 0x8000u;
    nes->cpu.s = 0xfdu;
    nes->cpu.p = 0x24u;
    nes->cpu.a = a_value;
    nes->cpu.x = 0u;
    nes->cpu.y = 0u;
    nes->cpu.nmi_prev =
        (uint8_t)(((ppu_status & 0x80u) != 0u && (ppu_control & 0x80u) != 0u) ? 1u : 0u);
    nes->bus.ram[NMI_COUNT] = 0u;
    nes->bus.ram[0x31u] = 0xffu;

    for (unsigned step = 0u; step < 12u; ++step) {
        uint16_t pc = nes->cpu.pc;
        uint8_t opcode = 0xffu;
        uint64_t cycle_before = nes->cpu_cycle;
        if (pc >= 0x8000u && (size_t)(pc - 0x8000u) < program_len)
            opcode = program[pc - 0x8000u];
        else if (pc >= 0x8200u && (size_t)(pc - 0x8200u) < sizeof handler)
            opcode = handler[pc - 0x8200u];

        nesturbator__cpu_step(nes);

        const char *next =
            nes->cpu.nmi_polled != 0u
                ? "NMI-entry"
                : (nes->cpu.nmi_pending != 0u
                       ? "next-instruction-poll"
                       : (nes->cpu.pc >= 0x8200u && nes->cpu.pc < 0x8200u + sizeof handler
                              ? "guest-handler"
                              : "source-sequence"));
        printf("sweep case=%s edge=%s offset=%+d alignment=CPU-boundary step=%02u "
               "PC=$%04X opcode=$%02X bus-cycle=%llu..%llu PPU=(%u,%u) "
               "prev/captured/polled=%u/%u/%u guest=%u next=%s\n",
               control_case, edge, offset, step, pc, opcode, (unsigned long long)cycle_before,
               (unsigned long long)nes->cpu_cycle, (unsigned)nes->ppu.scanline,
               (unsigned)nes->ppu.dot, nes->cpu.nmi_prev, nes->cpu.nmi_pending, nes->cpu.nmi_polled,
               nes->bus.ram[NMI_COUNT], next);
        CHECK_EQ_U64(nes->ppu.ppu_ticks, nes->ticks);
        if (nes->bus.ram[NMI_COUNT] != 0u && nes->cpu.pc == 0x8008u)
            break;
    }
    printf("sweep result case=%s edge=%s offset=%+d complete-boundary=1 guest-count=%u "
           "entry-X=%u control=%02X status=%02X\n",
           control_case, edge, offset, nes->bus.ram[NMI_COUNT], nes->bus.ram[0x31u],
           nes->ppu.control, nes->ppu.status);
    nesturbator_destroy((nesturbator *)nes);
}

static void test_bounded_control_transition_sweep(void)
{
    static const uint8_t control8[] = {
        0x8du, 0x00u, 0x20u, /* STA $2000 */
        0xa2u, 0x10u,        /* LDX #$10 */
        0x8eu, 0x40u, 0x00u, /* STX $0040 */
        0x4cu, 0x08u, 0x80u  /* JMP $8008 */
    };
    static const uint8_t control9[] = {
        0xa9u, 0x80u,        /* LDA #$80 */
        0x8du, 0x02u, 0x20u, /* STA $2002: prime I/O bus */
        0xeeu, 0x00u, 0x20u, /* INC $2000: retain both writes */
        0xa2u, 0x10u,        /* LDX #$10 */
        0x4cu, 0x09u, 0x80u  /* JMP $8009 */
    };
    static const int offsets[] = {-2, -1, 0, 1, 2};

    for (size_t i = 0u; i < sizeof offsets / sizeof offsets[0]; ++i) {
        int offset = offsets[i];
        trace_control_candidate("Control-8-STA", "set", 240u, 332u, offset, control8,
                                sizeof control8, 0x80u, 0u, 0u, 0u);
        trace_control_candidate("Control-8-STA", "clear", 260u, 332u, offset, control8,
                                sizeof control8, 0x80u, 0x80u, 0u, 0u);
        trace_control_candidate("Control-9-INC", "set", 240u, 311u, offset, control9,
                                sizeof control9, 0u, 0u, 0u, 0u);
        trace_control_candidate("Control-9-INC", "clear", 260u, 311u, offset, control9,
                                sizeof control9, 0u, 0x80u, 0u, 0u);
    }
}

int main(void)
{
    test_disabled_probe_endpoints_are_limited_guard();
    test_bounded_control_transition_sweep();
    test_frame_zero_empty_pcm_preserves_history();
    test_exact_transition_cutpoint("VBlank-set", 240u, 333u, 0u, 0u, 72u, 241u, 1u, 0u, 1u);
    test_exact_transition_cutpoint("VBlank-clear", 260u, 333u, 0x80u, 1u, 72u, 261u, 1u, 0u, 1u);
    test_captured_edge_survives_vblank_clear_load();
    test_edge_before_poll_replay_and_omitted_history();
    CHECK_DONE();
}

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

int main(void)
{
    test_disabled_probe_endpoints_are_limited_guard();
    test_exact_transition_cutpoint("VBlank-set", 240u, 333u, 0u, 0u, 72u, 241u, 1u, 0u, 1u);
    CHECK_DONE();
}

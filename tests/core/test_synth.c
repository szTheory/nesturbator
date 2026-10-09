/* SND-04: fixed-point event synthesis, filters and frame-boundary state. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../../src/internal.h"
#include "nesturbator.h"
#include "../check.h"

#define AUDIO_CAPACITY 1024u
#define AUDIO_PERIOD 315000u

static uint16_t pixels[256u * 240u];
static int16_t first_frame_a[AUDIO_CAPACITY];
static int16_t first_frame_b[AUDIO_CAPACITY];
static int16_t second_frame_a[AUDIO_CAPACITY];
static int16_t second_frame_b[AUDIO_CAPACITY];

typedef struct transition_capture {
    struct nesturbator *nes;
    uint64_t cycles[2];
    int32_t levels[2];
    int32_t prior_levels[2];
    unsigned count;
} transition_capture;

static void capture_transition(void *context, uint64_t cpu_cycle, int32_t level)
{
    transition_capture *capture = (transition_capture *)context;
    if (capture->count >= 2u)
        return;
    unsigned index = capture->count++;
    capture->cycles[index] = cpu_cycle;
    capture->levels[index] = level;
    capture->prior_levels[index] = capture->nes->synth.mixed_level;
}

static nesturbator *make(void)
{
    nesturbator_config cfg;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    nesturbator *inst = NULL;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    return inst;
}

static void make_image(uint8_t *image, size_t size)
{
    memset(image, 0, size);
    memcpy(image, "NES\032", 4u);
    image[4] = 1u;
    image[5] = 1u;
    /* Enable a pulse with volume 15 and timer 8, then keep the CPU running. */
    static const uint8_t program[] = {0xa9u, 0x01u, 0x8du, 0x15u, 0x40u, 0xa9u, 0xbfu, 0x8du,
                                      0x00u, 0x40u, 0xa9u, 0x08u, 0x8du, 0x02u, 0x40u, 0xa9u,
                                      0x00u, 0x8du, 0x03u, 0x40u, 0x4cu, 0x14u, 0x80u};
    memcpy(image + 16u, program, sizeof program);
    image[16u + 0x3ffcu] = 0u;
    image[16u + 0x3ffdu] = 0x80u;
}

static void check_kernel_tables(void)
{
    CHECK_EQ_U64(sizeof nesturbator__synth_kernel / sizeof nesturbator__synth_kernel[0],
                 NESTURBATOR_SYNTH_PHASES);
    CHECK_EQ_U64(sizeof nesturbator__synth_kernel[0] / sizeof nesturbator__synth_kernel[0][0],
                 NESTURBATOR_SYNTH_TAPS);
    for (unsigned phase = 0u; phase < NESTURBATOR_SYNTH_PHASES; phase++) {
        int32_t sum = 0;
        for (unsigned tap = 0u; tap < NESTURBATOR_SYNTH_TAPS; tap++)
            sum += nesturbator__synth_kernel[phase][tap];
        CHECK_EQ_U64(sum, 32768u);
    }
    for (unsigned tap = 0u; tap < 8u; tap++)
        CHECK_EQ_U64(nesturbator__synth_kernel[16u][tap],
                     nesturbator__synth_kernel[16u][15u - tap]);

    CHECK_EQ_U64(nesturbator__pulse_mix[0], 0u);
    for (unsigned level = 1u; level < 31u; level++)
        CHECK(nesturbator__pulse_mix[level] > nesturbator__pulse_mix[level - 1u]);
    CHECK_EQ_U64(nesturbator__tnd_mix[0][0][0], 0u);
    CHECK(nesturbator__tnd_mix[15][15][127] <= 32767u);
}

static void check_integrator_round_trip(void)
{
    nesturbator *inst = make();
    CHECK(inst != NULL);
    if (inst == NULL)
        return;
    struct nesturbator *nes = (struct nesturbator *)inst;
    int16_t drained[NESTURBATOR_SYNTH_TAPS];
    for (uint32_t phase = 0u; phase < 64u; phase++) {
        nesturbator__synth_reset(nes);
        nes->apu.sample_phase = (AUDIO_PERIOD * phase) / 64u;
        nesturbator__synth_transition(nes, 12345);
        for (unsigned tap = 0u; tap < NESTURBATOR_SYNTH_TAPS; tap++)
            nesturbator__synth_sample(nes);
        CHECK_EQ_U64(nes->synth.integrator_q15, INT64_C(12345) * INT64_C(32768));
        CHECK_EQ_U64(nes->synth.overflow, 0u);
        CHECK_EQ_U64(nesturbator__synth_drain(nes, drained, NESTURBATOR_SYNTH_TAPS),
                     NESTURBATOR_SYNTH_TAPS);
    }
    nesturbator__synth_reset(nes);
    nes->apu.sample_phase = AUDIO_PERIOD - 1u;
    nesturbator__synth_transition(nes, 32767);
    for (unsigned tap = 0u; tap < NESTURBATOR_SYNTH_TAPS; tap++)
        nesturbator__synth_sample(nes);
    CHECK_EQ_U64(nes->synth.integrator_q15, INT64_C(32767) * INT64_C(32768));
    CHECK_EQ_U64(nes->synth.overflow, 0u);
    nesturbator_destroy(inst);
}

static void check_filters_and_chunked_drain(void)
{
    nesturbator *whole = make();
    nesturbator *chunked = make();
    CHECK(whole != NULL && chunked != NULL);
    if (whole == NULL || chunked == NULL) {
        nesturbator_destroy(whole);
        nesturbator_destroy(chunked);
        return;
    }
    struct nesturbator *a = (struct nesturbator *)whole;
    struct nesturbator *b = (struct nesturbator *)chunked;
    int16_t expected[1598];
    int16_t actual[1598];
    nesturbator__synth_transition(a, 12000);
    nesturbator__synth_transition(b, 12000);
    for (unsigned i = 0u; i < 1598u; i++) {
        nesturbator__synth_sample(a);
        CHECK_EQ_U64(nesturbator__synth_drain(a, &expected[i], 1u), 1u);
        nesturbator__synth_sample(b);
        if (i == 798u) {
            CHECK_EQ_U64(nesturbator__synth_drain(b, actual, 799u), 799u);
        } else if (i == 1597u) {
            CHECK_EQ_U64(nesturbator__synth_drain(b, actual + 799u, 799u), 799u);
        }
    }
    for (unsigned i = 0u; i < 1598u; i++)
        CHECK_EQ_U64((uint16_t)actual[i], (uint16_t)expected[i]);
    CHECK_EQ_U64(b->synth.overflow, 0u);
    nesturbator_destroy(whole);
    nesturbator_destroy(chunked);
}

static void check_transition_observer_order(void)
{
    nesturbator *inst = make();
    CHECK(inst != NULL);
    if (inst == NULL)
        return;
    struct nesturbator *nes = (struct nesturbator *)inst;
    transition_capture capture;
    memset(&capture, 0, sizeof capture);
    capture.nes = nes;
    nesturbator__apu_set_transition_sink(nes, capture_transition, &capture);
    nes->ticks = 50u * 24u;
    nes->apu.pulse[0].reg[0] = 0x1fu;
    nes->apu.pulse[0].timer = 8u;
    nes->apu.pulse[0].phase = 7u;
    nes->apu.pulse[0].length = 1u;

    nesturbator__apu_write(nes, 0x4015u, 1u);
    int32_t enabled_level = (int32_t)nesturbator__apu_mixed_level(nes);
    CHECK(enabled_level > 0);
    nesturbator__apu_write(nes, 0x4015u, 0u);
    nesturbator__apu_write(nes, 0x4015u, 0u);

    CHECK_EQ_U64(capture.count, 2u);
    CHECK_EQ_U64(capture.cycles[0], 50u);
    CHECK_EQ_U64(capture.cycles[1], 50u);
    CHECK(capture.levels[0] == enabled_level);
    CHECK_EQ_U64(capture.levels[1], 0u);
    CHECK_EQ_U64(capture.prior_levels[0], 0u);
    CHECK(capture.prior_levels[1] == enabled_level);
    CHECK_EQ_U64(nes->synth.mixed_level, 0u);
    nesturbator_destroy(inst);
}

static void check_dc_filter_and_frame_path(void)
{
    nesturbator *inst = make();
    CHECK(inst != NULL);
    if (inst == NULL)
        return;
    struct nesturbator *nes = (struct nesturbator *)inst;
    int16_t sample = 0;
    unsigned nonzero = 0u;
    nesturbator__synth_transition(nes, 12000);
    for (unsigned i = 0u; i < 4096u; i++) {
        nesturbator__synth_sample(nes);
        CHECK_EQ_U64(nesturbator__synth_drain(nes, &sample, 1u), 1u);
        nonzero += sample != 0;
    }
    CHECK(nonzero > 0u);
    CHECK_EQ_U64((uint16_t)sample, 0u);
    CHECK_EQ_U64(nes->synth.overflow, 0u);
    nesturbator_destroy(inst);

    const size_t image_size = 16u + 16384u + 8192u;
    uint8_t *image = calloc(1u, image_size);
    nesturbator *a_inst = make();
    nesturbator *b_inst = make();
    CHECK(image != NULL && a_inst != NULL && b_inst != NULL);
    if (image == NULL || a_inst == NULL || b_inst == NULL) {
        free(image);
        nesturbator_destroy(a_inst);
        nesturbator_destroy(b_inst);
        return;
    }
    make_image(image, image_size);
    CHECK_EQ_U64(nesturbator_load_cartridge(a_inst, image, image_size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge(b_inst, image, image_size), NESTURBATOR_OK);
    struct nesturbator *a = (struct nesturbator *)a_inst;
    struct nesturbator *b = (struct nesturbator *)b_inst;
    for (unsigned frame = 0u; frame < 2u; frame++) {
        int16_t *a_samples = frame == 0u ? first_frame_a : second_frame_a;
        int16_t *b_samples = frame == 0u ? first_frame_b : second_frame_b;
        nesturbator_frame a_io;
        nesturbator_frame b_io;
        memset(&a_io, 0, sizeof a_io);
        memset(&b_io, 0, sizeof b_io);
        a_io.size = (uint32_t)sizeof a_io;
        a_io.video = pixels;
        a_io.video_pitch = 256u;
        a_io.audio = a_samples;
        a_io.audio_capacity = AUDIO_CAPACITY;
        b_io.size = (uint32_t)sizeof b_io;
        b_io.video = pixels;
        b_io.video_pitch = 256u;
        b_io.audio = b_samples;
        b_io.audio_capacity = AUDIO_CAPACITY;
        CHECK_EQ_U64(nesturbator_run_frame(a_inst, &a_io), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_run_frame(b_inst, &b_io), NESTURBATOR_OK);
        CHECK_EQ_U64(a_io.audio_count, frame == 0u ? 798u : 799u);
        CHECK_EQ_U64(b_io.audio_count, a_io.audio_count);
        CHECK_EQ_U64(memcmp(a_samples, b_samples, (size_t)a_io.audio_count * sizeof a_samples[0]),
                     0u);
        CHECK_EQ_U64(a->synth.pcm_count, 0u);
        CHECK_EQ_U64(b->synth.pcm_count, 0u);
        CHECK_EQ_U64(a->synth.overflow, 0u);
        CHECK_EQ_U64(b->synth.overflow, 0u);
    }
    nesturbator_destroy(a_inst);
    nesturbator_destroy(b_inst);
    free(image);
}

int main(void)
{
    check_kernel_tables();
    check_integrator_round_trip();
    check_filters_and_chunked_drain();
    check_transition_observer_order();
    check_dc_filter_and_frame_path();
    CHECK_DONE();
}

/* STATE-03 (D-08): the fork matrix. For each committed ROM the uninterrupted run A records a
   per-frame digest (video CRC, audio CRC, ticks, frame number) and saves a state at each fork
   point. For each point an instance B, pre-filled with another game, then the same ROM on
   different input, then whitebox-poisoned, loads the state and must continue exactly as A: every
   digest equal, and the final state bytes equal. A second uninterrupted run saves after every
   frame and must match the run that never saves. Usage:
   core.state_fork [--short] [--stress] <other.nes> [<rom>...]   (the first path is the pre-fill
   game; at least one ROM is needed unless --stress is given).
   --stress adds two synthetic DMA and interrupt programs (NROM and MMC3) forked at every frame
   0 to 60 (0, 1, 2, 3, 30 with --short); the run must see OAM DMA, DMC DMA and the IRQ poll
   latch pending at frame boundaries, so those states straddle the cut points. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../nmi_fixture.h"
#include "../state_test.h"

#define MAX_END 180u

struct digest {
    uint32_t video, audio;
    uint64_t ticks, frame_number;
};

/* ci fork set (E = 180) and the fixed asan set (E = 60); frame 0 is the power-on state. */
static const uint32_t FORKS_FULL[] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10,
                                      11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
                                      22, 23, 24, 25, 26, 27, 28, 29, 30, 60, 150};
static const uint32_t FORKS_SHORT[] = {0, 1, 30};
/* Stress images: every frame 0 to 60 over 90 frames (every straddle seen first by frame 44, and
   the run stays well under its 300 s timeout on a loaded runner); the fixed asan set over 60. */
static uint32_t g_stress_full[61];
static const uint32_t STRESS_SHORT[] = {0, 1, 2, 3, 30};

/* Frame-boundary observation of the reference run, set for the stress images only. */
struct straddle {
    uint32_t oam, dmc, poll; /* boundaries where the flag was non-zero */
    uint32_t first_oam, first_dmc, first_poll;
};

static int g_short;
static int g_observe;
static struct straddle g_seen;
static uint8_t *g_other;
static size_t g_other_size;

static void nmi_guest_marker(nesturbator *nes, uint8_t expected)
{
    uint8_t value = 0u;
    CHECK_EQ_U64(nesturbator_peek_cpu_ram(nes, NMI_FIXTURE_HOST_COUNT, &value), NESTURBATOR_OK);
    CHECK_EQ_U64(value, expected);
    for (uint16_t event = 1u; event <= expected; ++event) {
        CHECK_EQ_U64(
            nesturbator_peek_cpu_ram(nes, (uint16_t)(NMI_FIXTURE_HOST_EVENTS + event), &value),
            NESTURBATOR_OK);
        CHECK_EQ_U64(value, event);
    }
}

static nesturbator_status nmi_frame(nesturbator *nes, uint16_t buttons, struct digest *out)
{
    static uint16_t video[256u * 240u];
    static int16_t audio[1024];
    nesturbator_input in;
    nesturbator_frame io;
    uint8_t bytes[2];
    uint32_t crc = 0u;
    int nonzero_audio = 0;

    memset(&in, 0, sizeof in);
    in.size = (uint32_t)sizeof in;
    in.buttons[0] = (uint8_t)(buttons & 0xffu);
    in.buttons[1] = (uint8_t)(buttons >> 8);
    CHECK_EQ_U64(nesturbator_set_input(nes, &in), NESTURBATOR_OK);
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = video;
    io.video_pitch = 256u;
    io.audio = audio;
    io.audio_capacity = 1024u;
    nesturbator_status status = nesturbator_run_frame(nes, &io);
    if (status != NESTURBATOR_OK)
        return status;
    for (size_t i = 0u; i < 256u * 240u; ++i) {
        bytes[0] = (uint8_t)(video[i] & 0xffu);
        bytes[1] = (uint8_t)(video[i] >> 8);
        crc = nesturbator__crc32(crc, bytes, 2u);
    }
    out->video = crc;
    crc = 0u;
    for (uint32_t i = 0u; i < io.audio_count; ++i) {
        uint16_t bits = (uint16_t)audio[i];
        bytes[0] = (uint8_t)bits;
        bytes[1] = (uint8_t)(bits >> 8);
        crc = nesturbator__crc32(crc, bytes, 2u);
        nonzero_audio |= audio[i] != 0;
    }
    CHECK(nonzero_audio);
    out->audio = crc;
    out->ticks = io.ticks;
    out->frame_number = io.frame_number;
    CHECK_EQ_U64(out->frame_number, nes->frame_number);
    return status;
}

static void nmi_fork(void)
{
    static const uint32_t forks[] = {0u, 1u, 2u, 3u};
    static struct digest reference[5];
    static uint8_t image[NMI_FIXTURE_HOST_IMAGE_SIZE];
    static uint8_t *states[sizeof forks / sizeof forks[0]];
    uint8_t *final_state = NULL;
    uint8_t *scratch = NULL;
    const size_t image_size = nmi_fixture_host_image(image, sizeof image);
    const uint32_t end = 4u;
    nesturbator *a = st_make_instance();
    nesturbator *b = st_make_instance();
    size_t state_size;

    CHECK(image_size == sizeof image);
    CHECK(a != NULL && b != NULL);
    if (image_size == 0u || a == NULL || b == NULL)
        goto done;
    st_load(a, image, image_size);
    state_size = nesturbator_state_size(a);
    final_state = (uint8_t *)malloc(state_size);
    scratch = (uint8_t *)malloc(state_size);
    CHECK(state_size != 0u && final_state != NULL && scratch != NULL);
    if (state_size == 0u || final_state == NULL || scratch == NULL)
        goto done;
    nmi_guest_marker(a, 0u); /* frame 0 is the power-on boundary */
    for (size_t i = 0u; i < sizeof forks / sizeof forks[0]; ++i) {
        states[i] = (uint8_t *)malloc(state_size);
        CHECK(states[i] != NULL);
    }
    for (uint32_t frame = 0u; frame <= end; ++frame) {
        if (frame != 0u) {
            CHECK_EQ_U64(nmi_frame(a, st_buttons(frame), &reference[frame]), NESTURBATOR_OK);
            CHECK(reference[frame].video != 0u);
            nmi_guest_marker(a, (uint8_t)frame); /* one source event per completed frame */
        }
        if (frame < sizeof forks / sizeof forks[0]) {
            CHECK_EQ_U64(nesturbator_save_state(a, states[frame], state_size), NESTURBATOR_OK);
        }
    }
    CHECK(reference[1].video != reference[2].video);
    CHECK_EQ_U64(nesturbator_save_state(a, final_state, state_size), NESTURBATOR_OK);

    for (size_t i = 0u; i < sizeof forks / sizeof forks[0]; ++i) {
        uint32_t fork = forks[i];
        struct digest actual;
        st_load(b, image, image_size);
        for (uint32_t frame = 1u; frame <= fork + 3u; ++frame)
            CHECK_EQ_U64(nmi_frame(b, (uint16_t)(0xffu ^ st_buttons(frame)), &actual),
                         NESTURBATOR_OK);
        st_poison(b);
        CHECK_EQ_U64(nesturbator_load_state(b, states[i], state_size), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_save_state(b, scratch, state_size), NESTURBATOR_OK);
        CHECK(memcmp(scratch, states[i], state_size) == 0);
        nmi_guest_marker(b, (uint8_t)fork);
        for (uint32_t frame = fork + 1u; frame <= end; ++frame) {
            CHECK_EQ_U64(nmi_frame(b, st_buttons(frame), &actual), NESTURBATOR_OK);
            CHECK_EQ_U64(actual.video, reference[frame].video);
            CHECK_EQ_U64(actual.audio, reference[frame].audio);
            CHECK_EQ_U64(actual.ticks, reference[frame].ticks);
            CHECK_EQ_U64(actual.frame_number, reference[frame].frame_number);
            nmi_guest_marker(b, (uint8_t)frame);
        }
        CHECK_EQ_U64(nesturbator_save_state(b, scratch, state_size), NESTURBATOR_OK);
        CHECK(memcmp(scratch, final_state, state_size) == 0);
    }
    printf("NMI public fork: frames=0..%u forks=%lu marker=one NMI/frame audio=nonzero\n", end,
           (unsigned long)(sizeof forks / sizeof forks[0]));
done:
    for (size_t i = 0u; i < sizeof forks / sizeof forks[0]; ++i)
        free(states[i]);
    free(final_state);
    free(scratch);
    nesturbator_destroy(a);
    nesturbator_destroy(b);
}

/* The input B runs before the load: not the pattern A uses. */
static uint16_t other_buttons(uint32_t n)
{
    return (uint16_t)((uint32_t)(st_buttons(n + 1000u) ^ 0xffu) & 0xffu);
}

static int digest_same(const char *name, const char *what, uint32_t k, uint32_t frame,
                       const struct digest *a, const struct digest *b)
{
    if (a->video != b->video)
        printf("mismatch %s %s fork %u frame %u: video %08x vs %08x\n", name, what, k, frame,
               (unsigned)a->video, (unsigned)b->video);
    else if (a->audio != b->audio)
        printf("mismatch %s %s fork %u frame %u: audio %08x vs %08x\n", name, what, k, frame,
               (unsigned)a->audio, (unsigned)b->audio);
    else if (a->ticks != b->ticks)
        printf("mismatch %s %s fork %u frame %u: ticks %llu vs %llu\n", name, what, k, frame,
               (unsigned long long)a->ticks, (unsigned long long)b->ticks);
    else if (a->frame_number != b->frame_number)
        printf("mismatch %s %s fork %u frame %u: frame_number %llu vs %llu\n", name, what, k, frame,
               (unsigned long long)a->frame_number, (unsigned long long)b->frame_number);
    else
        return 1;
    check_failures++;
    return 0;
}

/* One code path for file ROMs and in-memory images. */
static void fork_image(const char *name, const uint8_t *img, size_t n, const uint32_t *forks,
                       size_t nforks, uint32_t end)
{
    static struct digest ref[MAX_END + 1u];
    struct digest d = {0};
    nesturbator *a = st_make_instance();
    nesturbator *b = st_make_instance();
    size_t size;
    uint8_t **fork_state = (uint8_t **)calloc(nforks, sizeof *fork_state);
    uint8_t *final_a, *scratch;
    size_t done = 0u, fi = 0u;
    uint32_t prefill = g_short ? 10u : 30u;

    st_load(a, img, n);
    size = nesturbator_state_size(a);
    CHECK(size > 64u);
    final_a = (uint8_t *)malloc(size);
    scratch = (uint8_t *)malloc(size);
    for (size_t i = 0u; i < nforks; ++i)
        fork_state[i] = (uint8_t *)malloc(size);

    /* Reference run; point 0 is saved before any frame. */
    for (uint32_t f = 0u; f <= end; ++f) {
        if (f != 0u) {
            CHECK_EQ_U64(st_frame(a, st_buttons(f), &ref[f].video, &ref[f].audio, &ref[f].ticks,
                                  &ref[f].frame_number),
                         NESTURBATOR_OK);
        }
        if (g_observe) {
            if (a->bus.oam_dma_pending != 0u && g_seen.oam++ == 0u)
                g_seen.first_oam = f;
            if (a->apu.dmc.dma_pending != 0u && g_seen.dmc++ == 0u)
                g_seen.first_dmc = f;
            if (a->cpu.poll_latch != 0u && g_seen.poll++ == 0u)
                g_seen.first_poll = f;
        }
        if (fi < nforks && forks[fi] == f) {
            CHECK_EQ_U64(nesturbator_save_state(a, fork_state[fi], size), NESTURBATOR_OK);
            fi++;
        }
    }
    CHECK_EQ_U64(nesturbator_save_state(a, final_a, size), NESTURBATOR_OK);

    for (size_t i = 0u; i < nforks; ++i) {
        uint32_t k = forks[i];
        int ok = 1;
        st_load(b, g_other, g_other_size);
        for (uint32_t f = 1u; f <= prefill; ++f)
            CHECK_EQ_U64(st_frame(b, other_buttons(f), NULL, NULL, NULL, NULL), NESTURBATOR_OK);
        st_load(b, img, n);
        for (uint32_t f = 1u; f <= k + 7u; ++f)
            CHECK_EQ_U64(st_frame(b, other_buttons(f), NULL, NULL, NULL, NULL), NESTURBATOR_OK);
        st_poison(b);
        CHECK_EQ_U64(nesturbator_load_state(b, fork_state[i], size), NESTURBATOR_OK);
        /* Adjacency: a state loaded and saved again is byte-identical. */
        CHECK_EQ_U64(nesturbator_save_state(b, scratch, size), NESTURBATOR_OK);
        if (memcmp(scratch, fork_state[i], size) != 0) {
            printf("mismatch %s fork %u: save after load differs from the loaded state\n", name, k);
            check_failures++;
        }
        for (uint32_t f = k + 1u; f <= end && ok; ++f) {
            CHECK_EQ_U64(st_frame(b, st_buttons(f), &d.video, &d.audio, &d.ticks, &d.frame_number),
                         NESTURBATOR_OK);
            ok = digest_same(name, "continue", k, f, &ref[f], &d);
        }
        if (ok) {
            CHECK_EQ_U64(nesturbator_save_state(b, scratch, size), NESTURBATOR_OK);
            if (memcmp(scratch, final_a, size) != 0) {
                printf("mismatch %s fork %u: final state bytes differ\n", name, k);
                check_failures++;
            }
        }
        done++;
    }

    /* Save-every-frame neutrality; runs under --short too (asan measured well under 120 s). */
    {
        nesturbator *c = st_make_instance();
        st_load(c, img, n);
        for (uint32_t f = 1u; f <= end; ++f) {
            CHECK_EQ_U64(st_frame(c, st_buttons(f), &d.video, &d.audio, &d.ticks, &d.frame_number),
                         NESTURBATOR_OK);
            CHECK_EQ_U64(nesturbator_save_state(c, scratch, size), NESTURBATOR_OK);
            if (!digest_same(name, "save-every-frame", 0u, f, &ref[f], &d))
                break;
        }
        nesturbator_destroy(c);
    }

    if (g_observe) {
        printf("straddle %s: oam_dma_pending %u (first %u), dmc.dma_pending %u (first %u), "
               "poll_latch %u (first %u) of %u boundaries\n",
               name, (unsigned)g_seen.oam, (unsigned)g_seen.first_oam, (unsigned)g_seen.dmc,
               (unsigned)g_seen.first_dmc, (unsigned)g_seen.poll, (unsigned)g_seen.first_poll,
               (unsigned)end + 1u);
        CHECK(g_seen.oam != 0u);
        CHECK(g_seen.dmc != 0u);
        CHECK(g_seen.poll != 0u);
    }
    printf("forks %s %u\n", name, (unsigned)done);
    CHECK_EQ_U64(done, nforks);
    for (size_t i = 0u; i < nforks; ++i)
        free(fork_state[i]);
    free(fork_state);
    free(final_a);
    free(scratch);
    nesturbator_destroy(a);
    nesturbator_destroy(b);
}

int main(int argc, char **argv)
{
    int first = 1, stress = 0;
    const uint32_t *forks;
    size_t nforks;
    uint32_t end;
    if (argc == 2 && strcmp(argv[1], "--nmi") == 0) {
        nmi_fork();
        CHECK_DONE();
    }
    if (argc > 1 && strcmp(argv[1], "--short") == 0) {
        g_short = 1;
        first = 2;
    }
    if (argc > first && strcmp(argv[first], "--stress") == 0) {
        stress = 1;
        first++;
    }
    if (argc - first < (stress ? 1 : 2)) {
        fprintf(stderr, "usage: core.state_fork [--short] [--stress] <other.nes> [<rom>...]\n");
        return 2;
    }
    if (!st_read_file(argv[first], &g_other, &g_other_size)) {
        fprintf(stderr, "cannot read %s\n", argv[first]);
        return 2;
    }
    forks = g_short ? FORKS_SHORT : FORKS_FULL;
    nforks = g_short ? sizeof FORKS_SHORT / sizeof FORKS_SHORT[0]
                     : sizeof FORKS_FULL / sizeof FORKS_FULL[0];
    end = g_short ? 60u : 180u;
    for (int i = first + 1; i < argc; ++i) {
        uint8_t *rom;
        size_t size;
        if (!st_read_file(argv[i], &rom, &size)) {
            fprintf(stderr, "cannot read %s\n", argv[i]);
            return 2;
        }
        fork_image(argv[i], rom, size, forks, nforks, end);
        free(rom);
    }
    if (stress) {
        static uint8_t buf[16u + 2u * INES_PRG_BANK + INES_CHR_BANK];
        static const uint16_t mappers[] = {0u, 4u};
        static const char *const names[] = {"stress-nrom", "stress-mmc3"};
        const uint32_t *sf = g_short ? STRESS_SHORT : g_stress_full;
        size_t ns = g_short ? sizeof STRESS_SHORT / sizeof STRESS_SHORT[0] : 61u;
        for (uint32_t k = 0u; k < 61u; ++k)
            g_stress_full[k] = k;
        for (size_t i = 0u; i < sizeof mappers / sizeof mappers[0]; ++i) {
            size_t n = st_stress_image(buf, sizeof buf, mappers[i]);
            CHECK(n != 0u);
            memset(&g_seen, 0, sizeof g_seen);
            g_observe = 1;
            fork_image(names[i], buf, n, sf, ns, g_short ? 60u : 90u);
            g_observe = 0;
        }
    }
    free(g_other);
    CHECK_DONE();
}

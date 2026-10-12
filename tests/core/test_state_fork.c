/* STATE-03 (D-08): the fork matrix. For each committed ROM the uninterrupted run A records a
   per-frame digest (video CRC, audio CRC, ticks, frame number) and saves a state at each fork
   point. For each point an instance B, pre-filled with another game, then the same ROM on
   different input, then whitebox-poisoned, loads the state and must continue exactly as A: every
   digest equal, and the final state bytes equal. A second uninterrupted run saves after every
   frame and must match the run that never saves. Usage:
   core.state_fork [--short] <other.nes> <rom>...   (the first path is the pre-fill game). */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../state_test.h"

#define MAX_END 180u

struct digest {
    uint32_t video, audio;
    uint64_t ticks, frame_number;
};

/* ci fork set (E = 180) and the fixed asan set (E = 60); frame 0 is the power-on state. */
static const uint32_t FORKS_FULL[] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
                                      12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
                                      24, 25, 26, 27, 28, 29, 30, 60, 150};
static const uint32_t FORKS_SHORT[] = {0, 1, 30};

static int g_short;
static uint8_t *g_other;
static size_t g_other_size;

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
        printf("mismatch %s %s fork %u frame %u: frame_number %llu vs %llu\n", name, what, k,
               frame, (unsigned long long)a->frame_number, (unsigned long long)b->frame_number);
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
    struct digest d;
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
            CHECK_EQ_U64(st_frame(b, st_buttons(f), &d.video, &d.audio, &d.ticks,
                                  &d.frame_number),
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

    /* Save-every-frame neutrality (skipped under --short, see the plan's asan fallback). */
    if (!g_short) {
        nesturbator *c = st_make_instance();
        st_load(c, img, n);
        for (uint32_t f = 1u; f <= end; ++f) {
            CHECK_EQ_U64(st_frame(c, st_buttons(f), &d.video, &d.audio, &d.ticks,
                                  &d.frame_number),
                         NESTURBATOR_OK);
            CHECK_EQ_U64(nesturbator_save_state(c, scratch, size), NESTURBATOR_OK);
            if (!digest_same(name, "save-every-frame", 0u, f, &ref[f], &d))
                break;
        }
        nesturbator_destroy(c);
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
    int first = 1;
    const uint32_t *forks;
    size_t nforks;
    uint32_t end;
    if (argc > 1 && strcmp(argv[1], "--short") == 0) {
        g_short = 1;
        first = 2;
    }
    if (argc - first < 2) {
        fprintf(stderr, "usage: core.state_fork [--short] <other.nes> <rom>...\n");
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
    free(g_other);
    CHECK_DONE();
}

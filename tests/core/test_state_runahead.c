/* LR-02 (D-12): RetroArch's second-instance run-ahead mode as a library property, with two
   independent instances in one thread (the libretro host covers the single-instance mode through
   the module).

   core.state_runahead [--short] <rom>...

   Instance A runs the real timeline with in[t]; each tick instance B loads A's state and runs N
   frames with in[t] repeated, the last one being the visible frame. For every ROM and for the two
   Phase 12 stress images the test requires:
     - the visible frame equals the one-instance model's (save, run N frames with in[t] repeated,
       keep the last, load) at every tick;
     - A's timeline, its frame video and audio hashes and its state bytes after every frame, equals
       an uninterrupted run;
     - on ticks where in[t..t+N] are all equal, the visible frame equals the uninterrupted run's
       frame t+N.
   nesteroids runs both input scripts at depths 1 to 3; every other image runs one script at one
   depth, both rotating with the image, so the run stays within its time budget. T is 180 ticks
   (60 with --short). */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../state_test.h"
#include "../runahead_model.h"

#define MIN_IMAGES 11u /* nine ROMs and two stress images */

struct totals {
    unsigned images, ticks, states, equal_ticks, expected_ticks;
};

static int run_depth(const char *name, const uint8_t *rom, size_t size, const uint16_t *in,
                     uint32_t T, unsigned N, const struct ra_oracle *oracle, struct totals *tot)
{
    struct ra_oracle a;
    uint64_t *two = (uint64_t *)calloc(T, sizeof *two);
    uint64_t *one = (uint64_t *)calloc(T, sizeof *one);
    unsigned bad = 0u;

    CHECK(two != NULL && one != NULL);
    if (two != NULL && one != NULL && ra_run_two_instance(rom, size, in, T, N, &a, two) &&
        ra_run_one_instance(rom, size, in, T, N, one)) {
        CHECK_EQ_U64(a.state_size, oracle->state_size);
        for (uint32_t t = 0u; t < T; t++) {
            int same_inputs = 1;
            if (two[t] != one[t]) {
                fprintf(stderr, "%s depth %u: tick %u: two-instance and one-instance differ\n",
                        name, N, (unsigned)t);
                bad++;
            }
            if (a.video[t] != oracle->video[t] || a.audio[t] != oracle->audio[t] ||
                memcmp(a.state + (size_t)t * a.state_size, oracle->state + (size_t)t * a.state_size,
                       a.state_size) != 0) {
                fprintf(stderr,
                        "%s depth %u: tick %u: instance A differs from the uninterrupted run\n",
                        name, N, (unsigned)t);
                bad++;
            }
            for (unsigned j = 1u; j <= N; j++)
                same_inputs = same_inputs && in[t + j] == in[t];
            if (same_inputs) {
                tot->equal_ticks++;
                if (two[t] != oracle->video[t + N]) {
                    fprintf(stderr, "%s depth %u: tick %u: equal inputs, visible frame differs\n",
                            name, N, (unsigned)t);
                    bad++;
                }
            }
            tot->ticks++;
            tot->states++;
        }
        ra_oracle_free(&a);
    } else {
        bad++;
    }
    tot->expected_ticks += T;
    CHECK_EQ_U64(bad, 0u);
    free(two);
    free(one);
    return bad == 0u;
}

static void run_image(const char *name, const uint8_t *rom, size_t size, unsigned index, int full,
                      uint32_t T, struct totals *tot)
{
    for (unsigned script = 1u; script <= 2u; script++) {
        uint16_t *in;
        struct ra_oracle oracle;

        if (!full && script != 1u + (index & 1u))
            continue;
        in = ra_script_make(script, script == 1u ? RA_SEED_1 : RA_SEED_2, T + RA_MAX_DEPTH);
        CHECK(in != NULL);
        if (in == NULL || !ra_run_uninterrupted(rom, size, in, T + RA_MAX_DEPTH, &oracle)) {
            CHECK(0);
            free(in);
            continue;
        }
        for (unsigned N = 1u; N <= RA_MAX_DEPTH; N++) {
            if (!full && N != 1u + index % RA_MAX_DEPTH)
                continue;
            run_depth(name, rom, size, in, T, N, &oracle, tot);
        }
        ra_oracle_free(&oracle);
        free(in);
    }
    tot->images++;
}

int main(int argc, char **argv)
{
    uint32_t T = 180u;
    int first = 1;
    unsigned index = 0u;
    struct totals tot;
    unsigned full_images = 0u;

    if (argc > first && strcmp(argv[first], "--short") == 0) {
        T = 60u;
        first++;
    }
    if (argc <= first) {
        fprintf(stderr, "usage: core.state_runahead [--short] <rom>...\n");
        return 2;
    }
    memset(&tot, 0, sizeof tot);
    for (int i = first; i < argc; i++, index++) {
        uint8_t *rom;
        size_t size;
        int full = strstr(argv[i], "nesteroids") != NULL;
        if (!st_read_file(argv[i], &rom, &size)) {
            fprintf(stderr, "cannot read %s\n", argv[i]);
            return 2;
        }
        full_images += full ? 1u : 0u;
        run_image(argv[i], rom, size, index, full, T, &tot);
        free(rom);
    }
    {
        static uint8_t buf[16u + 2u * INES_PRG_BANK + INES_CHR_BANK];
        static const uint16_t mappers[] = {0u, 4u};
        static const char *const names[] = {"stress-nrom", "stress-mmc3"};
        for (unsigned m = 0u; m < 2u; m++, index++) {
            size_t n = st_stress_image(buf, sizeof buf, mappers[m]);
            CHECK(n != 0u);
            if (n != 0u)
                run_image(names[m], buf, n, index, 0, T, &tot);
        }
    }
    /* D-20: counts that rule out a pass that did nothing. */
    CHECK(tot.images >= MIN_IMAGES);
    CHECK_EQ_U64(full_images, 1u);
    CHECK_EQ_U64(tot.ticks, tot.expected_ticks);
    CHECK(tot.ticks >= (RA_MAX_DEPTH * 2u + (tot.images - 1u)) * T);
    CHECK(tot.equal_ticks >= 1u);
    printf("core.state_runahead: images=%u ticks=%u states=%u equal_input_ticks=%u\n", tot.images,
           tot.ticks, tot.states, tot.equal_ticks);
    CHECK_DONE();
}

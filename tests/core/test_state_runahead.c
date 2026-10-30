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
   (30 with --short). */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../nmi_fixture.h"
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

static void run_nmi_fixture(void)
{
    static uint8_t image[NMI_FIXTURE_HOST_IMAGE_SIZE];
    const uint32_t T = 8u;
    const size_t image_size = nmi_fixture_host_image(image, sizeof image);
    uint16_t *in = ra_script_make(1u, RA_SEED_1, T + RA_MAX_DEPTH);
    struct ra_oracle oracle;
    nesturbator *probe = NULL;
    uint8_t *saved = NULL;

    CHECK(image_size == sizeof image && in != NULL);
    if (image_size == 0u || in == NULL) {
        free(in);
        return;
    }
    CHECK(ra_run_uninterrupted(image, image_size, in, T + RA_MAX_DEPTH, &oracle));
    if (oracle.state == NULL) {
        free(in);
        return;
    }
    probe = ra_instance(image, image_size);
    saved = (uint8_t *)malloc(oracle.state_size);
    CHECK(probe != NULL && saved != NULL);
    if (probe == NULL || saved == NULL)
        goto done;

    /* These are independent guest observations: each completed host frame causes
       one NMI, and the handler records its one-based event ordinal. */
    for (uint32_t t = 0u; t < oracle.frames; ++t) {
        CHECK_EQ_U64(oracle.guest_marker[t], t + 1u);
        CHECK(ra_audio_hash_is_nonzero_pcm(oracle.audio[t]));
        CHECK_EQ_U64(nesturbator_load_state(probe, oracle.state + (size_t)t * oracle.state_size,
                                            oracle.state_size),
                     NESTURBATOR_OK);
        ra_check_nmi_marker(probe, (uint8_t)(t + 1u));
    }
    CHECK(oracle.video[0] != oracle.video[1]);

    for (unsigned depth = 1u; depth <= RA_MAX_DEPTH; ++depth) {
        struct ra_oracle two;
        uint64_t *visible = (uint64_t *)calloc(T, sizeof *visible);
        nesturbator *one = ra_instance(image, image_size);
        CHECK(visible != NULL && one != NULL);
        if (visible == NULL || one == NULL) {
            free(visible);
            nesturbator_destroy(one);
            continue;
        }
        CHECK(ra_run_two_instance(image, image_size, in, T, depth, &two, visible));
        if (two.state != NULL) {
            for (uint32_t t = 0u; t < T; ++t) {
                CHECK_EQ_U64(two.guest_marker[t], t + 1u);
                CHECK_EQ_U64(memcmp(two.state + (size_t)t * two.state_size,
                                    oracle.state + (size_t)t * oracle.state_size,
                                    oracle.state_size),
                             0u);
                CHECK_EQ_U64(two.video[t], oracle.video[t]);
                CHECK_EQ_U64(two.audio[t], oracle.audio[t]);
                CHECK_EQ_U64(visible[t], oracle.video[t + depth]);
                CHECK_EQ_U64(nesturbator_load_state(probe, two.state + (size_t)t * two.state_size,
                                                    two.state_size),
                             NESTURBATOR_OK);
                ra_check_nmi_marker(probe, (uint8_t)(t + 1u));
                CHECK_EQ_U64(nesturbator_load_state(
                                 probe, oracle.state + (size_t)(t + depth) * oracle.state_size,
                                 oracle.state_size),
                             NESTURBATOR_OK);
                ra_check_nmi_marker(probe, (uint8_t)(t + depth + 1u));
            }
            ra_oracle_free(&two);
        }

        for (uint32_t t = 0u; t < T; ++t) {
            uint64_t video = 0u, audio = 0u;
            CHECK_EQ_U64(ra_lib_frame_status(one, in[t], &video, &audio, NULL), RA_FRAME_OK);
            CHECK_EQ_U64(video, oracle.video[t]);
            CHECK_EQ_U64(audio, oracle.audio[t]);
            ra_check_nmi_marker(one, (uint8_t)(t + 1u));
            CHECK_EQ_U64(nesturbator_save_state(one, saved, oracle.state_size), NESTURBATOR_OK);
            CHECK(memcmp(saved, oracle.state + (size_t)t * oracle.state_size, oracle.state_size) ==
                  0);

            for (unsigned j = 1u; j <= depth; ++j) {
                CHECK(in[t + j] != in[t]);
                CHECK_EQ_U64(ra_lib_frame_status(one, in[t + j], &video, &audio, NULL),
                             RA_FRAME_OK);
                CHECK_EQ_U64(video, oracle.video[t + j]);
                CHECK_EQ_U64(audio, oracle.audio[t + j]);
                ra_check_nmi_marker(one, (uint8_t)(t + j + 1u));
            }
            CHECK_EQ_U64(nesturbator_load_state(one, saved, oracle.state_size), NESTURBATOR_OK);
            CHECK_EQ_U64(nesturbator_save_state(one, saved, oracle.state_size), NESTURBATOR_OK);
            CHECK(memcmp(saved, oracle.state + (size_t)t * oracle.state_size, oracle.state_size) ==
                  0);
            ra_check_nmi_marker(one, (uint8_t)(t + 1u));
        }
        nesturbator_destroy(one);
        free(visible);
    }

    /* A wrong host that skips the restore leaves the speculative guest count ahead.
       This control is checked directly against the marker, not only frame equality. */
    {
        nesturbator *wrong = ra_instance(image, image_size);
        uint8_t count = 0u;
        CHECK(wrong != NULL);
        if (wrong != NULL) {
            CHECK_EQ_U64(ra_lib_frame_status(wrong, 0u, NULL, NULL, NULL), RA_FRAME_OK);
            CHECK_EQ_U64(nesturbator_peek_cpu_ram(wrong, NMI_FIXTURE_HOST_COUNT, &count),
                         NESTURBATOR_OK);
            CHECK_EQ_U64(count, 1u);
            for (unsigned j = 0u; j < 2u; ++j) {
                CHECK_EQ_U64(ra_lib_frame_status(wrong, 0u, NULL, NULL, NULL), RA_FRAME_OK);
            }
            CHECK_EQ_U64(nesturbator_peek_cpu_ram(wrong, NMI_FIXTURE_HOST_COUNT, &count),
                         NESTURBATOR_OK);
            CHECK_EQ_U64(ra_lib_frame_status(wrong, in[1], NULL, NULL, NULL), RA_FRAME_OK);
            CHECK_EQ_U64(nesturbator_peek_cpu_ram(wrong, NMI_FIXTURE_HOST_COUNT, &count),
                         NESTURBATOR_OK);
            CHECK(count != oracle.guest_marker[1]); /* expected 2 after restoring frame 1 */
            printf("core.state_runahead: omitted-load control marker=%u expected=%u\n", count,
                   oracle.guest_marker[1]);
            nesturbator_destroy(wrong);
        }
    }
    {
        nesturbator *wrong = ra_instance(image, image_size);
        uint8_t *stale = (uint8_t *)malloc(oracle.state_size);
        uint8_t count = 0u;
        CHECK(wrong != NULL && stale != NULL);
        if (wrong != NULL && stale != NULL) {
            CHECK_EQ_U64(ra_lib_frame_status(wrong, in[0], NULL, NULL, NULL), RA_FRAME_OK);
            CHECK_EQ_U64(nesturbator_save_state(wrong, stale, oracle.state_size), NESTURBATOR_OK);
            CHECK_EQ_U64(ra_lib_frame_status(wrong, in[1], NULL, NULL, NULL), RA_FRAME_OK);
            CHECK_EQ_U64(ra_lib_frame_status(wrong, in[2], NULL, NULL, NULL), RA_FRAME_OK);
            CHECK_EQ_U64(nesturbator_load_state(wrong, stale, oracle.state_size), NESTURBATOR_OK);
            CHECK_EQ_U64(nesturbator_peek_cpu_ram(wrong, NMI_FIXTURE_HOST_COUNT, &count),
                         NESTURBATOR_OK);
            CHECK(count != oracle.guest_marker[1]); /* stale frame 1, expected frame 2 */
            printf("core.state_runahead: stale-load control marker=%u expected=%u\n", count,
                   oracle.guest_marker[1]);
        }
        free(stale);
        nesturbator_destroy(wrong);
    }
    printf("core.state_runahead: NMI marker, state, frame, tick and PCM replay passed at depths "
           "1..3\n");
done:
    free(saved);
    nesturbator_destroy(probe);
    ra_oracle_free(&oracle);
    free(in);
}

/* D-18 native parity: every internally reached BRK/IRQ/NMI next-action cut
   continues through the library two-instance model and agrees with an
   uninterrupted native continuation for both established input scripts. */
static void run_interrupt_entry_cutpoints(void)
{
    static const enum nesturbator_entry_origin origins[] = {
        NESTURBATOR_ENTRY_BRK, NESTURBATOR_ENTRY_IRQ, NESTURBATOR_ENTRY_NMI};
    static uint8_t image[RA_ENTRY_ROM_SIZE];
    const uint32_t T = 8u;
    const size_t image_size = ra_entry_rom_build(image, sizeof image);
    nesturbator *probe = ra_instance(image, image_size);
    size_t state_size = probe != NULL ? nesturbator_state_size(probe) : 0u;
    unsigned cutpoints = 0u, script_depth_cases = 0u;

    CHECK_EQ_U64(image_size, RA_ENTRY_ROM_SIZE);
    CHECK(probe != NULL && state_size != 0u);
    if (probe == NULL || state_size == 0u) {
        nesturbator_destroy(probe);
        return;
    }

    for (size_t origin_index = 0u; origin_index < sizeof origins / sizeof origins[0];
         origin_index++) {
        enum nesturbator_entry_origin origin = origins[origin_index];
        for (uint8_t action = ra_entry_first_action(origin); action <= 7u; action++) {
            uint8_t *cutpoint = (uint8_t *)malloc(state_size);
            CHECK(cutpoint != NULL);
            if (cutpoint == NULL)
                continue;
            CHECK(ra_entry_cutpoint_state(image, image_size, origin, action, cutpoint, state_size));
            cutpoints++;

            for (unsigned script = 1u; script <= 2u; script++) {
                uint16_t *in =
                    ra_script_make(script, script == 1u ? RA_SEED_1 : RA_SEED_2, T + RA_MAX_DEPTH);
                CHECK(in != NULL);
                if (in == NULL)
                    continue;
                {
                    struct ra_oracle oracle;
                    memset(&oracle, 0, sizeof oracle);
                    CHECK(ra_run_from_state(image, image_size, cutpoint, state_size, in,
                                            T + RA_MAX_DEPTH, &oracle));
                    if (oracle.state != NULL) {
                        for (unsigned depth = 1u; depth <= RA_MAX_DEPTH; depth++) {
                            struct ra_oracle runahead;
                            uint64_t *visible = (uint64_t *)calloc(T, sizeof *visible);
                            memset(&runahead, 0, sizeof runahead);
                            CHECK(visible != NULL);
                            if (visible == NULL)
                                continue;
                            CHECK(ra_run_two_instance_from_state(image, image_size, cutpoint,
                                                                 state_size, in, T, depth,
                                                                 &runahead, visible));
                            if (runahead.state != NULL) {
                                for (uint32_t t = 0u; t < T; t++) {
                                    int same_inputs = 1;
                                    const uint8_t *actual =
                                        runahead.state + (size_t)t * runahead.state_size;
                                    const uint8_t *expected =
                                        oracle.state + (size_t)t * oracle.state_size;
                                    CHECK_EQ_U64(runahead.video[t], oracle.video[t]);
                                    CHECK_EQ_U64(runahead.audio[t], oracle.audio[t]);
                                    CHECK_EQ_U64(runahead.guest_marker[t], oracle.guest_marker[t]);
                                    CHECK_EQ_U64(runahead.guest_marker_secondary[t],
                                                 oracle.guest_marker_secondary[t]);
                                    CHECK(memcmp(actual, expected, state_size) == 0);
                                    CHECK_EQ_U64(ra_state_frame_number(actual),
                                                 ra_state_frame_number(expected));
                                    CHECK_EQ_U64(ra_state_ticks(actual), ra_state_ticks(expected));
                                    for (unsigned j = 1u; j <= depth; j++)
                                        same_inputs = same_inputs && in[t + j] == in[t];
                                    if (same_inputs)
                                        CHECK_EQ_U64(visible[t], oracle.video[t + depth]);
                                }
                                ra_oracle_free(&runahead);
                            }
                            free(visible);
                            script_depth_cases++;
                        }
                        ra_oracle_free(&oracle);
                    }
                }
                free(in);
            }
            free(cutpoint);
        }
    }
    CHECK_EQ_U64(cutpoints, RA_ENTRY_CUTPOINT_CLASSES);
    CHECK_EQ_U64(script_depth_cases, RA_ENTRY_CUTPOINT_CLASSES * 2u * RA_MAX_DEPTH);
    printf("core.state_runahead: entry cutpoints=%u scripts=2 depths=1..3 native parity passed\n",
           cutpoints);
    nesturbator_destroy(probe);
}

int main(int argc, char **argv)
{
    uint32_t T = 180u;
    int first = 1;
    unsigned index = 0u;
    struct totals tot;
    unsigned full_images = 0u;

    if (argc > first && strcmp(argv[first], "--short") == 0) {
        T = 30u;
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
    run_nmi_fixture();
    run_interrupt_entry_cutpoints();
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

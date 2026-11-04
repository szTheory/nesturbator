/* D-11: run_frame checks, buffer rules, the D-09 audio period, the D-02 test
   card's boundary pixels and two independent instances, through the public
   header only. */
#include <stdint.h>
#include <string.h>

#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"

#define W 256u
#define H 240u
#define WIDE 300u /* a pitch wider than a row */
#define AUDIO_CAP 1024u
#define POISON 0x5A5Au /* sentinel the core must overwrite or leave alone */

static uint16_t video[WIDE * H];
static uint16_t video2[W * H];
static int16_t audio[AUDIO_CAP];

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

/* Fills *io in place after a memset: a struct copy would leave its padding
   unspecified (C17 6.2.6.1p6). */
static void frame_io(nesturbator_frame *io, uint16_t *v, uint32_t pitch, int16_t *a, uint32_t cap)
{
    memset(io, 0, sizeof *io);
    io->size = (uint32_t)sizeof *io;
    io->video = v;
    io->video_pitch = pitch;
    io->audio = a;
    io->audio_capacity = cap;
}

static size_t build_loop_image(uint8_t *image, size_t capacity)
{
    static const uint8_t loop_code[] = {0x4Cu, 0x00u, 0x80u}; /* JMP $8000 */
    struct ines_spec spec;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = 1u;
    spec.prg_code = loop_code;
    spec.prg_code_len = sizeof loop_code;
    spec.nmi_vector = 0x8000u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    size_t image_size = ines_build(image, capacity, &spec);
    CHECK_EQ_U64(image_size, 16u + INES_PRG_BANK + INES_CHR_BANK);
    return image_size;
}

static uint64_t expected_pal_samples(uint64_t frames)
{
    /* Independent fraction from 322445/6448 frames/s at 48000 samples/s. */
    return frames * UINT64_C(48000) * UINT64_C(6448) / UINT64_C(322445);
}

static uint64_t expected_ntsc_samples(uint64_t frames)
{
    return frames * UINT64_C(714732) * UINT64_C(352) / UINT64_C(315000);
}

static uint64_t loop_frame_tick_increment(uint32_t frame_ticks, uint32_t cpu_cycle_ticks)
{
    const uint64_t loop_ticks = (uint64_t)cpu_cycle_ticks * 3u;
    return ((uint64_t)frame_ticks + loop_ticks - 1u) / loop_ticks * loop_ticks;
}

/* PAL calls end at the next absolute nominal frame endpoint; instruction-boundary overshoot is
   carried forward instead of adding one independently rounded frame after every call. */
static uint64_t next_pal_frame_ticks(uint32_t frame_number, uint64_t current_ticks)
{
    const uint64_t target = (uint64_t)frame_number * UINT64_C(1063920);
    CHECK(target >= current_ticks);
    return current_ticks + loop_frame_tick_increment((uint32_t)(target - current_ticks), 32u);
}

/* A valid guest loop may repeat its PC and pixels while frame time advances. */
static void test_looping_guest_progress(void)
{
    static const uint8_t loop_code[] = {0x4cu, 0x00u, 0x80u}; /* JMP $8000 */
    static uint16_t previous[W * H];
    uint8_t image[16u + INES_PRG_BANK + INES_CHR_BANK];
    struct ines_spec spec;
    const size_t image_size = sizeof image;
    nesturbator *inst = make();
    uint64_t previous_ticks = 0u;
    uint64_t previous_frame = 0u;

    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 1u;
    spec.chr_8k = 1u;
    spec.prg_code = loop_code;
    spec.prg_code_len = sizeof loop_code;
    spec.nmi_vector = 0x8000u;
    spec.reset_vector = 0x8000u;
    spec.irq_vector = 0x8000u;
    CHECK_EQ_U64(ines_build(image, sizeof image, &spec), image_size);
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, image, image_size), NESTURBATOR_OK);

    for (uint32_t frame = 0u; frame < 5u; ++frame) {
        nesturbator_frame io;
        frame_io(&io, video2, W, audio, AUDIO_CAP);
        CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
        CHECK(io.frame_number > previous_frame);
        CHECK(io.ticks > previous_ticks);
        if (frame >= 1u)
            CHECK(memcmp(video2, previous, sizeof previous) == 0);
        memcpy(previous, video2, sizeof previous);
        previous_frame = io.frame_number;
        previous_ticks = io.ticks;
    }

    nesturbator_unload_cartridge(inst);
    nesturbator_destroy(inst);
}

/* Every refused call leaves frame_number, ticks and the audio remainder as
   they were (D-07: a failing call changes no state). */
static void test_refusals(void)
{
    nesturbator *inst = make();
    nesturbator_frame io;
    frame_io(&io, video, W, audio, AUDIO_CAP);

    CHECK_EQ_U64(nesturbator_run_frame(NULL, &io), NESTURBATOR_ERR_ARGUMENT);
    CHECK_EQ_U64(nesturbator_run_frame(inst, NULL), NESTURBATOR_ERR_ARGUMENT);

    frame_io(&io, NULL, W, audio, AUDIO_CAP);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_ERR_ARGUMENT);

    frame_io(&io, video, W, NULL, AUDIO_CAP);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_ERR_ARGUMENT);

    frame_io(&io, video, W, audio, AUDIO_CAP);
    io.size = 0u;
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_ERR_STRUCT_SIZE);

    /* Pitch 255 is less than a row: ERR_BUFFER_TOO_SMALL. */
    frame_io(&io, video, 255u, audio, AUDIO_CAP);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_ERR_BUFFER_TOO_SMALL);

    /* The first frame yields 798 samples, so 797 is too small. */
    frame_io(&io, video, W, audio, 797u);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_ERR_BUFFER_TOO_SMALL);
    CHECK_EQ_U64(io.audio_count, 0);

    /* After all of that the instance is still at its start. */
    frame_io(&io, video, W, audio, 798u);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
    CHECK_EQ_U64(io.frame_number, 1);
    CHECK_EQ_U64(io.ticks, 714732);
    CHECK_EQ_U64(io.audio_count, 798);

    /* Frame 2 needs 799 (floor(2*714732*352/315000) = 1597 in total). A
       refusal now must not lose the carried remainder. */
    frame_io(&io, video, W, audio, 798u);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_ERR_BUFFER_TOO_SMALL);
    frame_io(&io, video, W, audio, AUDIO_CAP);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
    CHECK_EQ_U64(io.frame_number, 2);
    CHECK_EQ_U64(io.ticks, 2u * 714732u);
    CHECK_EQ_U64(io.audio_count, 799);

    /* No audio buffer and capacity 0 is accepted as an argument, but no
       frame has 0 samples. */
    frame_io(&io, video, W, NULL, 0u);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_ERR_BUFFER_TOO_SMALL);

    nesturbator_destroy(inst);
}

/* A pitch wider than a row: only columns 0-255 of each row are written. */
static void test_pitch(void)
{
    nesturbator *inst = make();
    for (size_t i = 0; i < WIDE * H; i++) {
        video[i] = POISON;
    }
    nesturbator_frame io;
    frame_io(&io, video, WIDE, audio, AUDIO_CAP);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
    unsigned bad = 0;
    for (uint32_t y = 0; y < H; y++) {
        const uint16_t *row = video + (size_t)y * WIDE;
        for (uint32_t x = 0; x < W; x++) {
            bad += row[x] == POISON;
        }
        for (uint32_t x = W; x < WIDE; x++) {
            bad += row[x] != POISON;
        }
    }
    CHECK_EQ_U64(bad, 0);
    /* The first pixel of the last row lands at 239 * 300. */
    CHECK_EQ_HEX(video[(size_t)239 * WIDE], 0x30);
    nesturbator_destroy(inst);
}

/* D-09: 352 samples per 315000 ticks. 13125 frames are 13125 * 714732 ticks,
   which is 29781 whole periods, so exactly 13125 * 714732 * 352 / 315000 =
   10482736 samples and no remainder. Every sample is silence. */
static void test_audio_period(void)
{
    nesturbator *inst = make();
    uint64_t total = 0;
    unsigned nonzero = 0;
    unsigned out_of_range = 0;
    for (uint32_t f = 0; f < 13125u; f++) {
        for (uint32_t i = 0; i < AUDIO_CAP; i++) {
            audio[i] = (int16_t)POISON;
        }
        nesturbator_frame io;
        frame_io(&io, video2, W, audio, AUDIO_CAP);
        if (nesturbator_run_frame(inst, &io) != NESTURBATOR_OK) {
            CHECK(0);
            break;
        }
        out_of_range += io.audio_count != 798u && io.audio_count != 799u;
        for (uint32_t i = 0; i < io.audio_count && i < AUDIO_CAP; i++) {
            nonzero += audio[i] != 0;
        }
        total += io.audio_count;
    }
    CHECK_EQ_U64(total, 10482736u);
    CHECK_EQ_U64(nonzero, 0);
    CHECK_EQ_U64(out_of_range, 0);
    nesturbator_destroy(inst);
}

/* D-02 boundary pixels: each side of every rule's edge. */
static void test_boundaries(void)
{
    nesturbator *inst = make();
    nesturbator_frame io;
    frame_io(&io, video2, W, audio, AUDIO_CAP);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
#define PX(x, y) video2[(size_t)(y) * W + (x)]
    /* Rule 1: the corner block covers x<8, y<8; (7,7) is its last pixel. */
    CHECK_EQ_HEX(PX(7, 7), 0x30);
    /* Rule 3: (8,7) is just right of the block, inside the red top band. */
    CHECK_EQ_HEX(PX(8, 7), 0x16);
    /* Rule 2: column 0 is border on chart rows too. */
    CHECK_EQ_HEX(PX(0, 8), 0x30);
    /* Rule 4: first chart pixel, b=0 r=0 c=0. */
    CHECK_EQ_HEX(PX(8, 8), 0x00);
    /* Rule 4: x=16 starts column c=1. */
    CHECK_EQ_HEX(PX(16, 8), 0x01);
    /* Rule 4: x=254 is in the last column, c=15, next to the border. */
    CHECK_EQ_HEX(PX(254, 8), 0x0F);
    /* Rule 4: y=35 is y-8=27: b=0, r=3, c=0, the last row of band 0. */
    CHECK_EQ_HEX(PX(8, 35), 0x30);
    /* Rule 4: y=36 is y-8=28: b=1, r=0, c=0, the first row of band 1. */
    CHECK_EQ_HEX(PX(8, 36), 0x40);
    /* Rule 4: y=46 is y-8=38: b=1, r=1; x=100 is c=6. 0x16 | 1<<6. */
    CHECK_EQ_HEX(PX(100, 46), 0x56);
    /* Rule 4: y=231 is y-8=223: b=7, r=3; x=254 is c=15. Last chart pixel. */
    CHECK_EQ_HEX(PX(254, 231), 0x3Fu | (7u << 6));
    /* Rule 3: y=232 starts the blue bottom band. */
    CHECK_EQ_HEX(PX(8, 232), 0x12);
    /* Rule 2: the bottom-right corner is border. */
    CHECK_EQ_HEX(PX(255, 239), 0x30);
#undef PX
    nesturbator_destroy(inst);
}

/* Two instances in one process share nothing: their counters run apart and
   their frames are the same. */
static void test_two_instances(void)
{
    nesturbator *a = make();
    nesturbator *b = make();
    nesturbator_frame ia;
    frame_io(&ia, video, W, audio, AUDIO_CAP);
    nesturbator_frame ib;
    frame_io(&ib, video2, W, audio, AUDIO_CAP);
    for (int i = 0; i < 5; i++) {
        if (i < 2) {
            CHECK_EQ_U64(nesturbator_run_frame(a, &ia), NESTURBATOR_OK);
        }
        CHECK_EQ_U64(nesturbator_run_frame(b, &ib), NESTURBATOR_OK);
    }
    CHECK_EQ_U64(ia.frame_number, 2);
    CHECK_EQ_U64(ia.ticks, 2u * 714732u);
    CHECK_EQ_U64(ib.frame_number, 5);
    CHECK_EQ_U64(ib.ticks, 5u * 714732u);
    CHECK(memcmp(video, video2, sizeof video2) == 0);
    nesturbator_destroy(a);
    nesturbator_destroy(b);
}

/* WR-03: a frame as a newer host would send it, this build's struct followed
   by fields it does not know. The frame is the struct with interior padding,
   so the caller zeroes the whole buffer with memset before setting fields. */
#define TAIL_BYTES 8u
typedef union big_frame {
    nesturbator_frame io;
    unsigned char bytes[sizeof(nesturbator_frame) + TAIL_BYTES];
} big_frame;

static void make_big(big_frame *b)
{
    memset(b, 0, sizeof *b);
    b->io.size = (uint32_t)sizeof b->bytes;
    b->io.video = video2;
    b->io.video_pitch = W;
    b->io.audio = audio;
    b->io.audio_capacity = AUDIO_CAP;
}

static void test_big_frame(void)
{
    nesturbator *inst = make();
    big_frame big;

    /* Every unknown byte zero: accepted. */
    make_big(&big);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &big.io), NESTURBATOR_OK);
    CHECK_EQ_U64(big.io.frame_number, 1);
    CHECK_EQ_U64(big.io.audio_count, 798);

    /* One non-zero byte past this build's struct: refused, no state change. */
    make_big(&big);
    big.bytes[sizeof(nesturbator_frame) + TAIL_BYTES - 1u] = 1u;
    CHECK_EQ_U64(nesturbator_run_frame(inst, &big.io), NESTURBATOR_ERR_STRUCT_SIZE);

    nesturbator_frame io;
    frame_io(&io, video2, W, audio, AUDIO_CAP);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
    CHECK_EQ_U64(io.frame_number, 2);
    nesturbator_destroy(inst);
}

/* PAL uses the pinned exact sample fraction (25-PAL-EVIDENCE.md), with a
   carried per-instance remainder and the unchanged 256x240 output contract. */
static void test_pal_frame_accounting(void)
{
    uint8_t image[16u + INES_PRG_BANK + INES_CHR_BANK];
    const size_t image_size = build_loop_image(image, sizeof image);
    nesturbator *inst = make();
    nesturbator_info info;
    memset(&info, 0, sizeof info);
    info.size = (uint32_t)sizeof info;
    nesturbator_get_info(inst, &info);
    CHECK_EQ_U64(info.fps_num, 39375000u);
    CHECK_EQ_U64(info.fps_den, 655171u);
    CHECK_EQ_U64(info.sample_rate_num, 48000u);
    CHECK_EQ_U64(info.sample_rate_den, 1u);

    CHECK_EQ_U64(nesturbator_load_cartridge_region(inst, image, image_size,
                                                   NESTURBATOR_REGION_PREFERENCE_PAL),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_get_region(inst), NESTURBATOR_REGION_PAL);
    memset(&info, 0, sizeof info);
    info.size = (uint32_t)sizeof info;
    nesturbator_get_info(inst, &info);
    CHECK_EQ_U64(info.width, W);
    CHECK_EQ_U64(info.height, H);
    CHECK_EQ_U64(info.fps_num, 322445u);
    CHECK_EQ_U64(info.fps_den, 6448u);
    CHECK_EQ_U64(info.sample_rate_num, 48000u);
    CHECK_EQ_U64(info.sample_rate_den, 1u);

    for (size_t i = 0u; i < WIDE * H; ++i)
        video[i] = POISON;
    for (size_t i = 0u; i < AUDIO_CAP; ++i)
        audio[i] = (int16_t)POISON;
    nesturbator_frame io;
    frame_io(&io, video, WIDE, audio, 958u);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_ERR_BUFFER_TOO_SMALL);
    CHECK_EQ_U64(io.audio_count, 0u);
    CHECK_EQ_U64(io.frame_number, 0u);
    CHECK_EQ_U64(io.ticks, 0u);
    CHECK_EQ_HEX(video[0], POISON);
    CHECK_EQ_HEX(audio[0], (uint16_t)POISON);

    frame_io(&io, video, WIDE, NULL, 0u);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_ERR_BUFFER_TOO_SMALL);
    frame_io(&io, video, WIDE, NULL, 1u);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_ERR_ARGUMENT);

    frame_io(&io, video, WIDE, audio, 959u);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
    CHECK_EQ_U64(io.audio_count, expected_pal_samples(1u));
    CHECK_EQ_U64(io.frame_number, 1u);
    CHECK_EQ_U64(io.ticks, loop_frame_tick_increment(1063920u, 32u));
    unsigned pitch_errors = 0u;
    unsigned nonzero_samples = 0u;
    for (uint32_t y = 0u; y < H; ++y) {
        for (uint32_t x = 0u; x < W; ++x)
            pitch_errors += video[(size_t)y * WIDE + x] == POISON;
        for (uint32_t x = W; x < WIDE; ++x)
            pitch_errors += video[(size_t)y * WIDE + x] != POISON;
    }
    for (uint32_t i = 0u; i < io.audio_count; ++i)
        nonzero_samples += audio[i] != 0;
    CHECK_EQ_U64(pitch_errors, 0u);
    CHECK_EQ_U64(nonzero_samples, 0u);

    CHECK_EQ_U64(nesturbator_reset(inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_get_region(inst), NESTURBATOR_REGION_PAL);
    uint64_t previous_ticks = io.ticks + 7u * 32u;
    frame_io(&io, video2, W, audio, AUDIO_CAP);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
    CHECK_EQ_U64(io.audio_count, expected_pal_samples(2u) - expected_pal_samples(1u));
    CHECK_EQ_U64(io.frame_number, 2u);
    CHECK_EQ_U64(io.ticks, next_pal_frame_ticks(2u, previous_ticks));

    uint8_t invalid_image[4] = {0u, 0u, 0u, 0u};
    CHECK_EQ_U64(nesturbator_load_cartridge_region(inst, invalid_image, sizeof invalid_image,
                                                   NESTURBATOR_REGION_PREFERENCE_PAL),
                 NESTURBATOR_ERR_CARTRIDGE);
    CHECK_EQ_U64(nesturbator_get_region(inst), NESTURBATOR_REGION_PAL);
    previous_ticks = io.ticks;
    frame_io(&io, video2, W, audio, AUDIO_CAP);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
    CHECK_EQ_U64(io.audio_count, expected_pal_samples(3u) - expected_pal_samples(2u));
    CHECK_EQ_U64(io.frame_number, 3u);
    CHECK_EQ_U64(io.ticks, next_pal_frame_ticks(3u, previous_ticks));

    /* A successful reload starts a fresh PAL sample remainder. */
    CHECK_EQ_U64(nesturbator_load_cartridge_region(inst, image, image_size,
                                                   NESTURBATOR_REGION_PREFERENCE_PAL),
                 NESTURBATOR_OK);
    frame_io(&io, video2, W, audio, AUDIO_CAP);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
    CHECK_EQ_U64(io.audio_count, expected_pal_samples(1u));
    CHECK_EQ_U64(io.frame_number, 1u);
    CHECK_EQ_U64(io.ticks, loop_frame_tick_increment(1063920u, 32u));
    nesturbator_destroy(inst);
}

/* Run the full reduced rational remainder period as arithmetic, then compare
   independent NTSC/PAL guests while advancing the instances in turn. */
static void test_pal_rational_period_and_interleaving(void)
{
    uint64_t total = 0u;
    uint64_t remainder = 0u;
    unsigned out_of_range = 0u;
    for (uint64_t frame = 0u; frame < 64489u; ++frame) {
        uint64_t accumulated = remainder + UINT64_C(1063920) * UINT64_C(640);
        uint32_t count = (uint32_t)(accumulated / UINT64_C(709379));
        remainder = accumulated % UINT64_C(709379);
        total += count;
        out_of_range += count != 959u && count != 960u;
    }
    CHECK_EQ_U64(total, UINT64_C(64489) * UINT64_C(48000) * UINT64_C(6448) / UINT64_C(322445));
    CHECK_EQ_U64(total, 61900800u);
    CHECK_EQ_U64(remainder, 0u);
    CHECK_EQ_U64(out_of_range, 0u);

    uint8_t image[16u + INES_PRG_BANK + INES_CHR_BANK];
    const size_t image_size = build_loop_image(image, sizeof image);
    nesturbator *ntsc = make();
    nesturbator *pal = make();
    CHECK_EQ_U64(nesturbator_load_cartridge_region(ntsc, image, image_size,
                                                   NESTURBATOR_REGION_PREFERENCE_NTSC),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_cartridge_region(pal, image, image_size,
                                                   NESTURBATOR_REGION_PREFERENCE_PAL),
                 NESTURBATOR_OK);
    uint64_t ntsc_ticks = 0u;
    uint64_t pal_ticks = 0u;
    for (uint64_t frame = 0u; frame < 5u; ++frame) {
        nesturbator_frame ntsc_io;
        frame_io(&ntsc_io, video, W, audio, AUDIO_CAP);
        CHECK_EQ_U64(nesturbator_run_frame(ntsc, &ntsc_io), NESTURBATOR_OK);
        CHECK_EQ_U64(ntsc_io.audio_count,
                     expected_ntsc_samples(frame + 1u) - expected_ntsc_samples(frame));
        ntsc_ticks += loop_frame_tick_increment(714732u, 24u);
        CHECK_EQ_U64(ntsc_io.ticks, ntsc_ticks);

        nesturbator_frame pal_io;
        frame_io(&pal_io, video2, W, audio, AUDIO_CAP);
        CHECK_EQ_U64(nesturbator_run_frame(pal, &pal_io), NESTURBATOR_OK);
        CHECK_EQ_U64(pal_io.audio_count,
                     expected_pal_samples(frame + 1u) - expected_pal_samples(frame));
        pal_ticks = next_pal_frame_ticks((uint32_t)(frame + 1u), pal_ticks);
        CHECK_EQ_U64(pal_io.ticks, pal_ticks);
    }
    nesturbator_destroy(ntsc);
    nesturbator_destroy(pal);
}

int main(void)
{
    test_refusals();
    test_looping_guest_progress();
    test_pitch();
    test_audio_period();
    test_boundaries();
    test_two_instances();
    test_big_frame();
    test_pal_frame_accounting();
    test_pal_rational_period_and_interleaving();
    CHECK_DONE();
}

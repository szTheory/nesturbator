/* D-11: run_frame checks, buffer rules, the D-09 audio period, the D-02 test
   card's boundary pixels and two independent instances, through the public
   header only. */
#include <stdint.h>
#include <string.h>

#include "nesturbator.h"
#include "../check.h"

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
    nesturbator_config cfg = NESTURBATOR_CONFIG_INIT;
    nesturbator *inst = NULL;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    return inst;
}

static nesturbator_frame frame_io(uint16_t *v, uint32_t pitch, int16_t *a, uint32_t cap)
{
    nesturbator_frame io;
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = v;
    io.video_pitch = pitch;
    io.audio = a;
    io.audio_capacity = cap;
    return io;
}

/* Every refused call leaves frame_number, ticks and the audio remainder as
   they were (D-07: a failing call changes no state). */
static void test_refusals(void)
{
    nesturbator *inst = make();
    nesturbator_frame io = frame_io(video, W, audio, AUDIO_CAP);

    CHECK_EQ_U64(nesturbator_run_frame(NULL, &io), NESTURBATOR_ERR_ARGUMENT);
    CHECK_EQ_U64(nesturbator_run_frame(inst, NULL), NESTURBATOR_ERR_ARGUMENT);

    io = frame_io(NULL, W, audio, AUDIO_CAP);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_ERR_ARGUMENT);

    io = frame_io(video, W, NULL, AUDIO_CAP);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_ERR_ARGUMENT);

    io = frame_io(video, W, audio, AUDIO_CAP);
    io.size = 0u;
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_ERR_STRUCT_SIZE);

    /* Pitch 255 is less than a row: ERR_BUFFER_TOO_SMALL. */
    io = frame_io(video, 255u, audio, AUDIO_CAP);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_ERR_BUFFER_TOO_SMALL);

    /* The first frame yields 798 samples, so 797 is too small. */
    io = frame_io(video, W, audio, 797u);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_ERR_BUFFER_TOO_SMALL);
    CHECK_EQ_U64(io.audio_count, 0);

    /* After all of that the instance is still at its start. */
    io = frame_io(video, W, audio, 798u);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
    CHECK_EQ_U64(io.frame_number, 1);
    CHECK_EQ_U64(io.ticks, 714732);
    CHECK_EQ_U64(io.audio_count, 798);

    /* Frame 2 needs 799 (floor(2*714732*352/315000) = 1597 in total). A
       refusal now must not lose the carried remainder. */
    io = frame_io(video, W, audio, 798u);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_ERR_BUFFER_TOO_SMALL);
    io = frame_io(video, W, audio, AUDIO_CAP);
    CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
    CHECK_EQ_U64(io.frame_number, 2);
    CHECK_EQ_U64(io.ticks, 2u * 714732u);
    CHECK_EQ_U64(io.audio_count, 799);

    /* No audio buffer and capacity 0 is accepted as an argument, but no
       frame has 0 samples. */
    io = frame_io(video, W, NULL, 0u);
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
    nesturbator_frame io = frame_io(video, WIDE, audio, AUDIO_CAP);
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
        nesturbator_frame io = frame_io(video2, W, audio, AUDIO_CAP);
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
    nesturbator_frame io = frame_io(video2, W, audio, AUDIO_CAP);
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
    nesturbator_frame ia = frame_io(video, W, audio, AUDIO_CAP);
    nesturbator_frame ib = frame_io(video2, W, audio, AUDIO_CAP);
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

int main(void)
{
    test_refusals();
    test_pitch();
    test_audio_period();
    test_boundaries();
    test_two_instances();
    CHECK_DONE();
}

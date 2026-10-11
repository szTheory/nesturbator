/* PLAY-01: nes-runner plays from boot into scoring play when a movie drives
   it, and the same check fails for every control (D-08 to D-13).

   The program links the library, loads tests/roms/runner.nes, replays
   generated NMOVIE1 files through nesturbator_set_input and observes the game
   with nesturbator_peek_cpu_ram. It adds no runner flag and no public API.

   RAM addresses (D-10), from the nes-runner v1 sources (MIT) and confirmed on
   the release ROM. Source files named by the labels in that tree:
     $3E        current_gamestate (main game file): 0 title, 1 credits, 2 game,
                3 dead, 4 highscore, 5 seed
     $5D..$60   PlayerScore3..0 (score file): base-100 digits, one binary byte
                each, $60 the lowest; the score rises by 1 per 8-pixel column
                crossed while alive in game
     $4D, $50..$53  title menu layout (title file): after title init
                $4D == 4 and $50..$53 == 02 05 04 01
     $61..$68   level buffer: all $F0 once a game has started
   Never read, because they change without input: $B2 (sound driver frame
   counter), $20, $1D, $27 (calc_scroll), $4E, $4F, the stack page $0100-$01FF
   and rng_seed (its address was not re-derived; the closed ram_reads table
   keeps it out whether or not it is known). The test enforces this at run
   time: every single-address read goes through ram_read, which accepts only
   addresses in ram_reads, and check_read_table rejects a table entry that is
   in never_read or in the stack page. The one bulk read is ram_digest, the
   whole-RAM equality digest that the D-12 causality check needs. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "audio_hash.h"
#include "check.h"
#include "movie.h"
#include "nesturbator.h"
#include "sha256.h"

#define ROM_SHA256 "1a67bfabdde2d96eacb04eb7f9d17e8da6c082d4976c318315fe31e2d19b4384"

#define FRAMES 600u
#define WINDOW 30u
#define CHECKPOINTS (FRAMES / WINDOW) /* frames 30, 60, ..., 600 */
#define PIXELS (256u * 240u)
#define AUDIO_CAPACITY 800u

#define ADDR_STATE 0x3Eu
#define ADDR_MENU_COUNT 0x4Du
#define ADDR_MENU_FIRST 0x50u
#define ADDR_SCORE_HIGH 0x5Du /* $5D..$60, most significant first */
#define ADDR_LEVEL_FIRST 0x61u
#define STATE_GAME 2u

/* The only single addresses the test may read. */
static const uint16_t ram_reads[] = {0x3E, 0x4D, 0x50, 0x51, 0x52, 0x53, 0x5D, 0x5E, 0x5F,
                                     0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68};

/* Addresses that must never be read (they move without input). */
static const uint16_t never_read[] = {0xB2, 0x20, 0x1D, 0x27, 0x4E, 0x4F};

struct trace {
    uint32_t frame_count;
    uint8_t state[CHECKPOINTS];        /* $3E at each checkpoint */
    uint32_t score[CHECKPOINTS];       /* score at each checkpoint */
    uint32_t changed[CHECKPOINTS];     /* pixels that differ from WINDOW frames earlier */
    int32_t p2p[CHECKPOINTS];          /* PCM peak-to-peak of the window ending at the checkpoint */
    char window_hash[CHECKPOINTS][65]; /* PCM sha256 of that window */
    uint64_t generation[CHECKPOINTS];  /* nesturbator_save_generation at each checkpoint */
    uint64_t ram_digest[FRAMES + 1u];  /* digest of the 2 KiB RAM after each frame */
    uint64_t frame_digest[FRAMES + 1u]; /* digest of the video after each frame (log only) */
    uint32_t first_input;               /* first frame with nonzero input, 0 if none */
    uint16_t snap[2][PIXELS];           /* video at frames 300 and 600 */
    uint8_t sram[2];                    /* battery span bytes 0 and 1 at the end */
    uint32_t sram_valid;                /* 1 when the cartridge has a battery span */
};

static int in_table(const uint16_t *table, size_t count, uint16_t addr)
{
    for (size_t i = 0u; i < count; i++)
        if (table[i] == addr)
            return 1;
    return 0;
}

static void check_read_table(void)
{
    for (size_t i = 0u; i < sizeof ram_reads / sizeof ram_reads[0]; i++) {
        CHECK(!in_table(never_read, sizeof never_read / sizeof never_read[0], ram_reads[i]));
        CHECK(ram_reads[i] < 0x0100u || ram_reads[i] > 0x01FFu);
    }
}

/* Every single-address read goes through here, so the table holds whether a
   call site spells the address as hex, decimal or a named constant. */
static uint8_t ram_read(const nesturbator *inst, uint16_t addr)
{
    uint8_t value = 0u;
    int allowed = in_table(ram_reads, sizeof ram_reads / sizeof ram_reads[0], addr);
    CHECK(allowed);
    if (!allowed)
        return 0u;
    CHECK_EQ_U64(nesturbator_peek_cpu_ram(inst, addr, &value), NESTURBATOR_OK);
    return value;
}

/* D-12 requires comparing the whole 2 KiB of RAM between two runs to find the
   first frame where they differ. This is the one bulk read: the result is a
   digest used only for equality, and no predicate takes any single address's
   value from it. */
static uint64_t ram_digest(const nesturbator *inst)
{
    uint64_t digest = UINT64_C(14695981039346656037); /* FNV-1a offset basis */
    for (uint16_t addr = 0u; addr < 0x0800u; addr++) {
        uint8_t value = 0u;
        CHECK_EQ_U64(nesturbator_peek_cpu_ram(inst, addr, &value), NESTURBATOR_OK);
        digest = (digest ^ value) * UINT64_C(1099511628211);
    }
    return digest;
}

static uint64_t video_digest(const uint16_t *video)
{
    uint64_t digest = UINT64_C(14695981039346656037);
    for (uint32_t i = 0u; i < PIXELS; i++)
        digest = (digest ^ video[i]) * UINT64_C(1099511628211);
    return digest;
}

static uint32_t read_score(const nesturbator *inst)
{
    uint32_t score = 0u;
    for (uint16_t i = 0u; i < 4u; i++)
        score = score * 100u + ram_read(inst, (uint16_t)(ADDR_SCORE_HIGH + i));
    return score;
}

static void check_layout_canary(const nesturbator *inst)
{
    static const uint8_t menu[4] = {0x02, 0x05, 0x04, 0x01};
    CHECK_EQ_U64(ram_read(inst, ADDR_MENU_COUNT), 4u);
    for (uint16_t i = 0u; i < 4u; i++)
        CHECK_EQ_U64(ram_read(inst, (uint16_t)(ADDR_MENU_FIRST + i)), menu[i]);
}

static void check_level_canary(const nesturbator *inst)
{
    for (uint16_t i = 0u; i < 8u; i++)
        CHECK_EQ_U64(ram_read(inst, (uint16_t)(ADDR_LEVEL_FIRST + i)), 0xF0u);
}

/* Runs FRAMES frames on a fresh instance. A NULL movie means zero input.
   Returns the status of nesturbator_load_cartridge; the instance keeps running
   after a failed load, the way a host that ignores the error would. The layout
   canary is asserted at frame 4 on every run that loaded the cartridge (all
   scenarios are input-free until frame 61); the level canary at frame 135 on
   the run named positive. */
static nesturbator_status collect_run(const uint8_t *rom, size_t size,
                                      const nesturbator_movie *movie, int positive, struct trace *t)
{
    static uint16_t video[PIXELS];
    static uint16_t previous[PIXELS];
    static int16_t audio[AUDIO_CAPACITY];
    nesturbator_config config;
    nesturbator *inst = NULL;
    nesturbator_status loaded;
    nesturbator_run_audio_hash window;
    int32_t low = 0;
    int32_t high = 0;

    memset(t, 0, sizeof *t);
    memset(previous, 0, sizeof previous);
    memset(&config, 0, sizeof config);
    config.size = (uint32_t)sizeof config;
    config.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&config, &inst), NESTURBATOR_OK);
    if (inst == NULL)
        return NESTURBATOR_ERR_ARGUMENT;
    loaded = nesturbator_load_cartridge(inst, rom, size);
    nesturbator_run_audio_hash_init(&window);

    for (uint32_t frame = 1u; frame <= FRAMES; frame++) {
        nesturbator_input input;
        nesturbator_frame io;
        uint32_t checkpoint = frame / WINDOW - 1u;

        memset(&input, 0, sizeof input);
        input.size = (uint32_t)sizeof input;
        if (movie != NULL && frame <= movie->frame_count) {
            input.buttons[0] = movie->masks[2u * (frame - 1u)];
            input.buttons[1] = movie->masks[2u * (frame - 1u) + 1u];
        }
        if ((input.buttons[0] | input.buttons[1]) != 0u && t->first_input == 0u)
            t->first_input = frame;
        CHECK_EQ_U64(nesturbator_set_input(inst, &input), NESTURBATOR_OK);

        memset(&io, 0, sizeof io);
        io.size = (uint32_t)sizeof io;
        io.video = video;
        io.video_pitch = 256u;
        io.audio = audio;
        io.audio_capacity = AUDIO_CAPACITY;
        CHECK_EQ_U64(nesturbator_run_frame(inst, &io), NESTURBATOR_OK);
        CHECK_EQ_U64(io.frame_number, frame);

        nesturbator_run_audio_hash_pcm(&window, audio, io.audio_count);
        for (uint32_t i = 0u; i < io.audio_count; i++) {
            if ((frame - 1u) % WINDOW == 0u && i == 0u)
                low = high = audio[0];
            if (audio[i] < low)
                low = audio[i];
            if (audio[i] > high)
                high = audio[i];
        }
        t->ram_digest[frame] = ram_digest(inst);
        t->frame_digest[frame] = video_digest(video);

        if (loaded == NESTURBATOR_OK && frame == 4u)
            check_layout_canary(inst);
        if (loaded == NESTURBATOR_OK && positive && frame == 135u)
            check_level_canary(inst);
        if (frame == 300u)
            memcpy(t->snap[0], video, sizeof video);
        if (frame == 600u)
            memcpy(t->snap[1], video, sizeof video);

        if (frame % WINDOW == 0u) {
            char transitions[65];
            uint32_t changed = 0u;
            for (uint32_t i = 0u; i < PIXELS; i++)
                changed += video[i] != previous[i];
            memcpy(previous, video, sizeof video);
            t->changed[checkpoint] = changed;
            t->p2p[checkpoint] = high - low;
            nesturbator_run_audio_hash_final(&window, transitions, t->window_hash[checkpoint]);
            nesturbator_run_audio_hash_init(&window);
            t->generation[checkpoint] = nesturbator_save_generation(inst);
            if (loaded == NESTURBATOR_OK) {
                t->state[checkpoint] = ram_read(inst, ADDR_STATE);
                t->score[checkpoint] = read_score(inst);
            }
        }
        t->frame_count = frame;
    }

    if (loaded == NESTURBATOR_OK) {
        uint8_t *span = NULL;
        size_t span_size = 0u;
        CHECK_EQ_U64(nesturbator_get_memory(inst, NESTURBATOR_MEMORY_SAVE_RAM, &span, &span_size),
                     NESTURBATOR_OK);
        if (span != NULL && span_size >= 2u) {
            t->sram[0] = span[0];
            t->sram[1] = span[1];
            t->sram_valid = 1u;
        }
    }
    nesturbator_destroy(inst);
    return loaded;
}

static uint8_t *read_file(const char *path, size_t *size)
{
    FILE *file = fopen(path, "rb");
    uint8_t *bytes = NULL;
    long length;

    *size = 0u;
    if (file == NULL)
        return NULL;
    if (fseek(file, 0, SEEK_END) != 0 || (length = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    bytes = (uint8_t *)malloc((size_t)length);
    if (bytes != NULL && fread(bytes, 1u, (size_t)length, file) != (size_t)length) {
        free(bytes);
        bytes = NULL;
    }
    fclose(file);
    if (bytes != NULL)
        *size = (size_t)length;
    return bytes;
}

static int read_scenario(const char *dir, const char *name, nesturbator_movie *movie)
{
    char path[1024];
    int n = snprintf(path, sizeof path, "%s/nesrunner-%s.nmovie", dir, name);
    if (n <= 0 || (size_t)n >= sizeof path)
        return 0;
    return nesturbator_movie_read(path, movie) != 0 && movie->frame_count == FRAMES;
}

static int rom_matches(const uint8_t *rom, size_t size)
{
    nesturbator_run_sha256 hash;
    uint8_t digest[32];
    char hex[65];

    nesturbator_run_sha256_init(&hash);
    nesturbator_run_sha256_update(&hash, rom, size);
    nesturbator_run_sha256_final(&hash, digest);
    nesturbator_run_sha256_hex(digest, hex);
    return size == 81936u && strcmp(hex, ROM_SHA256) == 0;
}

static struct trace pos_trace;

int main(int argc, char **argv)
{
    size_t rom_size = 0u;
    uint8_t *rom;
    nesturbator_movie pos;

    if (argc != 3) {
        fprintf(stderr, "usage: runner.boot_to_play <rom> <movie dir>\n");
        return 2;
    }
    check_read_table();
    rom = read_file(argv[1], &rom_size);
    CHECK(rom != NULL);
    if (rom == NULL)
        CHECK_DONE();
    CHECK(rom_matches(rom, rom_size));
    if (check_failures != 0)
        CHECK_DONE();
    CHECK(read_scenario(argv[2], "pos", &pos));
    if (check_failures != 0)
        CHECK_DONE();

    CHECK_EQ_U64(collect_run(rom, rom_size, &pos, 1, &pos_trace), NESTURBATOR_OK);

    /* P1 facts on the positive run. */
    for (uint32_t c = 150u; c <= FRAMES; c += WINDOW) {
        uint32_t i = c / WINDOW - 1u;
        CHECK_EQ_U64(pos_trace.state[i], STATE_GAME);
        if (c >= 210u)
            CHECK(pos_trace.score[i] > pos_trace.score[i - 2u]);
    }
    printf("positive final score %u\n", (unsigned)pos_trace.score[CHECKPOINTS - 1u]);
    CHECK(pos_trace.score[CHECKPOINTS - 1u] >= 36u);

    /* D-07: the game wrote its save from fresh SRAM. */
    CHECK_EQ_U64(pos_trace.sram_valid, 1u);
    CHECK_EQ_HEX(pos_trace.sram[0], 0x5Au);
    CHECK_EQ_HEX(pos_trace.sram[1], 0x6Fu);
    CHECK(pos_trace.generation[CHECKPOINTS - 1u] >= 2u);

    nesturbator_movie_free(&pos);
    free(rom);
    CHECK_DONE();
}

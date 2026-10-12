/* STATE-01, STATE-02 (D-01 to D-08, D-16): save states of the library. The tracer saves dabg.nes
   (NROM) at frame 60, loads the state into a fresh instance and checks that the next 60 frames,
   their audio, their ticks and the final state bytes equal the uninterrupted run's. argv[1] is
   tests/roms/dabg.nes. White box: includes internal.h to compare whole instances.

   The st_* helpers are file-local on purpose; a later plan moves them unchanged into a shared
   header for the fork and sweep tests. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"

/* ---- Helpers ---- */

static nesturbator *st_make_instance(void)
{
    nesturbator_config cfg;
    nesturbator *inst = NULL;
    memset(&cfg, 0, sizeof cfg);
    cfg.size = (uint32_t)sizeof cfg;
    cfg.abi = NESTURBATOR_ABI_VERSION;
    CHECK_EQ_U64(nesturbator_create(&cfg, &inst), NESTURBATOR_OK);
    return inst;
}

/* Reads a whole file; returns 1 on success. The caller frees *data. */
static int st_read_file(const char *path, uint8_t **data, size_t *size)
{
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    long len;
    if (f == NULL)
        return 0;
    if (fseek(f, 0, SEEK_END) != 0 || (len = ftell(f)) <= 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return 0;
    }
    buf = (uint8_t *)malloc((size_t)len);
    if (buf == NULL || fread(buf, 1u, (size_t)len, f) != (size_t)len) {
        free(buf);
        fclose(f);
        return 0;
    }
    fclose(f);
    *data = buf;
    *size = (size_t)len;
    return 1;
}

static void st_load(nesturbator *inst, const uint8_t *rom, size_t size)
{
    CHECK_EQ_U64(nesturbator_load_cartridge(inst, rom, size), NESTURBATOR_OK);
}

/* One frame with a 256-pitch video buffer and a 1024-sample audio buffer. The digests are the
   CRC-32 of the video pixels and of the audio samples, each serialised little-endian. */
static nesturbator_status st_frame(nesturbator *inst, uint16_t buttons, uint32_t *video_crc,
                                   uint32_t *audio_crc, uint64_t *ticks, uint64_t *frame_number)
{
    static uint16_t video[256u * 240u];
    static int16_t audio[1024];
    nesturbator_input in;
    nesturbator_frame io;
    nesturbator_status st;
    uint8_t bytes[2];
    uint32_t crc;
    memset(&in, 0, sizeof in);
    in.size = (uint32_t)sizeof in;
    in.buttons[0] = (uint8_t)(buttons & 0xffu);
    in.buttons[1] = (uint8_t)(buttons >> 8);
    CHECK_EQ_U64(nesturbator_set_input(inst, &in), NESTURBATOR_OK);
    memset(&io, 0, sizeof io);
    io.size = (uint32_t)sizeof io;
    io.video = video;
    io.video_pitch = 256u;
    io.audio = audio;
    io.audio_capacity = 1024u;
    st = nesturbator_run_frame(inst, &io);
    if (st != NESTURBATOR_OK)
        return st;
    crc = 0u;
    for (size_t i = 0u; i < 256u * 240u; ++i) {
        bytes[0] = (uint8_t)(video[i] & 0xffu);
        bytes[1] = (uint8_t)(video[i] >> 8);
        crc = nesturbator__crc32(crc, bytes, 2u);
    }
    if (video_crc != NULL)
        *video_crc = crc;
    crc = 0u;
    for (uint32_t i = 0u; i < io.audio_count; ++i) {
        bytes[0] = (uint8_t)((uint16_t)audio[i] & 0xffu);
        bytes[1] = (uint8_t)((uint16_t)audio[i] >> 8);
        crc = nesturbator__crc32(crc, bytes, 2u);
    }
    if (audio_crc != NULL)
        *audio_crc = crc;
    if (ticks != NULL)
        *ticks = io.ticks;
    if (frame_number != NULL)
        *frame_number = io.frame_number;
    return st;
}

/* A copy of the whole instance struct plus the cartridge RAM it points to, for "unchanged". */
struct st_snap {
    struct nesturbator nes;
    uint8_t *prg_ram;
    size_t prg_ram_size;
    uint8_t *chr_ram;
    size_t chr_ram_size;
};

static void st_snapshot(const nesturbator *inst, struct st_snap *s)
{
    memcpy(&s->nes, inst, sizeof s->nes);
    s->prg_ram_size = inst->cart.prg_ram_size;
    s->prg_ram = NULL;
    if (s->prg_ram_size != 0u) {
        s->prg_ram = (uint8_t *)malloc(s->prg_ram_size);
        memcpy(s->prg_ram, inst->cart.prg_ram, s->prg_ram_size);
    }
    s->chr_ram_size = inst->cart.chr_is_ram != 0u ? inst->cart.chr_size : 0u;
    s->chr_ram = NULL;
    if (s->chr_ram_size != 0u) {
        s->chr_ram = (uint8_t *)malloc(s->chr_ram_size);
        memcpy(s->chr_ram, inst->cart.chr, s->chr_ram_size);
    }
}

/* 1 when the instance is byte for byte what the snapshot recorded. */
static int st_same(const nesturbator *inst, const struct st_snap *s)
{
    if (memcmp(&s->nes, inst, sizeof s->nes) != 0)
        return 0;
    if (s->prg_ram_size != 0u && memcmp(s->prg_ram, inst->cart.prg_ram, s->prg_ram_size) != 0)
        return 0;
    if (s->chr_ram_size != 0u && memcmp(s->chr_ram, inst->cart.chr, s->chr_ram_size) != 0)
        return 0;
    return 1;
}

static void st_snap_free(struct st_snap *s)
{
    free(s->prg_ram);
    free(s->chr_ram);
    s->prg_ram = NULL;
    s->chr_ram = NULL;
}

/* The button pattern both runs use for frame n. */
static uint16_t st_buttons(uint32_t n)
{
    return (uint16_t)((n * 37u + (n >> 3)) & 0xffu);
}

/* ---- Cases ---- */

#define TRACER_FRAMES 60u

/* Tracer: save dabg at frame 60, load it into a fresh instance, run the next 60 frames on both,
   and compare every digest, every tick count, every frame number and the final state bytes. */
static void tracer_round_trip(const uint8_t *rom, size_t rom_size)
{
    uint32_t video_a[TRACER_FRAMES], audio_a[TRACER_FRAMES];
    uint64_t ticks_a[TRACER_FRAMES], frame_a[TRACER_FRAMES];
    nesturbator *a = st_make_instance();
    nesturbator *b = st_make_instance();
    size_t size;
    uint8_t *state, *final_a, *final_b;
    struct st_snap snap;
    st_load(a, rom, rom_size);
    for (uint32_t n = 1u; n <= TRACER_FRAMES; ++n)
        CHECK_EQ_U64(st_frame(a, st_buttons(n), NULL, NULL, NULL, NULL), NESTURBATOR_OK);
    size = nesturbator_state_size(a);
    CHECK(size > 64u);
    state = (uint8_t *)malloc(size);
    final_a = (uint8_t *)malloc(size);
    final_b = (uint8_t *)malloc(size);
    st_snapshot(a, &snap);
    CHECK_EQ_U64(nesturbator_save_state(a, state, size), NESTURBATOR_OK);
    /* D-01: save takes a const instance and changes nothing. */
    CHECK(st_same(a, &snap));
    st_snap_free(&snap);
    for (uint32_t i = 0u; i < TRACER_FRAMES; ++i)
        CHECK_EQ_U64(st_frame(a, st_buttons(TRACER_FRAMES + 1u + i), &video_a[i], &audio_a[i],
                              &ticks_a[i], &frame_a[i]),
                     NESTURBATOR_OK);

    st_load(b, rom, rom_size);
    CHECK_EQ_U64(nesturbator_load_state(b, state, size), NESTURBATOR_OK);
    for (uint32_t i = 0u; i < TRACER_FRAMES; ++i) {
        uint32_t video, audio;
        uint64_t ticks, frame_number;
        CHECK_EQ_U64(st_frame(b, st_buttons(TRACER_FRAMES + 1u + i), &video, &audio, &ticks,
                              &frame_number),
                     NESTURBATOR_OK);
        CHECK_EQ_HEX(video, video_a[i]);
        CHECK_EQ_HEX(audio, audio_a[i]);
        CHECK_EQ_U64(ticks, ticks_a[i]);
        CHECK_EQ_U64(frame_number, frame_a[i]);
        CHECK_EQ_U64(frame_number, TRACER_FRAMES + 1u + i);
    }
    CHECK_EQ_U64(nesturbator_save_state(a, final_a, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(b, final_b, size), NESTURBATOR_OK);
    CHECK(memcmp(final_a, final_b, size) == 0);
    free(state);
    free(final_a);
    free(final_b);
    nesturbator_destroy(a);
    nesturbator_destroy(b);
}

int main(int argc, char **argv)
{
    uint8_t *rom = NULL;
    size_t rom_size = 0u;
    if (argc < 2 || !st_read_file(argv[1], &rom, &rom_size)) {
        fprintf(stderr, "usage: core.state dabg.nes\n");
        return 2;
    }
    tracer_round_trip(rom, rom_size);
    free(rom);
    CHECK_DONE();
}

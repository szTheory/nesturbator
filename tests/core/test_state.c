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


/* Little-endian readers for the documented header, independent of the library's own code. */
static uint32_t le16(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8);
}

static uint32_t le32(const uint8_t *p)
{
    return le16(p) | (le16(p + 2) << 16);
}

static uint64_t le64(const uint8_t *p)
{
    return (uint64_t)le32(p) | ((uint64_t)le32(p + 4) << 32);
}

static void put32(uint8_t *p, uint32_t v)
{
    for (unsigned i = 0u; i < 4u; ++i)
        p[i] = (uint8_t)(v >> (8u * i));
}

/* Recomputes the state CRC-32 (every byte but offsets 28 to 31) and stores it at 28, so a test
   can change a field and still pass the checksum step. */
static void st_recrc(uint8_t *state, size_t size)
{
    uint32_t crc = nesturbator__crc32(0u, state, 28u);
    crc = nesturbator__crc32(crc, state + 32, size - 32u);
    put32(state + 28, crc);
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

/* A dabg instance at frame 60 and its state; shared by the cases below. */
struct fixture {
    nesturbator *inst;
    size_t size;
    uint8_t *state;
    uint64_t frame_number, ticks;
};

static void fixture_make(struct fixture *fx, const uint8_t *rom, size_t rom_size)
{
    fx->inst = st_make_instance();
    st_load(fx->inst, rom, rom_size);
    for (uint32_t n = 1u; n <= TRACER_FRAMES; ++n)
        CHECK_EQ_U64(st_frame(fx->inst, st_buttons(n), NULL, NULL, &fx->ticks, &fx->frame_number),
                     NESTURBATOR_OK);
    fx->size = nesturbator_state_size(fx->inst);
    fx->state = (uint8_t *)malloc(fx->size);
    CHECK_EQ_U64(nesturbator_save_state(fx->inst, fx->state, fx->size), NESTURBATOR_OK);
}

static void fixture_free(struct fixture *fx)
{
    free(fx->state);
    nesturbator_destroy(fx->inst);
}

/* CRC vector (IEEE 802.3): the check value, the empty input and the update property. */
static void crc_vector(void)
{
    static const uint8_t digits[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    CHECK_EQ_HEX(nesturbator__crc32(0u, digits, 9u), 0xcbf43926u);
    CHECK_EQ_HEX(nesturbator__crc32(0u, NULL, 0u), 0u);
    CHECK_EQ_HEX(nesturbator__crc32(nesturbator__crc32(0u, digits, 4u), digits + 4, 5u),
                 0xcbf43926u);
}

/* STATE-02: the documented header, byte by byte. */
static void header_layout(const struct fixture *fx)
{
    static const uint8_t magic[8] = {0x89u, 0x4eu, 0x54u, 0x53u, 0x0du, 0x0au, 0x1au, 0x0au};
    const uint8_t *s = fx->state;
    uint32_t crc;
    CHECK(memcmp(s, magic, sizeof magic) == 0);
    CHECK_EQ_U64(le16(s + 8), NESTURBATOR_STATE_FORMAT_VERSION);
    CHECK_EQ_U64(le16(s + 10), 64u);
    CHECK_EQ_U64(le32(s + 12), fx->size);
    CHECK_EQ_U64(le32(s + 16), NESTURBATOR_BEHAVIOUR_REVISION);
    CHECK_EQ_U64(le16(s + 20), 0u); /* dabg is mapper 0 */
    CHECK_EQ_U64(s[22], 0u);
    CHECK_EQ_U64(s[23], 0u);
    CHECK_EQ_HEX(le32(s + 24), ((const struct nesturbator *)fx->inst)->cart.rom_crc32);
    crc = nesturbator__crc32(0u, s, 28u);
    crc = nesturbator__crc32(crc, s + 32, fx->size - 32u);
    CHECK_EQ_HEX(le32(s + 28), crc);
    CHECK_EQ_U64(le64(s + 32), fx->frame_number);
    CHECK_EQ_U64(le64(s + 40), fx->ticks);
    for (size_t i = 48u; i < 64u; ++i)
        CHECK_EQ_U64(s[i], 0u);
    /* The ROM CRC-32 is of the file's PRG and CHR ROM, not of the whole file: it is not zero. */
    CHECK(le32(s + 24) != 0u);
}

/* D-05: save checks in order and writes only on OK. */
static void save_order(const struct fixture *fx)
{
    uint8_t *buf = (uint8_t *)malloc(fx->size + 32u);
    nesturbator *empty = st_make_instance();
    memset(buf, 0xcc, fx->size + 32u);
    CHECK_EQ_U64(nesturbator_save_state(NULL, buf, fx->size), NESTURBATOR_ERR_ARGUMENT);
    CHECK_EQ_U64(nesturbator_save_state(fx->inst, NULL, fx->size), NESTURBATOR_ERR_ARGUMENT);
    /* No cartridge: the cartridge check comes before the capacity check. */
    CHECK_EQ_U64(nesturbator_save_state(empty, buf, 0u), NESTURBATOR_ERR_STATE_CARTRIDGE);
    CHECK_EQ_U64(nesturbator_save_state(fx->inst, buf, fx->size - 1u),
                 NESTURBATOR_ERR_BUFFER_TOO_SMALL);
    CHECK_EQ_U64(nesturbator_save_state(fx->inst, buf, 0u), NESTURBATOR_ERR_BUFFER_TOO_SMALL);
    /* D-02: no error wrote a byte. */
    for (size_t i = 0u; i < fx->size + 32u; ++i)
        CHECK_EQ_HEX(buf[i], 0xccu);
    /* A larger capacity writes exactly the state size. */
    CHECK_EQ_U64(nesturbator_save_state(fx->inst, buf, fx->size + 32u), NESTURBATOR_OK);
    CHECK(memcmp(buf, fx->state, fx->size) == 0);
    for (size_t i = fx->size; i < fx->size + 32u; ++i)
        CHECK_EQ_HEX(buf[i], 0xccu);
    nesturbator_destroy(empty);
    free(buf);
}

/* Loads a (possibly altered) state, checks the status and, on a refusal, that the instance is
   byte for byte as it was (D-04). */
static void expect_load(nesturbator *inst, const uint8_t *state, size_t size,
                        nesturbator_status want, const char *what)
{
    struct st_snap snap;
    nesturbator_status got;
    st_snapshot(inst, &snap);
    got = nesturbator_load_state(inst, state, size);
    if (got != want)
        fprintf(stderr, "case %s: status %d, wanted %d\n", what, (int)got, (int)want);
    CHECK_EQ_U64(got, want);
    if (want != NESTURBATOR_OK) {
        if (!st_same(inst, &snap))
            fprintf(stderr, "case %s: instance changed by a refused load\n", what);
        CHECK(st_same(inst, &snap));
    }
    st_snap_free(&snap);
}

/* D-04: one case per step, in the order of the checks. The target instance is a fresh one with the
   same cartridge, so a successful load would be visible. */
static void load_order(const struct fixture *fx, const uint8_t *rom, size_t rom_size)
{
    nesturbator *t = st_make_instance();
    uint8_t *s = (uint8_t *)malloc(fx->size + 1u);
    st_load(t, rom, rom_size);

    /* 1: arguments. */
    CHECK_EQ_U64(nesturbator_load_state(NULL, fx->state, fx->size), NESTURBATOR_ERR_ARGUMENT);
    CHECK_EQ_U64(nesturbator_load_state(t, NULL, fx->size), NESTURBATOR_ERR_ARGUMENT);
    /* 2: shorter than the header. */
    expect_load(t, fx->state, 63u, NESTURBATOR_ERR_STATE_SIZE, "63 bytes");
    expect_load(t, fx->state, 0u, NESTURBATOR_ERR_STATE_SIZE, "0 bytes");
    /* 3: magic and version. */
    memcpy(s, fx->state, fx->size);
    s[0] ^= 1u;
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_FORMAT, "magic byte 0");
    memcpy(s, fx->state, fx->size);
    s[7] ^= 0x80u;
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_FORMAT, "magic byte 7");
    memcpy(s, fx->state, fx->size);
    s[8] = 2u;
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_FORMAT, "version 2");
    /* 4: identity, before the length. */
    memcpy(s, fx->state, fx->size);
    s[20] ^= 1u;
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_CARTRIDGE, "mapper id");
    memcpy(s, fx->state, fx->size);
    s[22] = 1u;
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_CARTRIDGE, "submapper");
    memcpy(s, fx->state, fx->size);
    s[23] = 1u;
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_CARTRIDGE, "region");
    memcpy(s, fx->state, fx->size);
    s[24] ^= 0x10u;
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_CARTRIDGE, "rom crc");
    memcpy(s, fx->state, fx->size);
    s[20] ^= 1u;
    expect_load(t, s, 100u, NESTURBATOR_ERR_STATE_CARTRIDGE, "wrong game and wrong length");
    /* 5: lengths. */
    expect_load(t, fx->state, fx->size - 1u, NESTURBATOR_ERR_STATE_SIZE, "one byte short");
    memcpy(s, fx->state, fx->size);
    s[fx->size] = 0u;
    expect_load(t, s, fx->size + 1u, NESTURBATOR_ERR_STATE_SIZE, "one byte long");
    memcpy(s, fx->state, fx->size);
    s[12] ^= 1u;
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_SIZE, "total size field");
    /* 6: checksum: a body byte, the behaviour revision, the CRC field, a reserved byte and
       header_size, none with the CRC recomputed. */
    memcpy(s, fx->state, fx->size);
    s[100] ^= 1u;
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_CHECKSUM, "body byte");
    memcpy(s, fx->state, fx->size);
    s[16] ^= 1u;
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_CHECKSUM, "behaviour revision");
    memcpy(s, fx->state, fx->size);
    s[28] ^= 1u;
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_CHECKSUM, "crc field");
    memcpy(s, fx->state, fx->size);
    s[50] ^= 1u;
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_CHECKSUM, "reserved byte, crc stale");
    memcpy(s, fx->state, fx->size);
    s[10] ^= 1u;
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_CHECKSUM, "header_size, crc stale");
    /* 7: invalid, with the CRC recomputed. A header_size flip is INVALID, never FORMAT. */
    memcpy(s, fx->state, fx->size);
    s[48] = 1u;
    st_recrc(s, fx->size);
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_INVALID, "reserved byte 48");
    memcpy(s, fx->state, fx->size);
    s[63] = 1u;
    st_recrc(s, fx->size);
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_INVALID, "reserved byte 63");
    memcpy(s, fx->state, fx->size);
    s[10] = 63u;
    st_recrc(s, fx->size);
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_INVALID, "header_size 63");
    memcpy(s, fx->state, fx->size);
    s[10] = 65u;
    st_recrc(s, fx->size);
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_INVALID, "header_size 65");
    /* A field out of range: cpu.jammed is body offset 26 (after the 14 core bytes and 12 cpu
       bytes), so byte 64 + 26. */
    memcpy(s, fx->state, fx->size);
    s[64 + 26] = 2u;
    st_recrc(s, fx->size);
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_INVALID, "cpu.jammed 2");
    /* The unaltered state loads. */
    expect_load(t, fx->state, fx->size, NESTURBATOR_OK, "valid state");
    free(s);
    nesturbator_destroy(t);
}

/* STATE-03 edge: with no cartridge the size is 0 and both calls say so. */
static void empty_instance(const struct fixture *fx)
{
    nesturbator *e = st_make_instance();
    uint8_t buf[64];
    CHECK_EQ_U64(nesturbator_state_size(NULL), 0u);
    CHECK_EQ_U64(nesturbator_state_size(e), 0u);
    CHECK_EQ_U64(nesturbator_save_state(e, buf, sizeof buf), NESTURBATOR_ERR_STATE_CARTRIDGE);
    expect_load(e, fx->state, fx->size, NESTURBATOR_ERR_STATE_CARTRIDGE, "load with no cartridge");
    expect_load(e, fx->state, 64u, NESTURBATOR_ERR_STATE_CARTRIDGE, "header only, no cartridge");
    nesturbator_destroy(e);
}

/* D-08: a load restores frame_number and ticks; the first frame after it is the next one. */
static void position_restored(const struct fixture *fx, const uint8_t *rom, size_t rom_size)
{
    nesturbator *t = st_make_instance();
    uint64_t ticks_resumed, frame_resumed, ticks_next, frame_next;
    st_load(t, rom, rom_size);
    CHECK_EQ_U64(nesturbator_load_state(t, fx->state, fx->size), NESTURBATOR_OK);
    CHECK_EQ_U64(st_frame(t, st_buttons(TRACER_FRAMES + 1u), NULL, NULL, &ticks_resumed,
                          &frame_resumed),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(frame_resumed, fx->frame_number + 1u);
    CHECK_EQ_U64(fx->frame_number, TRACER_FRAMES);
    CHECK_EQ_U64(st_frame(fx->inst, st_buttons(TRACER_FRAMES + 1u), NULL, NULL, &ticks_next,
                          &frame_next),
                 NESTURBATOR_OK);
    CHECK_EQ_U64(ticks_resumed, ticks_next);
    CHECK_EQ_U64(frame_resumed, frame_next);
    nesturbator_destroy(t);
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
    crc_vector();
    {
        struct fixture fx;
        fixture_make(&fx, rom, rom_size);
        header_layout(&fx);
        save_order(&fx);
        load_order(&fx, rom, rom_size);
        empty_instance(&fx);
        position_restored(&fx, rom, rom_size);
        fixture_free(&fx);
    }
    free(rom);
    CHECK_DONE();
}

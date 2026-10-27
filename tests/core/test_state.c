/* STATE-01, STATE-02 (D-01 to D-08, D-16): save states of the library. The tracer saves dabg.nes
   (NROM) at frame 60, loads the state into a fresh instance and checks that the next 60 frames,
   their audio, their ticks and the final state bytes equal the uninterrupted run's. argv[1] is
   tests/roms/dabg.nes. White box: includes internal.h to compare whole instances. The st_*
   helpers live in tests/state_test.h, shared with the fork and sweep tests. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"
#include "nesturbator.h"
#include "../check.h"
#include "../ines.h"
#include "../state_test.h"

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
        uint32_t video = 0u, audio = 0u;
        uint64_t ticks = 0u, frame_number = 0u;
        CHECK_EQ_U64(
            st_frame(b, st_buttons(TRACER_FRAMES + 1u + i), &video, &audio, &ticks, &frame_number),
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
    s[8] = 1u;
    expect_load(t, s, fx->size, NESTURBATOR_ERR_STATE_FORMAT, "version 1");
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
    /* A field out of range: cpu.jammed is body offset 28 (after the 14 core bytes and 14 cpu
       bytes), so byte 64 + 26. */
    memcpy(s, fx->state, fx->size);
    s[64 + 28] = 2u;
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
    CHECK_EQ_U64(
        st_frame(t, st_buttons(TRACER_FRAMES + 1u), NULL, NULL, &ticks_resumed, &frame_resumed),
        NESTURBATOR_OK);
    CHECK_EQ_U64(frame_resumed, fx->frame_number + 1u);
    CHECK_EQ_U64(fx->frame_number, TRACER_FRAMES);
    CHECK_EQ_U64(
        st_frame(fx->inst, st_buttons(TRACER_FRAMES + 1u), NULL, NULL, &ticks_next, &frame_next),
        NESTURBATOR_OK);
    CHECK_EQ_U64(ticks_resumed, ticks_next);
    CHECK_EQ_U64(frame_resumed, frame_next);
    nesturbator_destroy(t);
}

/* ---- Board registers (STATE-02, STATE-03) ---- */

#define BOARD_PAGES (48u + 48u + 8u + 8u)

/* Where every page pointer lands, as an offset: -1 for NULL, the PRG, CHR or PRG-RAM offset with a
   tag in the high bits otherwise, so two instances holding the same image compare equal. */
static void board_pages(const nesturbator *inst, int64_t *out)
{
    const uint8_t *p[BOARD_PAGES];
    size_t n = 0u;
    for (size_t i = 0u; i < 48u; ++i)
        p[n++] = inst->map.cpu_r[i];
    for (size_t i = 0u; i < 48u; ++i)
        p[n++] = inst->map.cpu_w[i];
    for (size_t i = 0u; i < 8u; ++i)
        p[n++] = inst->map.chr_r[i];
    for (size_t i = 0u; i < 8u; ++i)
        p[n++] = inst->map.chr_w[i];
    for (size_t i = 0u; i < n; ++i) {
        if (p[i] == NULL)
            out[i] = -1;
        else if (p[i] >= inst->cart.prg && p[i] < inst->cart.prg + inst->cart.prg_size)
            out[i] = (int64_t)(p[i] - inst->cart.prg);
        else if (p[i] >= inst->cart.chr && p[i] < inst->cart.chr + inst->cart.chr_size)
            out[i] = ((int64_t)1 << 40) + (int64_t)(p[i] - inst->cart.chr);
        else if (inst->cart.prg_ram_size != 0u && p[i] >= inst->cart.prg_ram &&
                 p[i] < inst->cart.prg_ram + inst->cart.prg_ram_size)
            out[i] = ((int64_t)2 << 40) + (int64_t)(p[i] - inst->cart.prg_ram);
        else
            out[i] = ((int64_t)3 << 40);
    }
    for (size_t i = 0u; i < 4u; ++i)
        out[n + i] = inst->map.nt[i];
}

#define BOARD_SLOTS (BOARD_PAGES + 4u)

static uint8_t board_image[16u + 16384u * 16u + 8192u * 16u];

static size_t board_build(uint16_t mapper, uint8_t prg_16k, uint8_t chr_8k, uint8_t ram_shift)
{
    struct ines_spec spec;
    memset(&spec, 0, sizeof spec);
    spec.mapper = mapper;
    spec.prg_16k = prg_16k;
    spec.chr_8k = chr_8k;
    spec.nes2 = ram_shift != 0u ? 1u : 0u;
    spec.prg_ram_shift = ram_shift;
    spec.reset_vector = 0x8000u;
    return ines_build(board_image, sizeof board_image, &spec);
}

/* Registers set by the case, before the save. */
typedef void (*board_set_fn)(struct nesturbator__mapper *m);

/* Saves A with set() applied, loads into a fresh B holding the same image, and requires the
   registers and the rebuilt page tables to be equal. extra is the register bytes the board adds
   and the 8 KiB of PRG RAM a board has to the state over an NROM image with the same CHR kind.
   Returns A for the caller to destroy. */
static void board_case(uint16_t mapper, uint8_t prg_16k, uint8_t chr_8k, uint8_t ram_shift,
                       board_set_fn set, size_t extra)
{
    static int64_t pages_a[BOARD_SLOTS], pages_b[BOARD_SLOTS], pages_0[BOARD_SLOTS];
    nesturbator *a = st_make_instance();
    nesturbator *b = st_make_instance();
    nesturbator *twin = st_make_instance();
    nesturbator *fresh = st_make_instance();
    size_t image = board_build(mapper, prg_16k, chr_8k, ram_shift), size, twin_size;
    uint8_t *state;
    CHECK(image != 0u);
    st_load(a, board_image, image);
    st_load(fresh, board_image, image);
    set(&a->mapper);
    CHECK(nesturbator__mapper_load(a) == 1);
    size = nesturbator_state_size(a);
    state = (uint8_t *)malloc(size);
    CHECK_EQ_U64(nesturbator_save_state(a, state, size), NESTURBATOR_OK);
    st_load(b, board_image, image);
    CHECK_EQ_U64(nesturbator_load_state(b, state, size), NESTURBATOR_OK);
    CHECK(memcmp(&a->mapper, &b->mapper, sizeof a->mapper) == 0);
    board_pages(a, pages_a);
    board_pages(b, pages_b);
    CHECK(memcmp(pages_a, pages_b, sizeof pages_a) == 0);
    if (mapper != 0u) {
        /* The case moved the pages: a load that dropped the registers would differ. */
        board_pages(fresh, pages_0);
        CHECK(memcmp(pages_a, pages_0, sizeof pages_a) != 0);
    }
    /* The state is the NROM state of the same shape plus the board's register bytes. */
    image = board_build(0u, 2u, chr_8k != 0u ? 1u : 0u, 0u);
    st_load(twin, board_image, image);
    twin_size = nesturbator_state_size(twin);
    CHECK_EQ_U64(size - twin_size, extra);
    free(state);
    nesturbator_destroy(a);
    nesturbator_destroy(b);
    nesturbator_destroy(twin);
    nesturbator_destroy(fresh);
}

static void set_uxrom(struct nesturbator__mapper *m)
{
    m->reg.uxrom.bank = 0xa7u;
}
static void set_cnrom(struct nesturbator__mapper *m)
{
    m->reg.cnrom.bank = 0xa7u;
}
static void set_axrom(struct nesturbator__mapper *m)
{
    m->reg.axrom.bank = 0xa7u;
}
static void set_nrom(struct nesturbator__mapper *m)
{
    (void)m;
}

static void set_mmc1(struct nesturbator__mapper *m)
{
    m->reg.mmc1.shift = 0x13u;
    m->reg.mmc1.count = 3u;
    m->reg.mmc1.control_x = 0x1fu;
    m->reg.mmc1.chr0 = 0x05u;
    m->reg.mmc1.chr1 = 0x1eu;
    m->reg.mmc1.prg = 0x0cu;
    m->reg.mmc1.last_write = 0x0123456789abcdefu;
}

static void set_mmc3(struct nesturbator__mapper *m)
{
    m->irq = 1u;
    m->reg.mmc3.select = 0xc5u;
    for (unsigned i = 0u; i < 8u; ++i)
        m->reg.mmc3.r[i] = (uint8_t)(0x11u * (i + 1u) + 3u);
    m->reg.mmc3.mirror = 0xfdu;
    m->reg.mmc3.a001_x = 0x40u;
    m->reg.mmc3.latch = 0x6bu;
    m->reg.mmc3.counter = 0x2eu;
    m->reg.mmc3.reload = 1u;
    m->reg.mmc3.irq_enable = 1u;
    m->reg.mmc3.a12 = 1u;
    m->reg.mmc3.low_cycle = 0xfedcba9876543210u;
}

/* STATE-02: all six boards of the loader provide the mandatory state op. */
static void ops_table(void)
{
    static const uint16_t ids[6] = {0u, 1u, 2u, 3u, 4u, 7u};
    for (size_t i = 0u; i < 6u; ++i) {
        struct nesturbator__mapper_ops ops;
        memset(&ops, 0, sizeof ops);
        CHECK(nesturbator__mapper_ops_for(ids[i], &ops) == 1);
        CHECK(ops.state != NULL);
    }
}

/* NROM adds nothing after irq: the twin is the NROM, so the extra is 0. */
static void board_nrom(void)
{
    board_case(0u, 2u, 1u, 0u, set_nrom, 0u);
}
static void board_uxrom(void)
{
    board_case(2u, 8u, 0u, 0u, set_uxrom, 1u);
}
static void board_cnrom(void)
{
    board_case(3u, 2u, 4u, 0u, set_cnrom, 1u);
}
static void board_axrom(void)
{
    board_case(7u, 8u, 0u, 0u, set_axrom, 1u);
}
static void board_mmc1(void)
{
    board_case(1u, 16u, 0u, 7u, set_mmc1, 14u + 8192u);
}
static void board_mmc3(void)
{
    board_case(4u, 8u, 16u, 0u, set_mmc3, 24u + 8192u);
}

/* Pitfall P1: map.a12 is derived from ppu.bus_addr, not saved. */
static void mmc3_a12_derived(void)
{
    static const uint16_t addrs[2] = {0x1000u, 0x0000u};
    for (size_t i = 0u; i < 2u; ++i) {
        nesturbator *a = st_make_instance();
        nesturbator *b = st_make_instance();
        size_t image = board_build(4u, 8u, 16u, 0u), size;
        uint8_t *state;
        st_load(a, board_image, image);
        a->ppu.bus_addr = addrs[i];
        size = nesturbator_state_size(a);
        state = (uint8_t *)malloc(size);
        CHECK_EQ_U64(nesturbator_save_state(a, state, size), NESTURBATOR_OK);
        st_load(b, board_image, image);
        b->map.a12 = (uint8_t)(1u - ((addrs[i] >> 12) & 1u));
        CHECK_EQ_U64(nesturbator_load_state(b, state, size), NESTURBATOR_OK);
        CHECK_EQ_U64(b->map.a12, (b->ppu.bus_addr >> 12) & 1u);
        CHECK_EQ_U64(b->map.a12, addrs[i] >> 12);
        free(state);
        nesturbator_destroy(a);
        nesturbator_destroy(b);
    }
}

/* ---- STATE-01 contract (D-01, D-02, D-05, D-06) ---- */

static uint8_t jam_image[16u + 16384u * 2u + 8192u];

static size_t jam_build(void)
{
    static const uint8_t jam_code[] = {0x02u};
    struct ines_spec spec;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 2u;
    spec.chr_8k = 1u;
    spec.prg_code = jam_code;
    spec.prg_code_len = sizeof jam_code;
    spec.reset_vector = 0x8000u;
    return ines_build(jam_image, sizeof jam_image, &spec);
}

/* Runs frames until the status is not OK or n frames passed; returns the last status. */
static nesturbator_status run_frames(nesturbator *inst, uint32_t n)
{
    nesturbator_status st = NESTURBATOR_OK;
    for (uint32_t i = 0u; i < n && st == NESTURBATOR_OK; ++i)
        st = st_frame(inst, st_buttons(i), NULL, NULL, NULL, NULL);
    return st;
}

/* D-01: the size is 0 with no instance or cartridge, constant from load to unload, and reading it
   changes nothing. */
static void size_shape(const uint8_t *rom, size_t rom_size)
{
    nesturbator *e = st_make_instance();
    nesturbator *inst = st_make_instance();
    nesturbator *jam = st_make_instance();
    nesturbator *nrom = st_make_instance();
    nesturbator *mmc1 = st_make_instance();
    size_t s, jam_size;
    struct st_snap snap;
    CHECK_EQ_U64(nesturbator_state_size(NULL), 0u);
    CHECK_EQ_U64(nesturbator_state_size(e), 0u);
    st_load(inst, rom, rom_size);
    st_snapshot(inst, &snap);
    s = nesturbator_state_size(inst);
    CHECK(s > 64u);
    CHECK(st_same(inst, &snap));
    st_snap_free(&snap);
    CHECK_EQ_U64(run_frames(inst, 1000u), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_state_size(inst), s);
    CHECK_EQ_U64(nesturbator_reset(inst), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_state_size(inst), s);
    CHECK_EQ_U64(run_frames(inst, 3u), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_state_size(inst), s);
    nesturbator_unload_cartridge(inst);
    CHECK_EQ_U64(nesturbator_state_size(inst), 0u);
    /* A reload of the same image gives the same constant. */
    st_load(inst, rom, rom_size);
    CHECK_EQ_U64(nesturbator_state_size(inst), s);

    /* Across a JAM. */
    CHECK(jam_build() != 0u);
    st_load(jam, jam_image, sizeof jam_image);
    jam_size = nesturbator_state_size(jam);
    CHECK(jam_size > 64u);
    CHECK_EQ_U64(run_frames(jam, 3u), NESTURBATOR_STOP_JAM);
    CHECK_EQ_U64(nesturbator_state_size(jam), jam_size);

    /* 8 KiB PRG RAM and CHR RAM add at least 16 KiB over a CHR-ROM NROM image. */
    st_load(nrom, board_image, board_build(0u, 2u, 1u, 0u));
    st_load(mmc1, board_image, board_build(1u, 16u, 0u, 7u));
    CHECK(nesturbator_state_size(mmc1) >= nesturbator_state_size(nrom) + 8192u + 8192u);

    nesturbator_destroy(e);
    nesturbator_destroy(inst);
    nesturbator_destroy(jam);
    nesturbator_destroy(nrom);
    nesturbator_destroy(mmc1);
}

/* D-02: the library never calls the allocator to save a state. */
static void save_never_allocates(const uint8_t *rom, size_t rom_size)
{
    struct st_counting_allocator c = {0u, 0u};
    nesturbator *inst = st_make_instance_counting(&c);
    size_t size;
    uint8_t *buf;
    unsigned allocs, frees;
    st_load(inst, rom, rom_size);
    CHECK_EQ_U64(run_frames(inst, 5u), NESTURBATOR_OK);
    size = nesturbator_state_size(inst);
    buf = (uint8_t *)malloc(size);
    allocs = c.allocs;
    frees = c.frees;
    CHECK(allocs > 0u);
    for (unsigned i = 0u; i < 100u; ++i) {
        CHECK_EQ_U64(nesturbator_save_state(inst, buf, size), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_state_size(inst), size);
    }
    CHECK_EQ_U64(nesturbator_load_state(inst, buf, size), NESTURBATOR_OK);
    CHECK_EQ_U64(c.allocs, allocs);
    CHECK_EQ_U64(c.frees, frees);
    free(buf);
    nesturbator_destroy(inst);
}

/* D-01: save takes a const instance and leaves every byte of it, and the cartridge RAM, alone. */
static void save_const(const uint8_t *rom, size_t rom_size)
{
    nesturbator *inst = st_make_instance();
    struct st_snap snap;
    size_t size;
    uint8_t *buf;
    st_load(inst, rom, rom_size);
    CHECK_EQ_U64(run_frames(inst, 20u), NESTURBATOR_OK);
    size = nesturbator_state_size(inst);
    buf = (uint8_t *)malloc(size);
    st_snapshot(inst, &snap);
    for (unsigned i = 0u; i < 3u; ++i)
        CHECK_EQ_U64(nesturbator_save_state(inst, buf, size), NESTURBATOR_OK);
    CHECK(st_same(inst, &snap));
    /* An error also leaves it alone. */
    CHECK_EQ_U64(nesturbator_save_state(inst, buf, size - 1u), NESTURBATOR_ERR_BUFFER_TOO_SMALL);
    CHECK(st_same(inst, &snap));
    st_snap_free(&snap);
    free(buf);
    nesturbator_destroy(inst);
}

/* D-05: exactly state_size bytes on OK; nothing on any error. */
static void buffer_rules(const uint8_t *rom, size_t rom_size)
{
    nesturbator *inst = st_make_instance();
    nesturbator *empty = st_make_instance();
    size_t size;
    uint8_t *buf;
    st_load(inst, rom, rom_size);
    CHECK_EQ_U64(run_frames(inst, 10u), NESTURBATOR_OK);
    size = nesturbator_state_size(inst);
    buf = (uint8_t *)malloc(size + 16u);
    memset(buf, 0xcc, size + 16u);
    CHECK_EQ_U64(nesturbator_save_state(inst, buf, size + 16u), NESTURBATOR_OK);
    for (size_t i = size; i < size + 16u; ++i)
        CHECK_EQ_HEX(buf[i], 0xccu);
    CHECK_EQ_HEX(buf[0], 0x89u);
    memset(buf, 0xcc, size + 16u);
    CHECK_EQ_U64(nesturbator_save_state(inst, buf, size - 1u), NESTURBATOR_ERR_BUFFER_TOO_SMALL);
    CHECK_EQ_U64(nesturbator_save_state(empty, buf, size + 16u), NESTURBATOR_ERR_STATE_CARTRIDGE);
    for (size_t i = 0u; i < size + 16u; ++i)
        CHECK_EQ_HEX(buf[i], 0xccu);
    free(buf);
    nesturbator_destroy(inst);
    nesturbator_destroy(empty);
}

/* STATE-01 idempotency: saving twice, or save, load, save, gives the same bytes. */
static void idempotent(const uint8_t *rom, size_t rom_size)
{
    nesturbator *inst = st_make_instance();
    size_t size;
    uint8_t *a, *b, *c;
    st_load(inst, rom, rom_size);
    CHECK_EQ_U64(run_frames(inst, 40u), NESTURBATOR_OK);
    size = nesturbator_state_size(inst);
    a = (uint8_t *)malloc(size);
    b = (uint8_t *)malloc(size);
    c = (uint8_t *)malloc(size);
    CHECK_EQ_U64(nesturbator_save_state(inst, a, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(inst, b, size), NESTURBATOR_OK);
    CHECK(memcmp(a, b, size) == 0);
    CHECK_EQ_U64(run_frames(inst, 7u), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_state(inst, a, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(inst, c, size), NESTURBATOR_OK);
    CHECK(memcmp(a, c, size) == 0);
    free(a);
    free(b);
    free(c);
    nesturbator_destroy(inst);
}

/* STATE-01 concurrency: the state code has no shared writable data, so a second instance running
   frames between the calls of the first changes nothing in the first one's state. */
static void two_instances(const uint8_t *rom, size_t rom_size)
{
    nesturbator *a = st_make_instance();
    nesturbator *b = st_make_instance();
    size_t size;
    uint8_t *alone, *mixed, *b_state;
    st_load(a, rom, rom_size);
    st_load(b, rom, rom_size);
    CHECK_EQ_U64(run_frames(a, 30u), NESTURBATOR_OK);
    size = nesturbator_state_size(a);
    alone = (uint8_t *)malloc(size);
    mixed = (uint8_t *)malloc(size);
    b_state = (uint8_t *)malloc(size);
    CHECK_EQ_U64(nesturbator_save_state(a, alone, size), NESTURBATOR_OK);
    CHECK_EQ_U64(run_frames(b, 17u), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(b, b_state, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_state(b, alone, size), NESTURBATOR_OK);
    CHECK_EQ_U64(run_frames(b, 5u), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(a, mixed, size), NESTURBATOR_OK);
    CHECK(memcmp(alone, mixed, size) == 0);
    /* B, loaded from A's state, is in A's state plus 5 frames, not in its own earlier one. */
    CHECK(memcmp(b_state, alone, size) != 0);
    free(alone);
    free(mixed);
    free(b_state);
    nesturbator_destroy(a);
    nesturbator_destroy(b);
}

/* D-06: a jammed instance saves, and the latch travels in both directions. The image waits in a
   loop while CPU RAM $10 is zero and runs a JAM opcode when it is not, so a test chooses the frame
   that jams: LDA $10; BEQ $8000; JAM. */
static size_t gated_jam_build(void)
{
    static const uint8_t code[] = {0xa5u, 0x10u, 0xf0u, 0xfcu, 0x02u};
    struct ines_spec spec;
    memset(&spec, 0, sizeof spec);
    spec.prg_16k = 2u;
    spec.chr_8k = 1u;
    spec.prg_code = code;
    spec.prg_code_len = sizeof code;
    spec.reset_vector = 0x8000u;
    return ines_build(jam_image, sizeof jam_image, &spec);
}

static void jam_both_ways(void)
{
    nesturbator *jammed = st_make_instance();
    nesturbator *fresh = st_make_instance();
    nesturbator *twin = st_make_instance();
    size_t size, image = gated_jam_build();
    uint8_t *jam_state, *clean_state, *again;
    CHECK(image != 0u);
    st_load(jammed, jam_image, image);
    st_load(fresh, jam_image, image);
    st_load(twin, jam_image, image);
    size = nesturbator_state_size(jammed);
    jam_state = (uint8_t *)malloc(size);
    clean_state = (uint8_t *)malloc(size);
    again = (uint8_t *)malloc(size);
    CHECK_EQ_U64(run_frames(jammed, 3u), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(jammed, clean_state, size), NESTURBATOR_OK);
    jammed->bus.ram[0x10] = 1u;
    CHECK_EQ_U64(run_frames(jammed, 3u), NESTURBATOR_STOP_JAM);
    /* Saving a jammed instance is OK and never STOP_JAM. */
    CHECK_EQ_U64(nesturbator_save_state(jammed, jam_state, size), NESTURBATOR_OK);
    /* The jammed state is frame-invariant: further STOP_JAM frames leave every saved byte as it
       was, so a host that tracks the jammed state by its CRC sees one identity (phase 19). */
    CHECK_EQ_U64(run_frames(jammed, 3u), NESTURBATOR_STOP_JAM);
    CHECK_EQ_U64(nesturbator_save_state(jammed, again, size), NESTURBATOR_OK);
    CHECK(memcmp(again, jam_state, size) == 0);
    /* Loading the jammed state: OK, then the next frame returns STOP_JAM. */
    CHECK_EQ_U64(nesturbator_load_state(fresh, jam_state, size), NESTURBATOR_OK);
    CHECK_EQ_U64(st_frame(fresh, 0u, NULL, NULL, NULL, NULL), NESTURBATOR_STOP_JAM);
    /* Loading it into an instance that is not jammed, then saving, gives the same bytes. */
    CHECK_EQ_U64(nesturbator_load_state(twin, jam_state, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_save_state(twin, again, size), NESTURBATOR_OK);
    CHECK(memcmp(again, jam_state, size) == 0);
    /* A jammed instance loading the clean state is running again. */
    CHECK_EQ_U64(nesturbator_load_state(jammed, clean_state, size), NESTURBATOR_OK);
    CHECK_EQ_U64(st_frame(jammed, 0u, NULL, NULL, NULL, NULL), NESTURBATOR_OK);
    /* The sprite-0 hit is set while pixels are composed, so a frame without a video buffer must
       be refused rather than run: otherwise emulation would depend on output (phase 19). */
    {
        static int16_t audio[1024];
        nesturbator_frame io;
        memset(&io, 0, sizeof io);
        io.size = (uint32_t)sizeof io;
        io.video = NULL;
        io.video_pitch = 256u;
        io.audio = audio;
        io.audio_capacity = 1024u;
        CHECK_EQ_U64(nesturbator_save_state(jammed, again, size), NESTURBATOR_OK);
        CHECK_EQ_U64(nesturbator_run_frame(jammed, &io), NESTURBATOR_ERR_ARGUMENT);
        CHECK_EQ_U64(nesturbator_save_state(jammed, jam_state, size), NESTURBATOR_OK);
        CHECK(memcmp(again, jam_state, size) == 0);
    }
    free(jam_state);
    free(clean_state);
    free(again);
    nesturbator_destroy(jammed);
    nesturbator_destroy(fresh);
    nesturbator_destroy(twin);
}

/* Reset: a state saved right after nesturbator_reset loads and the next frames match. */
static void after_reset(const uint8_t *rom, size_t rom_size)
{
    nesturbator *a = st_make_instance();
    nesturbator *b = st_make_instance();
    size_t size;
    uint8_t *state;
    st_load(a, rom, rom_size);
    st_load(b, rom, rom_size);
    CHECK_EQ_U64(run_frames(a, 10u), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_reset(a), NESTURBATOR_OK);
    size = nesturbator_state_size(a);
    state = (uint8_t *)malloc(size);
    CHECK_EQ_U64(nesturbator_save_state(a, state, size), NESTURBATOR_OK);
    CHECK_EQ_U64(nesturbator_load_state(b, state, size), NESTURBATOR_OK);
    for (uint32_t i = 0u; i < 5u; ++i) {
        uint32_t va = 0u, aa = 0u, vb = 0u, ab = 0u;
        uint64_t ta = 0u, fa = 0u, tb = 0u, fb = 0u;
        CHECK_EQ_U64(st_frame(a, st_buttons(i), &va, &aa, &ta, &fa), NESTURBATOR_OK);
        CHECK_EQ_U64(st_frame(b, st_buttons(i), &vb, &ab, &tb, &fb), NESTURBATOR_OK);
        CHECK_EQ_HEX(va, vb);
        CHECK_EQ_HEX(aa, ab);
        CHECK_EQ_U64(ta, tb);
        CHECK_EQ_U64(fa, fb);
    }
    free(state);
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
    ops_table();
    board_nrom();
    board_uxrom();
    board_cnrom();
    board_axrom();
    board_mmc1();
    board_mmc3();
    mmc3_a12_derived();
    size_shape(rom, rom_size);
    save_never_allocates(rom, rom_size);
    save_const(rom, rom_size);
    buffer_rules(rom, rom_size);
    idempotent(rom, rom_size);
    two_instances(rom, rom_size);
    jam_both_ways();
    after_reset(rom, rom_size);
    free(rom);
    CHECK_DONE();
}
